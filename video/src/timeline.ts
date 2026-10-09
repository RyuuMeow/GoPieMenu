// Scene boundaries also define voiceover slots. Keep narration comfortably inside each slot.
export const FPS = 30;
export const scenes = [
  {
    id: "hook",
    start: 0,
    seconds: 6,
    label: "让操作，跟上灵感",
    narration:
      "灵感正好，别让找菜单打断你。GoPieMenu，让常用操作来到鼠标身边。",
  },
  {
    id: "gesture",
    start: 6,
    seconds: 8,
    label: "按住 · 移向 · 松开",
    narration:
      "按住 Control 和鼠标右键，移向目标，再松开。一个顺手的动作，就能调用常用功能。",
  },
  {
    id: "actions",
    start: 14,
    seconds: 9,
    label: "把常用操作，放进一个圈",
    narration:
      "快捷键、应用、文件和网址，都能放进同一个圆形菜单。直接点击扇区编辑，设置好再点 Apply。",
  },
  {
    id: "workflow",
    start: 23,
    seconds: 9,
    label: "为你的工作流，准备专属菜单",
    narration:
      "绘画时，把图层和工具放在手边。还能按应用设置专属菜单，让不同工作流各就各位。",
  },
  {
    id: "style",
    start: 32,
    seconds: 8,
    label: "你的菜单，你的风格",
    narration:
      "调整颜色、透明度和尺寸。每个菜单都能拥有自己的外观，按你的习惯来。",
  },
  {
    id: "outro",
    start: 40,
    seconds: 8,
    label: "少一点寻找，多一点专注",
    narration:
      "GoPieMenu，Windows 上的开源圆形快捷菜单。项目地址放在简介里，来做一个属于你的菜单。",
  },
] as const;
export const TOTAL_FRAMES = 48 * FPS;
