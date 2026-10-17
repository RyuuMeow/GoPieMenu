# Changelog

## [Unreleased]

## [2.2.0]

### English
<!-- gpm:lang en -->

#### What's new

- Choose **Persistent floating menu** for any profile. It appears automatically for the foreground application, supports repeated click actions, and can be moved by dragging its center.
- Click a submenu sector to open or close its list. **Collapse submenu after click** optionally closes the list after selecting a child action while keeping the pie visible; it is off by default.
- Application-specific menus take priority over global menus across both display modes. Only one main menu is shown. Hide all persistent menus from the center context menu or tray; positions, expansion, and the hidden state are remembered.
- Set text size, icon size, text outline, and border width independently for the pie and submenus. Colors, font family, opacity, and animation remain shared within each profile, with matching live previews.
- The installer completion page now offers a checked **開啟 GoPieMenu** option to launch the app. Uncheck it to finish without opening the app; silent installations never auto-launch it.

#### Fixes and compatibility

- Persistent pie and submenu clicks retain foreground focus so keyboard actions reach the active application. Switching applications refreshes the appropriate menu reliably.
- Existing configurations keep shortcut mode and their previous submenu appearance. The new submenu dimensions become independent when edited.

<!-- gpm:endlang -->

### 繁體中文
<!-- gpm:lang zh-TW -->

#### 新功能

- 每份選單都可選擇「常駐懸浮選單」，依前景程式自動顯示，可重複點擊執行動作，並拖曳中心移動位置。
- 點擊子選單扇形可展開或收起清單。新增「點擊後收起子選單」選項，勾選後執行子項目會收起清單，圓盤保持顯示；預設不勾選。
- 快捷與常駐模式共用應用程式專用選單優先於全域選單的規則，同時只顯示一個主選單。可從中心右鍵選單或系統匣隱藏所有常駐選單，並記住位置、展開與隱藏狀態。
- 圓盤本體與子選單可分別設定文字大小、圖示大小、文字外框與邊框寬度，並即時預覽。配色、字型、不透明度與動畫仍由同一份選單共用。
- 安裝完成頁新增預設勾選的「開啟 GoPieMenu」，可取消勾選以直接結束安裝；靜默安裝不會自動開啟程式。

#### 修正與相容性

- 常駐圓盤與子選單不再因點擊而丟失前景焦點，快捷鍵動作可送至目前應用程式；切換程式時也會正確更新對應選單。
- 既有設定保留快捷模式與原本的子選單外觀，之後可獨立調整子選單尺寸。

<!-- gpm:endlang -->

### 简体中文
<!-- gpm:lang zh-CN -->

#### 新功能

- 每份菜单都可选择“常驻悬浮菜单”，根据前台应用自动显示，可反复点击执行操作，并拖动中心移动位置。
- 点击子菜单扇形可展开或收起列表。新增“点击后收起子菜单”选项，勾选后执行子项目会收起列表，圆盘保持显示；默认不勾选。
- 快捷与常驻模式共用应用专用菜单优先于全局菜单的规则，同时只显示一个主菜单。可从中心右键菜单或系统托盘隐藏所有常驻菜单，并记住位置、展开与隐藏状态。
- 圆盘本体与子菜单可分别设置文字大小、图标大小、文字外框与边框宽度，并实时预览。配色、字体、不透明度与动画仍由同一份菜单共用。
- 安装完成页新增默认勾选的“開啟 GoPieMenu”，可取消勾选以直接结束安装；静默安装不会自动启动程序。

#### 修复与兼容性

- 常驻圆盘与子菜单不会因点击而丢失前台焦点，快捷键操作可发送到当前应用；切换应用时也会正确更新对应菜单。
- 已有配置保留快捷模式与原来的子菜单外观，之后可单独调整子菜单尺寸。

<!-- gpm:endlang -->

<!-- gpm:lang shared -->
### Validation

