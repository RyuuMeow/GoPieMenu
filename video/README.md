# GoPieMenu 功能与设置导览

简体中文视频草稿，1920 × 1080 / 30 fps / 59 秒。主合成为 `GoPieMenu-Bilibili`。全片统一采用白色、浅灰、蓝色强调和深灰文字，对齐软件界面。Slate、Ocean、Obsidian 的深色仅出现在真实菜单预设中。

## 启动与验证

在 `video` 目录运行 `npm ci`，然后 `npm run dev -- --no-open --port=3001`。预览地址为 http://localhost:3001/GoPieMenu-Bilibili 。独立于其他 Remotion 示例项目。

`npm run lint` 检查 ESLint 与 TypeScript；`npm run build` 验证 bundle；`npm run render` 输出 `out/gopiemenu-bilibili.mp4`（H.264）。目前交付为 Studio 草稿，未导出或发布。无配音、背景音乐与最终逐句字幕。

## 导览顺序与配音槽

| 场景 / WAV 文件名 | 时间     | 介绍                             |
| ----------------- | -------- | -------------------------------- |
| hook              | 0–4 秒   | 产品开场                         |
| gesture           | 4–8 秒   | 默认按住、划过、松开             |
| overview          | 8–13 秒  | 切换、新建、管理菜单             |
| actions           | 13–19 秒 | Add action 与右侧动作设置        |
| arrange           | 19–22 秒 | Arrange actions 拖动排序         |
| submenu           | 22–31 秒 | 分类、子动作与二级菜单编辑       |
| trigger           | 31–36 秒 | 触发方式、快捷键、指定应用       |
| style             | 36–44 秒 | Frost → Slate → Ocean → Obsidian |
| settings          | 44–48 秒 | 开机启动、导入导出、自定义图标   |
| apply             | 48–51 秒 | 预览、Apply、Discard             |
| workflow          | 51–55 秒 | 返回绘图工作流                   |
| outro             | 55–59 秒 | 项目入口                         |

旁白与时长的单一来源为 `src/timeline.ts`，主合成、独立场景和配音起点自动从这里生成。组件映射在 `src/sceneRegistry.ts`。GPT-SoVITS 生成的 PCM WAV 放入 `public/voice/<场景名>.wav`，缺失文件保持静音。音频必须短于对应场景，建议留 0.5 秒余量；实际配音完成后再调整节奏和字幕。独立场景不包含旁白。

## 素材与真实性

- `preset-frost/slate/ocean/obsidian.png` 来自本次在实际 GoPieMenu 的 Appearance 中逐一点击预设后的截图。使用擦除动画展示切换，不是连续录屏，也不是用示意圆环替代预设。
- `tour-*.png` 来自实际菜单管理、Add action、排序、触发设置与全局设置。`submenu-*.png` 来自实际 ClipStudioPaint 配置的图层分类与新图层动作设置。
- 取材前无未保存变更；四种预设仅用于预览，完成后用 Discard 还原，未 Apply 或改动用户已保存配置。其余设置只导航和读取。
- 软件截图搭配高亮框、推近和切换动画；不将高亮标注描述为软件原生动画。Apply 段解释操作，未实际提交设置。
- 开场、默认手势和二级菜单前半段为标注的动效示意。实际配置使用 F12，默认手势示意使用 Ctrl + 右键，影片分别说明。
- Logo 与绘图 GIF 来自本仓库 `resources`；GIF 源为 720 × 553，发布前可替换高清录屏。
- 中文字体优先使用 Microsoft YaHei，跨系统渲染需安装对应字体或 Noto Sans CJK SC。

## 发布文案草稿

标题：给鼠标装一个快捷菜单！一分钟上手 GoPieMenu

简介：从新建菜单、添加动作，到二级菜单、触发键与应用匹配，再到 Frost、Slate、Ocean、Obsidian 四种外观预设。一分钟认识 GoPieMenu 的简单设置界面。Windows 开源项目，下载和说明：https://github.com/RyuuMeow/GoPieMenu 。绘图软件仅为使用场景示例。
