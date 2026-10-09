# Editor architecture and validation

## State and persistence

`EditorSession` owns the draft, stable profile/item IDs, selection, and up to 100 undo snapshots. Consecutive edits to one field merge for 650ms. Profile switching and hiding the window preserve drafts. Import is an undoable draft operation. The runtime reads only `ConfigManager`'s committed configuration.

Apply validates non-empty shortcuts/URLs, unique IDs, two-level hierarchy, triggers, and safe geometry. Empty targets and legacy None actions are valid placeholders. `PieItem::IsNoOp` lets the executor skip them before handler validation or dispatch, without a failure notification. `QSaveFile` writes atomically with direct-write fallback disabled. Only a successful commit replaces the runtime configuration and emits `ConfigChanged`. Parse/type errors, invalid imports, write errors, and failed file replacement retain the draft and original file. Configuration JSON remains version 1.0; legacy appearance aliases and existing custom colors are supported. Fresh configurations use Frost at approximately 96% effective sector opacity. Existing saved styles retain their values until edited or replaced by a preset.

Appearance writes always target the current profile's `StyleOverride`, including presets. `GlobalStyle` remains a read-only fallback for legacy profiles and seeds an independent style for newly created profiles. The first local edit copies the effective legacy style before changing a field; loading alone does not rewrite files or mark the draft dirty. Duplicating a profile copies its effective style. There is no shared/custom switch in the UI. Undo/redo, discard, apply, import, reload, profile switching, and runtime rendering preserve this isolation.

## UI and rendering

`SettingsWindow` owns the QML engine, session, recorder, and icon catalog. QML provides the toolbar, canvas, contextual inspector, organizer, and compact drawer. Shared buttons, menus, tooltips, fields, checkboxes, sliders, switches, and dialogs use `Theme.js` tokens. The three-dot menu toggles and suppresses its button tooltip while open. Apply/Discard keep their toolbar positions when disabled. Advanced controls do not replace the canvas. Submenu containers cannot be nested or converted to actions while they contain children.

`AppDialog` supplies the white rounded surface, modal focus, scrollable body, and fixed footer for settings, about, icon selection, color selection, running applications, and exit confirmation. `RunningApplications` only enumerates process names; QML handles search and selection. Quit confirmation is asynchronous, and a failed Apply leaves its dialog and draft open. File/folder selection remains a native dialog; the application requests the light color scheme. The duplicate-instance startup notice remains a light QWidget message box.

`ColorWheelItem` paints and hit-tests the hue ring and saturation/value square, retaining hue for achromatic colors. `ColorPicker` stages H/S/V/alpha and RGB/RGBA hex changes until confirmation; configuration colors retain Qt's ARGB representation. Selection IDs guard against applying a delayed result to a different item. The last 12 confirmed colors are stored separately in QSettings; isolated preview/test sessions do not write that history.

`WindowTitleBar` and `WindowResizeHandles` provide system move/resize with shared, unfilled caption buttons. Windows DWM supplies rounded corners where supported. Native maximize/restore preserves the Windows state, including taskbar-free monitors where Qt 6.9's frameless maximization can report FullScreen. The organizer tracks the dragged row by ID, draws an insertion gap, and commits one move on release; edge scrolling and Escape cancellation keep the draft stable during the gesture.

Reordering within a container preserves selection, including an empty selection. Moving an action to another container still follows the moved item. `ActionContextMenu` is shared by preview sectors, preview submenu rows, and organizer rows; it targets the clicked ID without selecting it first. Deleting a different action preserves the current selection. The preview hint does not depend on the Enabled switch.

The QWidget tray menu matches the QML menu's light surface, spacing, corner radii, and hover color. Application and tray ICO resources come from the supplied Pie-Menu-SVG-Pack, with the application SVG master retained under `resources/logo`. The GitHub button beside About uses the verified repository URL.

`PieScene` and `ListScene` share drawing, colors, geometry, and hit testing between `PiePreviewItem` and the runtime QWidget menus. Icons are supplied before painting. The preview composes its logical transform with the Qt Quick device-pixel transform and uses fixed label bounds centered on the menu origin, so renaming does not move or scale the menu. Runtime bounds still follow rendered text. Window placement accounts for negative monitor coordinates, label extents, screen edges, and small work areas. Long submenu lists page with the wheel or edge scrolling. Paint requests occur on changes, animation, or input; there is no idle repaint timer.

## Input and asynchronous work

