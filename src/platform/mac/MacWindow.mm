#include "platform/mac/MacWindow.h"

#import <AppKit/AppKit.h>
#include <QGuiApplication>
#include <QWidget>

namespace {
NSView* NativeView(const WId wid)
{
    // On macOS WId is the NSView pointer of the QWidget's native window. ARC
    // requires the bridge cast because the handle travels as an integer.
    return (__bridge NSView*)(reinterpret_cast<void*>(wid));
}
}

namespace gpm {
void MacPrepareNonActivatingWindow(QWidget* widget)
{
    if (!widget || !widget->internalWinId()) return;
    // WId is an NSView only under the Cocoa platform plugin. Offscreen builds
    // (used by the UI tests) hand out opaque handles that must not be messaged.
    if (QGuiApplication::platformName() != QStringLiteral("cocoa")) return;
    NSView* view = NativeView(widget->internalWinId());
    NSWindow* window = [view window];
    if (!window) return;
    // Qt may reuse the native window, so applying the contract must be idempotent.
    // A persistent menu follows the user across spaces instead of moving only when
    // the app is activated (which never happens for a nonactivating panel).
    [window setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces
        | NSWindowCollectionBehaviorFullScreenAuxiliary | NSWindowCollectionBehaviorStationary];
    [window setHidesOnDeactivate:NO];
    [window setLevel:NSFloatingWindowLevel];
    if ([window isKindOfClass:[NSPanel class]]) {
        NSPanel* panel = (NSPanel*)window;
        // The nonactivating mask keeps a click on the persistent menu from
        // activating GoPieMenu, which would otherwise clear the target app's
        // foreground window before the action is dispatched.
        [panel setStyleMask:[panel styleMask] | NSWindowStyleMaskNonactivatingPanel];
        [panel setBecomesKeyOnlyIfNeeded:YES];
        [panel setFloatingPanel:YES];
    }
}

void MacActivateApplication()
{
    [NSApp activateIgnoringOtherApps:YES];
}
}
