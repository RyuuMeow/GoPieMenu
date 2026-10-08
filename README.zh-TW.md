# GoPieMenu

[English](README.md) · [繁體中文](README.zh-TW.md) · [简体中文](README.zh-CN.md)

Windows 圓形快捷選單，可錄製快捷鍵、啟動應用程式、開啟檔案／網址與執行指令。

![淺色編輯器](resources/demo/Editor2.png)

## 使用方式

1. 從系統匣開啟 **Settings**。頂部切換設定檔，旁邊的管理選單可新增、複製或刪除。
2. 點擊扇形直接編輯；**Add action** 新增動作或子選單。快捷鍵直接錄製，檔案／應用程式直接選取，圖示可搜尋。
3. 展開 **Arrange actions** 排序。雙擊子選單或按 **Edit actions**，即可在原工作區編輯；維持兩層結構。
4. 點擊頂部觸發方式，修改快捷鍵、啟用狀態與應用程式範圍。新增及複製的設定檔預設停用，完成後再啟用。
5. 草稿即時顯示在畫布，按 **Apply** 驗證並保存成功後才影響實際選單。可用 **Discard** 捨棄，或 Ctrl+Z／Ctrl+Y 復原／重做。

切換設定檔保留草稿。關閉主視窗會隱藏至系統匣；退出時會處理未套用變更。匯入先成為可復原的草稿，損壞檔案和保存失敗不會覆蓋目前設定。

預設觸發為 **Ctrl＋滑鼠右鍵**：按住、移向項目、放開執行。亦支援鍵盤按住及切換模式；切換模式以左鍵確認，Escape 取消。符合目前應用程式的設定檔優先於全域設定檔。

## 外觀與圖示

主視窗使用淺色 Frost 風格，既有 Pie 自訂配色保留。**Appearance** 保留畫布，提供預設樣式與大小；幾何、配色、動畫等放在 Advanced。

圖示選擇器提供搜尋、網格與清除。內建 Iconoir 圖示直接嵌入程式；自訂 SVG、PNG、JPEG、ICO 可放進 Settings 開啟的圖示資料夾。

JSON 設定格式維持 1.0，存於 Qt 應用程式資料目錄；Settings 可匯入／匯出。

## 建置與驗證

需要 Windows、Visual Studio 2022 C++、CMake 3.21+、Qt 6.9（Widgets、Quick、QML、Quick Controls、SVG、Test）。

```powershell
./scripts/build.ps1 -QtRoot "C:/Qt/6.9.0/msvc2022_64"
```

指令會建置 Release，執行核心及 100%、125%、150%、200% 縮放的 UI 測試，截圖輸出至 `build/artifacts`。增量建置可加 `-SkipTests`。

執行時將 Qt bin 加到 PATH，開啟 `build/Release/GoPieMenu.exe --settings`。參數 `--preview` 使用暫存設定，不安裝全域掛鉤、不執行動作，也不更動 Windows 啟動設定。

封裝 QML 需使用 `windeployqt --release --qmldir src/ui/qml --no-translations deploy/GoPieMenu.exe`。CI 已包含建置、回歸測試、截圖及安裝程式封裝。

架構、測試範圍、效能數據及尚需實機驗證項目，見 [架構與驗證](docs/architecture.md)。

## 授權

GoPieMenu 使用 [GPL-3.0](LICENSE)。內建圖示來自 [Iconoir](https://github.com/iconoir-icons/iconoir)，採 MIT 授權。
