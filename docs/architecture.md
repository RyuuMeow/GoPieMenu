# Editor architecture and validation

## State and persistence

`EditorSession` owns the draft, stable profile/item IDs, selection, and up to 100 undo snapshots. Consecutive edits to one field merge for 650ms. Profile switching and hiding the window preserve drafts. Import is an undoable draft operation. The runtime reads only `ConfigManager`'s committed configuration.

Apply validates action requirements, unique IDs, two-level hierarchy, triggers, and safe geometry. `QSaveFile` writes atomically with direct-write fallback disabled. Only a successful commit replaces the runtime configuration and emits `ConfigChanged`. Parse/type errors, invalid imports, write errors, and failed file replacement retain the draft and original file. Configuration JSON remains version 1.0; legacy appearance aliases and existing custom colors are supported. Fresh configurations use Frost.

## UI and rendering

`SettingsWindow` owns the QML engine, session, recorder, and icon catalog. QML provides the single toolbar, canvas, contextual inspector, organizer, and compact drawer. Native file/color/application dialogs remain C++. Advanced controls do not replace the canvas. Submenu containers cannot be nested or converted to actions while they contain children.

`PieScene` and `ListScene` share drawing, colors, geometry, and hit testing between `PiePreviewItem` and the runtime QWidget menus. Icons are supplied before painting. The preview composes its logical transform with the Qt Quick device-pixel transform. Window placement accounts for negative monitor coordinates, label extents, screen edges, and small work areas. Long submenu lists page with the wheel or edge scrolling. Paint requests occur on changes, animation, or input; there is no idle repaint timer.

## Input and asynchronous work

`InputState` is a pure state machine behind the Win32 hook adapter. It handles sided modifiers, repeat suppression, matching swallowed releases, app-specific priority, hold/toggle confirmation, and Escape cancellation. The active trigger is a value snapshot. Self-generated input carries `OwnInputTag`, so sending an action does not retrigger the hook. Recording suspends hooks. Runtime widgets consume a selection before dispatch and reject input while closing.

`IconCatalog` indexes filenames once and filters metadata without decoding thumbnails. The QML grid creates visible cells and two nearby rows. Search is coalesced for one frame (16ms). Managed worker pools load images; cancelled responses still finish, and stale generation results are ignored. The decoded-image cache is capped at 64 MiB and the GUI-ready cache at 16 MiB. Keys distinguish file modification time, raster dimensions (including DPI), and tint.

Resolution order is the writable application-data icon folder, the legacy executable-adjacent `icons` folder, then embedded resources. Absolute paths remain supported. Filesystem changes refresh the catalog and invalidate images. Engine shutdown waits for managed work. The response's final signal runs on its owning thread, following [Qt's response lifetime contract](https://doc.qt.io/qt-6/qquickimageresponse.html).

## Automated coverage

Run `scripts/build.ps1` as described in the README. CTest runs core once and the Windows QML UI at scale factors 1, 1.25, 1.5, and 2. Tests use temporary configurations, disable startup registry changes, and never execute actions.

- Core: draft isolation across profiles; undo/redo/apply/discard; safe selection after deletion; invalid imports and malformed persisted files; creation and atomic-replacement failures; legacy JSON; hierarchy-preserving moves; trigger state; injected events; shortcut parsing.
- UI: canvas selection, typing, shortcut/trigger recording, icon search and selection, pointer drag sorting, submenu editing, appearance, Apply, hide/reopen, 800×520 drawer, QML warning detection, high-DPI painted coordinates, four-corner/negative-screen placement, a 100-item list, rapid menu reopen, and closing engines during icon loading.
- Packaging: `windeployqt --qmldir src/ui/qml` includes QML dependencies. `--smoke-test` uses preview isolation and exits after startup. CI tests the deployed executable with the Qt SDK removed from PATH, then compiles the installer. Screenshots and text logs are retained as artifacts.

## Local Release measurements

Measured on 2026-10-09, Windows 11, Qt 6.9.0 / MSVC 2022, software Qt Quick rendering, 1,371 embedded icons. Same development machine, four scale factors; warm opening sampled three times per scale.

| Operation | Observed range | Target |
| --- | ---: | ---: |
| Warm picker open and first rendered frame | 48–71ms | <150ms |
| Search text through filtered model, including 16ms coalescing | 5–31ms | <100ms |
| Cold metadata index | 52–63ms | Background, UI remains usable |
| Metadata filter alone | 0.094–0.131ms | <100ms |

Warm timing includes the test helper's 30ms layout wait. These are local samples, not latency guarantees or cross-device benchmarks. Logs are in `build/artifacts`; timing values are reported, not enforced as brittle CI thresholds.

The local Release build and all five CTest entries passed. Deployed startup passed without an SDK PATH, and the Inno Setup installer compiled successfully. The GitHub workflow has been updated but has not been run remotely in this work session. Real keyboard/mouse feel in third-party apps, elevated target apps, and physically moving between monitors with different Windows scaling settings still require hands-on acceptance; uniform scale-factor tests do not simulate that hardware transition.
