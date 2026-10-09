export const FPS = 30;
const chapters = [
  {
    id: "hook",
    seconds: 4,
    label: "常用操作就在手边",
    narration: "GoPieMenu，一分钟，设置你的快捷菜单。",
  },
  {
    id: "gesture",
    seconds: 4,
    label: "按住划过松开",
    narration: "按住，划过去，松开执行。触发键也能自己设置。",
  },
  {
    id: "overview",
    seconds: 5,
    label: "选单管理",
    narration: "左上角切换菜单，点加号新建，更多选项里管理。",
  },
  {
    id: "actions",
    seconds: 6,
    label: "动作设置",
    narration: "Add action 添加操作。点选动作，在右侧改名称、类型和快捷键。",
  },
  {
    id: "arrange",
    seconds: 3,
    label: "排列动作",
    narration: "打开排列列表，拖动整行调整顺序。",
  },
  {
    id: "submenu",
    seconds: 9,
    label: "二级菜单",
    narration:
      "一个扇形，还能装更多。移向分类，进入列表，松开执行。双击子菜单，继续编辑。",
  },
  {
    id: "trigger",
    seconds: 5,
    label: "触发与应用匹配",
    narration: "设置触发方式和按键，还能指定在哪个应用使用。",
  },
  {
    id: "style",
    seconds: 8,
    label: "四种外观预设",
    narration:
      "打开 Appearance。Frost、Slate、Ocean、Obsidian。四种风格，一键切换。",
  },
  {
    id: "settings",
    seconds: 4,
    label: "全局设置",
    narration: "全局设置里，管理开机启动，导入导出和自定义图标。",
  },
  {
    id: "apply",
    seconds: 3,
    label: "保存生效",
    narration: "预览满意，点 Apply，保存生效。",
  },
  {
    id: "workflow",
    seconds: 4,
    label: "回到创作",
    narration: "回到创作，常用操作围着你转。",
  },
  {
    id: "outro",
    seconds: 4,
    label: "项目入口",
    narration: "GoPieMenu，让操作跟上你的手。项目地址见简介。",
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
