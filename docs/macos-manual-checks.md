# macOS manual acceptance checks

Automated CI covers the portable libraries, the editor, the runtime menus, and the QML tests at four scale factors on a virtual 1920 × 1080 screen. The checks below need a real Mac desktop, an interactive login session, and, for the trigger tests, another application to receive the actions. Run them on both an Apple silicon and an Intel Mac when both are available, and once on a mixed-DPI multi-monitor setup.

## Permissions and first launch

- Fresh download from a browser: opening the app is blocked by Gatekeeper. Choose **Open** from the Finder context menu, or **System Settings > Privacy & Security > Open Anyway**, and confirm. The app then starts in the menu bar without a Dock icon.
- First start shows the Accessibility permission prompt. Grant it in **System Settings > Privacy & Security > Accessibility** (and **Input Monitoring** when macOS lists it), then restart GoPieMenu.
- With permissions granted, restart the app: no error banner appears, and a configured trigger opens the menu.
- Revoke Accessibility while GoPieMenu runs, then trigger a menu: the app must not hang, and the next start must report the missing permission again.
- Repeat after upgrading the app with a new ad-hoc signature: TCC may keep or reset the entry; verify the app still asks for the permission instead of silently failing.

## Input and actions

- Default **Ctrl + right mouse button** hold: the pie appears under the pointer, follows the pointer, and confirms on release. Escape cancels.
- A custom key-hold trigger and a key-toggle trigger both work; toggle confirms on a left click and cancels on Escape.
- Sided modifiers are independent: **Left Shift** and **Right Shift** (and the Control, Option, and Command variants) stay distinct in the recorded trigger.
- Keyboard shortcut actions reach the frontmost application: verify **Cmd+A/Down/Up** style shortcuts in TextEdit, and a shortcut that is already held by the user is released and restored.
- Launch app, open file or folder, open URL, and run command actions work from both the pie and a submenu.
- With the action hint enabled, the hint appears on the pie menu's screen with the configured position, size, margin, opacity, and hold time.
- When a password field or another app with **Secure Input** enabled is focused, the trigger does not fire; this is a macOS limitation and must fail closed instead of sending input.

## Persistent floating menus

- An app-scoped persistent menu appears only while that app is frontmost, and disappears or yields correctly when another app or an app-specific shortcut menu takes over.
- Clicking the persistent menu does not steal keyboard focus: type in the target app before and after clicking an action and confirm the text still lands in the app.
- Repeated clicks run actions repeatedly; dragging the center moves the menu and clamps it to the screen; the submenu follows the center.
- Submenu toggle, **Collapse submenu after click**, center **Hide all persistent menus**, and the menu bar toggle keep their remembered state across app switches.
- The persistent menu stays above ordinary windows without taking focus, appears on every Space, and is not hidden by Mission Control.
- With two displays of different scale factors, the persistent menu and the action hint appear on the expected screen and position.

## Editor, menu bar, and startup

- The editor opens from the menu bar, raises over other apps, and can be minimized, maximized/zoomed, resized, and closed without losing drafts.
- The custom title bar buttons, dialogs, color picker, icon picker, organizer drag-and-drop, language switch/restart, import/export, preview hint, and Apply/Discard work at 100 % and 200 % display scaling.
- The menu bar item uses the monochrome template image and stays readable in light and dark menu bars. Its menu shows the same actions as the Windows tray: Settings, update entry when available, hide persistent menus, Pause/Resume, and Quit.
- Enable **Start at login**, log out and back in: GoPieMenu starts in the menu bar with `--autostart`. Disable it again and confirm `~/Library/LaunchAgents/com.gopiemenu.app.plist` is removed.
- A second manual launch raises the existing editor instead of starting a second instance. Restarting for a language change applies pending edits and replaces the running process.

## Updates and packaging

- With a test release that has a DMG asset, **Check for updates** shows localized notes, the download verifies against the merged `SHA256SUMS.txt`, and **Open and exit** mounts the disk image after resolving dirty drafts.
- Cancelling a download, closing the dialog with Later, and skipping a version keep the app running and remember the choice.
- The packaged `GoPieMenu-<version>.dmg` opens without the Qt SDK installed, the app inside runs, and the checksum in `SHA256SUMS.txt` matches both the Windows installer and the DMG lines.
- Dragging the app into Applications from the mounted image replaces the previous copy and keeps the configuration under `~/Library/Application Support/GoPieMenu`.