The release workflow builds and tests on Windows, including core and update regression tests, multilingual release-note extraction, and UI/native-input tests at 100%, 125%, 150%, and 200% scaling. It verifies deployed startup before packaging and publishing the installer with `SHA256SUMS.txt`.

Real input in third-party or elevated applications, pen/touch hardware, mixed-DPI monitor moves, and the final installation flow remain manual acceptance checks.

Full changes: https://github.com/RyuuMeow/GoPieMenu/compare/v2.1.0...v2.2.0

<!-- gpm:endlang -->

## [2.1.0]

### English
<!-- gpm:lang en -->

GoPieMenu now supports English, Traditional Chinese, and Simplified Chinese. New configurations follow the system language by default, falling back to English for unsupported languages. You can choose a language in Settings; apply the change and restart the app for it to take effect. Menu edits still require an explicit Apply, and unapplied changes are handled before the app exits.

#### What's new

- Chinese interfaces use Microsoft JhengHei UI. Changing language offers Later or a blue Restart button; Restart saves all pending edits first.
- Manual launches show the editor, including when already running. Windows autostart stays in the tray.

- GoPieMenu checks for stable releases automatically at startup. When an update is available, it shows a tray notification if the window is hidden or a dialog if the window is open.
- Release notes appear in the current UI language, with English used when a translation is unavailable.
- Skip a release to stop being notified about that version.
- Download the installer and verify its SHA256 checksum. Before installation, confirm the update and resolve any unapplied edits; GoPieMenu then exits so the installer can run.

<!-- gpm:endlang -->

### 繁體中文
<!-- gpm:lang zh-TW -->

GoPieMenu 現在支援英文、繁體中文與簡體中文。新設定預設依照系統語言顯示，不支援的語言則使用英文。你也可以在「設定」中選擇語言，套用後重新啟動應用程式生效。選單編輯仍須明確按下「套用」，結束應用程式前也會處理尚未套用的變更。

#### 新功能

- 中文介面使用微軟正黑體 UI。切換語言會提供「稍後」與藍色「重新啟動」按鈕，重新啟動前會套用所有待存變更。
- 手動開啟會顯示編輯器，已在背景執行時也會喚回；Windows 自動啟動則留在系統匣。

- GoPieMenu 會在啟動時自動檢查穩定版更新。若有更新可用，視窗隱藏時會顯示系統匣通知；視窗開啟時則會顯示對話方塊。
- 發行說明會以目前介面語言顯示；若缺少該語言的翻譯，則改用英文。
- 可以略過某個版本，之後不再收到該版本的通知。
- 下載安裝程式並驗證 SHA256 校驗碼。安裝前請確認要更新，並處理尚未套用的變更；接著 GoPieMenu 會結束，讓安裝程式執行。

<!-- gpm:endlang -->

### 简体中文
<!-- gpm:lang zh-CN -->

GoPieMenu 现在支持英语、繁体中文和简体中文。新配置默认跟随系统语言显示，不支持的语言则使用英语。你也可以在“设置”中选择语言，应用后重启程序生效。编辑菜单仍需明确点击“应用”，退出应用前也会处理尚未应用的更改。

#### 新功能

- 中文界面使用 Microsoft JhengHei UI。切换语言会提供“稍后”与蓝色“重新启动”按钮，重新启动前会应用所有待保存更改。
- 手动打开会显示编辑器，已在后台运行时也会唤回；Windows 自动启动则留在系统托盘。

- GoPieMenu 会在启动时自动检查稳定版更新。如果有可用更新，窗口隐藏时会显示系统托盘通知；窗口打开时则会显示对话框。
- 发布说明会以当前界面语言显示；如果没有对应语言的翻译，则使用英语。
- 可以跳过某个版本，之后不再收到该版本的通知。
- 下载安装程序并验证 SHA256 校验和。安装前请确认更新，并处理尚未应用的更改；随后 GoPieMenu 会退出，以便运行安装程序。

