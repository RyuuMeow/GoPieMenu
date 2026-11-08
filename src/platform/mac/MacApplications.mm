#include "platform/mac/MacApplications.h"

#include <QCoreApplication>
#include <QMetaObject>
#include <QThread>

#import <AppKit/AppKit.h>

namespace {
std::function<void()> gForegroundHandler;

void EnsureWorkspaceObserver();
void DispatchForegroundChange();
}

// NSWorkspace only posts activation notifications through its own notification
// center, so the observer has to live outside Qt's event system.
@interface GPMWorkspaceObserver : NSObject
- (void)workspaceDidActivateApplication:(NSNotification*)notification;
@end

@implementation GPMWorkspaceObserver
- (void)workspaceDidActivateApplication:(NSNotification*)notification
{
    Q_UNUSED(notification);
    DispatchForegroundChange();
}
@end

namespace {
QString ExecutableName(NSRunningApplication* application)
{
    NSURL* executable = application.executableURL;
    if (executable) {
        const auto name = QString::fromNSString(executable.lastPathComponent);
        if (!name.isEmpty()) return name;
    }
    NSURL* bundle = application.bundleURL;
    if (bundle) {
        auto name = QString::fromNSString(bundle.lastPathComponent);
        if (name.endsWith(QStringLiteral(".app"), Qt::CaseInsensitive)) name.chop(4);
        if (!name.isEmpty()) return name;
    }
    return QString::fromNSString(application.localizedName);
}

bool IsOwnApplication(NSRunningApplication* application)
{
    if (!application) return true;
    NSRunningApplication* current = [NSRunningApplication currentApplication];
    if (!current) return false;
    if (application.processIdentifier == current.processIdentifier) return true;
    // A second process of the same bundle (for example the restart helper)
    // must not appear in its own scope list.
    return application.bundleIdentifier && current.bundleIdentifier
        && [application.bundleIdentifier isEqualToString:current.bundleIdentifier];
}

void EnsureWorkspaceObserver()
{
    static GPMWorkspaceObserver* observer = nil;
    if (observer) return;
    observer = [[GPMWorkspaceObserver alloc] init];
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    [[workspace notificationCenter] addObserver:observer
                                      selector:@selector(workspaceDidActivateApplication:)
                                          name:NSWorkspaceDidActivateApplicationNotification
                                        object:workspace];
}

void DispatchForegroundChange()
{
    auto* application = QCoreApplication::instance();
    if (!application) return;
    // AppKit normally posts this on the main run loop; keep the handler on the
    // Qt GUI thread even if a newer SDK delivers it elsewhere.
    if (QThread::currentThread() != application->thread()) {
        QMetaObject::invokeMethod(application, [] {
            if (gForegroundHandler) gForegroundHandler();
        }, Qt::QueuedConnection);
        return;
    }
    if (gForegroundHandler) gForegroundHandler();
}
}

namespace gpm {
QString MacForegroundProcessName()
{
    @autoreleasepool {
        NSRunningApplication* application = [[NSWorkspace sharedWorkspace] frontmostApplication];
        if (!application || application.terminated || IsOwnApplication(application)) return {};
        return ExecutableName(application);
    }
}

QStringList MacRunningApplications()
{
    QStringList names;
    @autoreleasepool {
        for (NSRunningApplication* application in [[NSWorkspace sharedWorkspace] runningApplications]) {
            // Regular apps are the ones a user can switch to and scope a menu by;
            // accessories and background agents have no app scope to offer.
            if (application.activationPolicy != NSApplicationActivationPolicyRegular) continue;
            if (application.terminated || IsOwnApplication(application)) continue;
            const auto name = ExecutableName(application);
            if (name.isEmpty() || names.contains(name, Qt::CaseInsensitive)) continue;
            names.append(name);
        }
    }
    names.sort(Qt::CaseInsensitive);
    return names;
}

void MacSetForegroundChangeHandler(std::function<void()> handler)
{
    gForegroundHandler = std::move(handler);
    EnsureWorkspaceObserver();
}

void MacClearForegroundChangeHandler()
{
    // The observer stays registered so repeated hook install/uninstall cycles do
    // not touch AppKit; a cleared handler simply ignores notifications.
    gForegroundHandler = nullptr;
}
}
