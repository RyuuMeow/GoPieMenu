# GoPieMenu 功能与设置导览

简体中文视频草稿，1920 × 1080 / 30 fps / 41 秒。主合成为 `GoPieMenu-Bilibili`。全片统一采用白色、浅灰、蓝色强调和深灰文字，对齐软件界面。Slate、Ocean、Obsidian 的深色仅出现在真实菜单预设中。

每个镜头只保留一个短标题与一个操作焦点，不叠加段落说明、步骤编号或重复标语。主要设置段以 React / SVG 重建为简化操作示意：下拉展开、增加扇形、录入按键、拖动让位、输入应用名、开关切换、Apply 完成反馈。复合主题按时间拆成单独镜头。风格段采用源码预设色值连续插值，直接改变矢量菜单外观。详细说明保留在旁白稿，不铺满画面。

短字采用约 0.2 秒整体入场，标题位置跟随主角或操作控件：开场左右呼应、手势按拍切字、设置提示靠近高亮、风格名与菜单并置、结尾居中。短段落 1–2 秒完成一个动作，复杂设置保留辨识时间。

## 启动与验证

在 `video` 目录运行 `npm ci`，然后 `npm run dev -- --no-open --port=3001`。预览地址为 http://localhost:3001/GoPieMenu-Bilibili 。独立于其他 Remotion 示例项目。

`npm run lint` 检查 ESLint 与 TypeScript；`npm run build` 验证 bundle；`npm run render` 输出 `out/gopiemenu-bilibili.mp4`（H.264）。目前交付为 Studio 草稿，未导出或发布。已加入背景配乐与操作音效；无配音与最终逐句字幕。

## 配乐与音效

`public/audio/` 为本项目原创程序合成素材，无外部采样。120 BPM 轻电子背景包含柔和键盘和弦、低音、轻打击与短旋律；pop、tick、swipe、confirm 分别对应弹出、选择、滑动及完成。运行 `npm run audio` 可用 Node 标准库重建全部 WAV，生成源码为 `scripts/generate-audio.mjs`。

`src/Soundtrack.tsx` 管理混音音量和相对场景的帧级音效时点。背景音量为 0.48，音效为 0.3–0.5；音乐开头淡入、末尾 1.5 秒淡出。检测到某段旁白 WAV 时，该段背景自动降低 55%，边界平滑过渡。独立场景仍用于画面编辑；声音在主合成播放。

## 导览顺序与配音槽

| 场景 / WAV 文件名 | 时间     | 介绍                             |
| ----------------- | -------- | -------------------------------- |
| hook              | 0–2 秒   | 产品开场                         |
| gesture           | 2–5 秒   | 默认按住、划过、松开             |
| overview          | 5–8 秒   | 切换、新建、管理菜单             |
| actions           | 8–12 秒  | Add action 与右侧动作设置        |
| arrange           | 12–14 秒 | Arrange actions 拖动排序         |
| submenu           | 14–20 秒 | 分类、子动作与二级菜单编辑       |
| trigger           | 20–24 秒 | 触发方式、快捷键、指定应用       |
| style             | 24–30 秒 | Frost → Slate → Ocean → Obsidian |
| settings          | 30–33 秒 | 开机启动、导入导出、自定义图标   |
| apply             | 33–35 秒 | 预览、Apply、Discard             |
| workflow          | 35–38 秒 | 返回绘图工作流                   |
| outro             | 38–41 秒 | 项目入口                         |

旁白与时长的单一来源为 `src/timeline.ts`，主合成、独立场景和配音起点自动从这里生成。组件映射在 `src/sceneRegistry.ts`。GPT-SoVITS 生成的 PCM WAV 放入 `public/voice/<场景名>.wav`，缺失旁白文件时保留配乐和音效。音频必须短于对应场景，建议留 0.5 秒余量；实际配音完成后再调整节奏和字幕。独立场景不包含旁白。

## 素材与真实性

- 主导览不再使用静态界面截图。`DemoUI.tsx` 提供可动画的控件与菜单，`Tour.tsx` 和 `Submenu.tsx` 编排操作演示。画面标注「操作示意」，不是录屏或对原生动画的承诺。
- Frost / Slate / Ocean / Obsidian 色值来自 `StyleConfig.h` 与 `EditorSession::setStylePreset`，使用矢量插值演示变化；结构和标签为减少阅读负担做了简化。
- 展示的选单管理、添加动作、快捷键、排序、子菜单、应用匹配、开机启动、导入导出与 Apply 均依据实际功能。所有演出仅发生在影片，不操作用户设置或文件。
- `public/assets/preset-*.png`、`tour-*.png`、`submenu-*.png` 留作取材参考，主合成不再引用。
- Logo 与唯一保留的实际绘图 GIF 来自本仓库 `resources`。GIF 源为 720 × 553，发布前可替换高清录屏。
- 默认手势示意使用 Ctrl + 右键，触发设置演示使用可自定义的 F12。
- 中文字体优先使用 Microsoft YaHei，跨系统渲染需安装对应字体或 Noto Sans CJK SC。

## 发布文案草稿

标题：给鼠标装一个快捷菜单！快速上手 GoPieMenu

简介：从新建菜单、添加动作，到二级菜单、触发键与应用匹配，再到 Frost、Slate、Ocean、Obsidian 四种外观预设。快速认识 GoPieMenu 的简单设置界面。Windows 开源项目，下载和说明：https://github.com/RyuuMeow/GoPieMenu 。绘图软件仅为使用场景示例。
