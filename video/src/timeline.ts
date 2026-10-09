export const FPS = 30;
// Voice slots correspond to the six editable scenes. Durations are seconds.
export const scenes = [
  {
    id: "hook",
    start: 0,
    seconds: 4,
    label: "直接叫到手边",
    narration: "别找菜单了，直接叫到手边。",
  },
  {
    id: "gesture",
    start: 4,
    seconds: 5,
    label: "按住划过松开",
    narration: "按住，划过去，松开。就是这么顺手。",
  },
  {
    id: "actions",
    start: 9,
    seconds: 6,
    label: "动作与编辑",
    narration: "快捷键、应用、文件，装进一个圈。设置好，点 Apply。",
  },
  {
    id: "workflow",
    start: 15,
    seconds: 6,
    label: "专属工作流",
    narration: "画画时，工具就在旁边。不同应用，用不同菜单。",
  },
  {
    id: "style",
    start: 21,
    seconds: 6,
    label: "连续换色",
    narration: "换个配色，换个心情。每个菜单，都能有自己的风格。",
  },
  {
    id: "outro",
    start: 27,
    seconds: 5,
    label: "项目入口",
    narration: "GoPieMenu，让操作跟上你的手。项目地址见简介。",
  },
] as const;
export const TOTAL_FRAMES = 32 * FPS;
