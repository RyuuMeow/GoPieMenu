# GoPieMenu 功能展示影片

B 站优先的简体中文初稿，1920 × 1080 / 30 fps / 41 秒。主合成为 `GoPieMenu-Bilibili`，七个场景也可在 Studio 的 Scenes 文件夹里单独编辑。新版采用弹性菜单、逐字入场、指针轨迹、点击波纹、局部推近、快切、扇形分解、透视列表展开与分层斜切转场；一般段落约 4–6 秒，二级菜单段 9 秒。

```powershell
cd video
npm ci
npm run dev -- --no-open --port=3001
```

打开 http://localhost:3001/GoPieMenu-Bilibili 。本项目独立于 `C:/Users/guguc/GitHub/remotion/packages/example`，不修改原有 Studio 范例。

## 验证与导出

`npm run lint` 执行 ESLint 与 TypeScript 检查；`npm run build` 验证视频 bundle。`npm run render` 输出 H.264 MP4 至 `out/gopiemenu-bilibili.mp4`，可作为 B 站与 YouTube 的横版上传素材。当前交付为无旁白的 Studio 初稿，尚未发布或导出成片。

## GPT-SoVITS 配音

旁白稿、段落起点和时长的单一来源是 `src/timeline.ts`。逐段生成 PCM WAV 后放入 `public/voice/`，文件名为 `hook.wav`、`gesture.wav`、`submenu.wav`、`actions.wav`、`workflow.wav`、`style.wav`、`outro.wav`。主合成自动发现并加载音频；缺失文件的段落保持静音。独立场景合成用于画面编辑，不包含主合成的旁白轨。

| 文件名       | 时间     | 建议音频最长时长 |
| ------------ | -------- | ---------------- |
| hook.wav     | 0–4 秒   | 3.5 秒           |
| gesture.wav  | 4–9 秒   | 4.5 秒           |
| submenu.wav  | 9–18 秒  | 8.5 秒           |
| actions.wav  | 18–24 秒 | 5.5 秒           |
| workflow.wav | 24–30 秒 | 5.5 秒           |
| style.wav    | 30–36 秒 | 5.5 秒           |
| outro.wav    | 36–41 秒 | 4.5 秒           |

这是初步节奏，收到实际语音后再按自然语速调整场景。超出片段长度的语音会被截断，不能直接使用。调整时同步 `timeline.ts`、`Promo.tsx` 的 Series 时长，以及 `Root.tsx` 的主合成与独立场景时长。中文字体优先使用本机 Microsoft YaHei；跨系统渲染需安装对应字体或 Noto Sans CJK SC。

## 素材与真实性

- `submenu-parent.png`、`submenu-live.png`、`submenu-editor.png` 是本次直接从已打开的 GoPieMenu 捕获的 ClipStudioPaint 配置画面，依次展示图层分类、五项子动作、新图层快捷键设置。仅导航和选择，没有更改或 Apply 配置。剪辑为截图快切与镜头动画，并非连续录屏。
- 其余 `public/assets/` 来自本仓库 `resources/demo/` 和 `resources/logo/`，保留真实界面与原有演示内容。
- 编辑器与选色器使用新版浅色截图；绘图演示是现有 720 × 553 GIF，清晰度受源文件限制，发布前建议换成高清录屏。
- 开场、手势、二级菜单前半段及配色段的圆形菜单为明确标注的动效示意，并非实时录制的界面。仅说明默认 Ctrl + 右键按住模式。
- 功能与用语依据仓库 README，未加入实测加速数字或未实现功能。
- 当前无背景音乐，无最终逐句字幕；后续依实际配音对齐字幕，不把草稿时间直接当成成片字幕。

## 发布文案草稿

标题：给鼠标装一个快捷菜单！GoPieMenu，让常用操作围着你转

简介：快捷键、应用、文件与网址，都可以放进鼠标旁边的圆形菜单。视频展示 GoPieMenu 的默认呼叫方式、自定义动作、二级菜单分类、应用专属菜单与独立外观设置。Windows 开源项目，下载和使用说明：https://github.com/RyuuMeow/GoPieMenu 。录屏中的绘图软件为使用场景示例。

后续配音：用户自有 GPT-SoVITS 模型；尚未生成或接入音频。