<!-- gpm:endlang -->

<!-- gpm:lang shared -->
### Validation

The local Release build and all seven CTest entries passed, including core and update regression tests, release-note extraction, and UI tests at 100%, 125%, 150%, and 200% scaling. Deployed startup with the SDK removed from PATH, TLS backend loading, and installer packaging also passed.

Language tests cover Later, save-before-restart, save failure, and launch failure at all four scales. A deployed process also passed the wait-for-parent restart check. Update tests use simulated network responses and installer launches. Actual Windows sign-in startup, notification delivery, and an elevated installation remain manual acceptance checks.

<!-- gpm:endlang -->

## [2.0.0]

### English
<!-- gpm:lang en -->

GoPieMenu 2.0 rebuilds the menu editor around a live preview, independent profile appearance, and an explicit Apply/Discard workflow.

#### What's new

- Edit actions directly from the pie preview. Switch profiles without losing drafts, and use Undo/Redo before applying changes to the running menu.
- Arrange actions by dragging an entire row, with insertion markers, edge scrolling, and selection preserved. Right-click preview or organizer actions to duplicate or delete them.
- Customize each profile independently: presets, size, colors, opacity, and animation. Duplicating a profile copies its appearance without linking later edits.
- Choose colors with a hue ring, saturation/value square, HSV/opacity controls, RGB/RGBA hex input, and recent colors.
- Use a searchable icon picker with asynchronous loading and embedded icons. Custom SVG, PNG, JPEG, and ICO files remain supported.
- Combine Ctrl, Shift, Alt, and Win modifiers independently with mouse triggers. Keyboard recording keeps the selected activation mode.
- Save empty actions as placeholders; triggering them does nothing.
- Enjoy a refreshed light editor, consistent dialogs and tray menu, new application artwork, and custom window controls.

#### Upgrading from 1.0

1. Export your configuration from Settings, then quit GoPieMenu from the tray before running the installer.
2. Download and run `GoPieMenu_Setup.exe` from this release. This is the Windows x64 installer; Qt dependencies are included.
3. Open Settings, review your profiles, and use **Apply** to save changes. Closing the editor hides it to the tray; quitting prompts about unapplied edits.

The configuration location and version 1.0 JSON format are retained. Existing appearance values and supported legacy style keys are preserved. Appearance edits now affect only the selected profile. Choose **Frost** to opt into the new, nearly opaque preset. Invalid configurations are reported without overwriting the original file; import is an undoable draft operation.

<!-- gpm:endlang -->

### 繁體中文
<!-- gpm:lang zh-TW -->

GoPieMenu 2.0 重新打造選單編輯器，提供即時預覽、各設定檔獨立的外觀，以及明確的「套用／捨棄」流程。

#### 新功能

- 直接在圓餅預覽中編輯動作。切換設定檔時草稿會保留；套用至執行中的選單前，可以使用復原／重做。
- 拖曳整列即可排列動作，並提供插入標記、邊緣捲動且保留選取狀態。在預覽或整理清單中的動作上按右鍵，即可複製或刪除。
- 為每個設定檔獨立自訂外觀：預設樣式、大小、顏色、不透明度與動畫。複製設定檔時會一併複製外觀，之後的編輯則彼此獨立。
- 使用色相環、飽和度／明度方塊、HSV／透明度控制項、RGB/RGBA 十六進位輸入及最近使用的顏色來選色。
- 使用可搜尋的圖示選擇器，支援非同步載入與內嵌圖示。仍可使用自訂 SVG、PNG、JPEG 與 ICO 檔案。
- 滑鼠觸發方式可各自搭配 Ctrl、Shift、Alt、Win 修飾鍵。錄製鍵盤快捷鍵時會保留已選的啟用模式。
- 可以將尚未設定的動作儲存為佔位項目；觸發這類項目不會執行任何操作。
- 編輯器改用清爽明亮的外觀，並統一對話方塊與系統匣選單樣式，加入全新應用程式圖示及自訂視窗控制按鈕。

