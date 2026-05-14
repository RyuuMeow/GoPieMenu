# GoPieMenu

<p align="center">
  <a href="README.md">English</a> |
  <a href="README.zh-TW.md">繁體中文</a> |
  <a href="README.zh-CN.md">简体中文</a>
</p>

<p align="center">
  <b>一款快速、可自定义的 Windows 放射状（圆盘）菜单。</b><br>
  灵感来自 Blender 直观的交互设计。
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-Windows-blue">
  <img src="https://img.shields.io/badge/language-C++20-blue">
  <img src="https://img.shields.io/badge/framework-Qt%206.9-green">
  <img src="https://img.shields.io/github/license/RyuuMeow/GoPieMenu">
</p>

---

## 🎬 Demo

![GoPieMenu Demo](resources/demo/GoPieMenu_Demo.gif)

![GoPieMenu Demo](resources/demo/Demo1.gif)

---

## 📖 概览

**GoPieMenu** 是一款强大且高度可自定义的 **Windows 放射状（圆盘）菜单工具**。
它可以让你直接从 **光标位置** 触发快捷键、启动应用程序或执行命令，比传统菜单带来更快的工作流程。

设计深受 **Blender** 高效圆盘菜单启发，将同样的交互方式带到任何 Windows 应用程序中。

---

### 🚀 为什么选择 GoPieMenu？

| 功能 | 传统菜单 | GoPieMenu |
| :--- | :--- | :--- |
| **速度** | 慢（需要精确点击） | **即时**（肌肉记忆） |
| **专注** | 视线移到任务栏/功能区 | **保持在光标中心** |
| **自定义** | 由开发者固定 | **JSON 完全可编程** |
| **场景** | 仅全局 | **应用程序专属**配置文件 |

---

## 📷 界面预览

![GoPieMenu Demo](resources/demo/Demo1.png)
![GoPieMenu Demo](resources/demo/Demo2.png)
![GoPieMenu Demo](resources/demo/Demo3.png)

---

## ✨ 功能特性

### 🎯 场景感知配置文件

为不同应用程序创建不同的圆盘菜单。

示例：

* Photoshop → 画笔 / 图层快捷键
* VS Code → 构建 / 运行 / 终端
* 浏览器 → 标签页管理

---

### ⚡ 多种启动模式

选择最适合你工作流程的交互方式：

**鼠标长按**

* 按住鼠标按键
* 移动到菜单切片
* 松开即可执行

**键盘长按**

* 按住键盘按键
* 移动以选择
* 松开按键即可执行
* *（无需鼠标点击）*

**按键切换**

* 按下按键打开菜单
* 移动以选择
* 点击即可执行

---

### 🎨 现代化界面

* 极简深色主题
* 流畅动画
* 玻璃拟态视觉风格
* 清晰的放射状布局

---

### 🧠 智能窗口选择器

从当前运行中的窗口快速选择，将菜单绑定到特定应用程序。

示例：

```
photoshop.exe
code.exe
chrome.exe
```

---

### 🧩 图标支持

内置支持搜索的 **Notion 风格图标选择器**。

你可以：

* 搜索图标
* 指派自定义图标
* 以可视化方式整理操作

---

### 💾 导入 / 导出

配置会保存为 **JSON**。

你可以轻松：

* 备份设置
* 与他人分享配置文件
* 将配置纳入版本控制

---

### 🚀 高性能

使用 **底层 Win32 hooks** 以确保：

* 极快的输入检测
* 可靠的触发
* 极低的额外开销

---

## 🚀 使用方式

### 1️⃣ 打开设置

右键点击 **GoPieMenu 系统托盘图标** 并选择：

```
Settings
```

---

### 2️⃣ 创建配置文件

新增配置文件并设置 **触发条件**。

触发条件示例：

* `Ctrl + Mouse Right Button`
* `Ctrl + Shift + M`

---

### 3️⃣ 添加菜单项

定义每个切片要执行的内容：

* 启动应用程序
* 发送快捷键
* 打开 URL
* 运行命令

---

### 4️⃣ 设置应用程序筛选器（可选）

限制菜单只在特定应用程序中使用。

示例：

```
chrome.exe
photoshop.exe
AnyApp.exe
```

圆盘菜单只会在该应用程序获得焦点时显示。

---

### 5️⃣ 启动

使用你的触发条件，并移动鼠标选择切片。

享受 **更快速的工作流程** 🚀

---

## 🛠 从源码构建

GoPieMenu 使用 **C++20** 与 **Qt 6.9** 构建。

### 要求

* Visual Studio 2022 (MSVC)
* CMake 3.16+
* Qt 6.9.0 或更新版本

---

### 克隆仓库

```bash
git clone https://github.com/RyuuMeow/GoPieMenu.git
cd GoPieMenu
```

---

### 配置 CMake

```bash
mkdir build
cd build

cmake .. -DCMAKE_PREFIX_PATH="C:/Path/To/Qt/6.9.0/msvc2022_64"
```

---

### 构建

```bash
cmake --build . --config Release
```

---

## 📁 项目结构

```
src/
 ├─ core/      Win32 hooks 与操作执行
 ├─ models/    数据结构与 JSON 序列化
 └─ ui/        Qt 界面与放射状菜单渲染
```

---

## 🤝 贡献

欢迎贡献！

你可以通过以下方式协助：

* 报告错误
* 建议功能
* 提交 pull request
* 改进文档

如果你觉得 GoPieMenu 实用，欢迎在 GitHub 上给这个项目一颗 ⭐。

---

## 🙏 致谢

默认图标由 **Iconoir** 提供
https://github.com/iconoir-icons/iconoir

---

## 📜 许可证

本项目采用 **GNU General Public License v3.0 (GPL-3.0)** 许可。

你可以依照 GPL-3.0 许可条款自由使用、修改和分发本软件。

完整许可条款请见：
https://www.gnu.org/licenses/gpl-3.0.html
