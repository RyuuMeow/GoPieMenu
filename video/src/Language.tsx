import { createContext, useContext } from "react";

export const LanguageContext = createContext<"zh" | "en">("zh");
const english: Record<string, string> = {
  就在手边: "Within reach",
  动效示意: "Animated demo",
  按住: "Hold",
  划过: "Swipe",
  执行: "Release",
  "Ctrl + 右键 · 操作示意": "Ctrl + right-click · Animated demo",
  回到创作: "Back to creating",
  "实际操作 GIF": "Live demo",
  原速播放: "Original speed",
  "Windows 开源快捷菜单": "An open-source radial menu for Windows",
  项目地址见简介: "Download link in the description",
  操作示意: "Animated demo",
  选个菜单: "Pick a menu",
  添加动作: "Add an action",
  设个快捷键: "Set a shortcut",
  拖动排序: "Drag to reorder",
  指定应用: "Choose an app",
  设置触发键: "Set a trigger",
  "导入 · 导出": "Import · Export",
  开机启动: "Start with Windows",
  完成: "Done",
  双击编辑: "Double-click to edit",
  松开执行: "Release to run",
  再选动作: "Pick an action",
  打开分类: "Open a submenu",
  应用: "Apps",
  网址: "Links",
  文件: "Files",
  快捷键: "Shortcuts",
  变形: "Transform",
  调色: "Color",
  图层: "Layers",
  "图层 ›": "Layers ›",
  新图层: "New layer",
  切换图层显示: "Toggle visibility",
  锁定透明度: "Lock alpha",
  "✓ 新图层": "✓ New layer",
};
export const useLanguage = () => useContext(LanguageContext);
export const useCopy = () => {
  const language = useLanguage();
  return (text: string) => (language === "en" ? (english[text] ?? text) : text);
};
export const Copy = ({ children }: { children: string }) => useCopy()(children);
