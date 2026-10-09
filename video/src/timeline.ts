export const FPS = 30;
const chapters = [
  {
    id: "hook",
    seconds: 2,
    label: "常用操作就在手边",
    narration: "常用操作，就在手边。",
  },
  {
    id: "gesture",
    seconds: 3,
    label: "按住划过松开",
    narration: "按住，划过，松开。",
  },
  {
    id: "overview",
    seconds: 3,
    label: "选单管理",
    narration: "选个菜单，开始设置。",
  },
  {
    id: "actions",
    seconds: 4,
    label: "动作设置",
    narration: "添加动作，设个快捷键。",
  },
  {
    id: "arrange",
    seconds: 2,
    label: "排列动作",
    narration: "拖动，调整顺序。",
  },
  {
    id: "submenu",
    seconds: 6,
    label: "二级菜单",
    narration: "打开分类，再选动作。双击子菜单，继续编辑。",
  },
  {
    id: "trigger",
    seconds: 4,
    label: "触发与应用匹配",
    narration: "设置触发键，指定应用。",
  },
  {
    id: "style",
    seconds: 6,
    label: "四种外观预设",
    narration: "Frost、Slate、Ocean、Obsidian。",
  },
  {
    id: "settings",
    seconds: 3,
    label: "全局设置",
    narration: "开机启动，导入导出。",
  },
  {
    id: "apply",
    seconds: 2,
    label: "保存生效",
    narration: "Apply，完成。",
  },
  {
    id: "workflow",
    seconds: 3,
    label: "回到创作",
    narration: "回到创作。",
  },
  {
    id: "outro",
    seconds: 3,
    label: "项目入口",
    narration: "GoPieMenu，项目地址见简介。",
  },
] as const;
export type SceneId = (typeof chapters)[number]["id"];
export const scenes = chapters.map((scene, index) => ({
  ...scene,
  start: chapters.slice(0, index).reduce((sum, item) => sum + item.seconds, 0),
}));
export const TOTAL_FRAMES = chapters.reduce(
  (total, scene) => total + scene.seconds * FPS,
  0,
);