`InputState` is a pure state machine behind the Win32 hook adapter. It handles sided modifiers, repeat suppression, matching swallowed releases, app-specific priority, hold/toggle confirmation, and Escape cancellation. The active trigger is a value snapshot. Self-generated input carries `OwnInputTag`, so sending an action does not retrigger the hook. Recording suspends hooks. Runtime widgets consume a selection before dispatch and reject input while closing.

The editor exposes Ctrl, Shift, Alt, and Win independently from activation mode, plus left/right/middle/X1/X2 mouse buttons. Keyboard recording preserves the chosen keyboard mode and does not convert a mouse-hold trigger to a keyboard trigger.

`IconCatalog` indexes filenames once and filters metadata without decoding thumbnails. The QML grid creates visible cells and two nearby rows. Search is coalesced for one frame (16ms). Managed worker pools load images; cancelled responses still finish, and stale generation results are ignored. The decoded-image cache is capped at 64 MiB and the GUI-ready cache at 16 MiB. Keys distinguish file modification time, raster dimensions (including DPI), and tint.

Resolution order is the writable application-data icon folder, the legacy executable-adjacent `icons` folder, then embedded resources. Absolute paths remain supported. Filesystem changes refresh the catalog and invalidate images. Engine shutdown waits for managed work. The response's final signal runs on its owning thread, following [Qt's response lifetime contract](https://doc.qt.io/qt-6/qquickimageresponse.html).

## Automated coverage

Run `scripts/build.ps1` as described in the README. CTest runs core once and the Windows QML UI at scale factors 1, 1.25, 1.5, and 2. Tests use temporary configurations, disable startup registry changes, and never execute actions.

- Core: draft isolation across profiles; undo/redo/apply/discard; safe selection after deletion; invalid imports and malformed persisted files; creation and atomic-replacement failures; legacy JSON; hierarchy-preserving moves; trigger state, including left/right Win modifiers with mouse and keyboard triggers; injected events; shortcut parsing.
- UI: canvas selection, typing, shortcut/trigger recording, profile creation/deletion and selector synchronization, menu toggle/tooltip behavior, stable preview geometry after renaming, icon centering/search/selection, whole-row drag insertion at both boundaries, autoscroll and cancellation, submenu editing, appearance, and Apply/hide/reopen.
- Dialog/window UI: color conversion, ring/square pointer input, hex validation and focus, confirm/cancel isolation and recent colors, modal shortcut blocking, unsaved exit success/failure/cancel, enabled switch, maximize/work-area/restore, running-app selection dialog, and compact color layout. Existing coverage includes the 800×520 drawer, QML warning detection, high-DPI painted coordinates, four-corner/negative-screen placement, a 100-item list, rapid menu reopen, and closing engines during icon loading.
- Packaging: `windeployqt --qmldir src/ui/qml` includes QML dependencies. `--smoke-test` uses preview isolation and exits after startup. CI tests the deployed executable with the Qt SDK removed from PATH, then compiles the installer. Screenshots and text logs are retained as artifacts.

Additional checks cover independent profile appearance (including legacy defaults and custom styles), blank-action save/import, no-op dispatch through fake action handlers, selection-preserving sorting/deletion, all three action context-menu locations, disabled-menu hints, and tray Pause behavior. The GitHub test intercepts URL opening rather than launching a browser. Action dispatch is built as `gpm_actions` so the same production dispatcher can be tested without executing real actions.

## Local Release measurements

Measured on 2026-10-09, Windows 11, Qt 6.9.0 / MSVC 2022, software Qt Quick rendering, 1,377 embedded icons. Same development machine, four scale factors; warm opening sampled three times per scale.

| Operation | Observed range | Target |
| --- | ---: | ---: |
| Warm picker open and first rendered frame | 40–60ms | <150ms |
| Search text through filtered model, including 16ms coalescing | 21–25ms | <100ms |
| Cold metadata index | 51–65ms | Background, UI remains usable |
| Metadata filter alone | 0.089–0.153ms | <100ms |

Warm timing includes the test helper's 30ms layout wait. These are local samples, not latency guarantees or cross-device benchmarks. Logs are in `build/artifacts`; timing values are reported, not enforced as brittle CI thresholds.

The local Release build and all five CTest entries passed. Deployed startup passed without an SDK PATH, and the Inno Setup installer compiled successfully. The GitHub workflow has not been run remotely in this work session. Real keyboard/mouse feel in third-party apps, Windows-key combinations interacting with Start/system shortcuts, elevated target apps, and physically moving between monitors with different Windows scaling settings still require hands-on acceptance; uniform scale-factor tests do not simulate that hardware transition.