#### 從 1.0 升級

1. 在「設定」中匯出設定，然後先從系統匣結束 GoPieMenu，再執行安裝程式。
2. 下載並執行此版本中的 `GoPieMenu_Setup.exe`。這是 Windows x64 安裝程式，已包含 Qt 相依元件。
3. 開啟「設定」並檢查各設定檔，然後按下「套用」儲存變更。關閉編輯器會將它隱藏至系統匣；結束程式時會詢問如何處理尚未套用的變更。

設定檔位置與 1.0 版 JSON 格式維持不變，既有外觀值及支援的舊版樣式索引鍵也會保留。外觀變更現在只會影響選取的設定檔。選擇 **Frost** 即可使用全新且接近不透明的預設樣式。遇到無效設定時會顯示錯誤，不會覆寫原始檔案；匯入設定則會建立可復原的草稿。

<!-- gpm:endlang -->

### 简体中文
<!-- gpm:lang zh-CN -->

GoPieMenu 2.0 重新打造了菜单编辑器，提供实时预览、各配置独立的外观，以及明确的“应用/放弃”流程。

#### 新功能

- 直接在饼形菜单预览中编辑操作。切换配置时草稿会保留；应用到正在运行的菜单前，可以使用撤销/重做。
- 拖动整行即可排列操作，并提供插入标记、边缘滚动且保留选中状态。在预览或整理列表中的操作上单击右键，即可复制或删除。
- 为每个配置单独自定义外观：预设样式、大小、颜色、不透明度和动画。复制配置时会一并复制外观，后续编辑则互不影响。
- 使用色相环、饱和度/明度方块、HSV/不透明度控件、RGB/RGBA 十六进制输入和最近使用的颜色来选色。
- 使用可搜索的图标选择器，支持异步加载和内嵌图标。仍可使用自定义 SVG、PNG、JPEG 和 ICO 文件。
- 鼠标触发方式可分别搭配 Ctrl、Shift、Alt、Win 修饰键。录制键盘快捷键时会保留已选的激活模式。
- 可以将尚未设置的操作保存为占位项；触发这类项目不会执行任何操作。
- 编辑器采用清爽明亮的新外观，并统一对话框和系统托盘菜单样式，加入全新应用图标及自定义窗口控制按钮。

#### 从 1.0 升级

1. 在“设置”中导出配置，然后先从系统托盘退出 GoPieMenu，再运行安装程序。
2. 下载并运行此版本中的 `GoPieMenu_Setup.exe`。这是 Windows x64 安装程序，已包含 Qt 依赖项。
3. 打开“设置”并检查各配置，然后点击“应用”保存更改。关闭编辑器会将其隐藏到系统托盘；退出程序时会询问如何处理尚未应用的更改。

配置文件位置和 1.0 版 JSON 格式保持不变，已有外观值及受支持的旧版样式键也会保留。外观更改现在只会影响选中的配置。选择 **Frost** 即可使用全新且近乎不透明的预设样式。遇到无效配置时会显示错误，不会覆盖原始文件；导入配置会创建可撤销的草稿。

<!-- gpm:endlang -->

<!-- gpm:lang shared -->
### Validation

The release workflow runs core tests and UI tests at 100%, 125%, 150%, and 200% scaling, then checks that the deployed application starts without the Qt SDK on PATH before compiling the installer. `SHA256SUMS.txt` contains the installer's checksum.

Real input behavior in individual applications, interactions between Win-key combinations and Windows system shortcuts, elevated applications, and moving between monitors with different physical scaling settings remain manual verification areas.

Full changes: https://github.com/RyuuMeow/GoPieMenu/compare/v1.0.0...v2.0.0

<!-- gpm:endlang -->
