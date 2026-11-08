#pragma once
class QWidget;

namespace gpm {
// Applies the macOS counterpart of the Windows WS_EX_NOACTIVATE contract: Qt
// creates an NSPanel for Qt::Tool widgets, and the nonactivating panel mask plus
// a floating, space-independent window keeps persistent menus interactive
// without pulling keyboard focus away from the working application.
void MacPrepareNonActivatingWindow(QWidget* widget);

// LSUIElement applications have no Dock icon, so showing the editor must ask
// AppKit explicitly to bring GoPieMenu to the front.
void MacActivateApplication();
}
