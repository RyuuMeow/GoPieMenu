import { Img, staticFile, useCurrentFrame } from "remotion";
import { MotionStage, Words, snap } from "./motion";

type Focus = { x: number; y: number; w: number; h: number };
const Guide = ({
  title,
  step,
  lines,
  asset,
  focus,
}: {
  title: string;
  step: string;
  lines: string[];
  asset: string;
  focus?: Focus;
}) => {
  const f = useCurrentFrame();
  const ratio = 1180 / 1298;
  return (
    <MotionStage>
      <Words text={title} top={135} size={80} />
      <div
        style={{
          position: "absolute",
          left: 96,
          top: 325,
          fontSize: 24,
          color: "#507fcf",
          fontWeight: 700,
          letterSpacing: 2,
        }}
      >
        {step}
      </div>
      <div
        style={{
          position: "absolute",
          left: 96,
          top: 410,
          width: 470,
          fontSize: 36,
          lineHeight: 1.8,
          color: "#62738b",
        }}
      >
        {lines.map((line, i) => (
          <div
            key={line}
            style={{
              opacity: snap(f, i * 6, i * 6 + 12, 0, 1),
              translate: `0 ${snap(f, i * 6, i * 6 + 16, 25, 0)}px`,
            }}
          >
            {line}
          </div>
        ))}
      </div>
      <div
        style={{
          position: "absolute",
          left: 645,
          top: 250,
          width: 1180,
          height: 783,
          overflow: "hidden",
          borderRadius: 18,
          boxShadow: "0 18px 60px #33547a18",
          border: "1px solid #dce4ef",
          translate: `${snap(f, 0, 15, 100, 0)}px 0`,
          opacity: snap(f, 0, 10, 0, 1),
        }}
      >
        <Img src={staticFile(`assets/${asset}`)} style={{ width: "100%" }} />
        {focus && (
          <div
            style={{
              position: "absolute",
              left: focus.x * ratio,
              top: focus.y * ratio,
              width: focus.w * ratio,
              height: focus.h * ratio,
              border: "3px solid #507fcf",
              background: "#507fcf08",
              borderRadius: 12,
              boxShadow: "0 0 0 5px #507fcf12",
              opacity: snap(f, 12, 23, 0, 1),
            }}
          />
        )}
      </div>
      <div
        style={{
          position: "absolute",
          left: 96,
          bottom: 65,
          width: 450,
          color: "#7e8b9d",
          fontSize: 20,
          lineHeight: 1.6,
        }}
      >
        真实软件截图 · 镜头与高亮为导览标注
      </div>
    </MotionStage>
  );
};

export const Overview = () => (
  <Guide
    title="先选菜单，再开始设置。"
    step="01 / 选单管理"
    lines={["左上角切换菜单。", "＋ 新建；⋯ 管理。", "每个菜单，各有用途。"]}
    asset="tour-menus.png"
    focus={{ x: 25, y: 45, w: 495, h: 240 }}
  />
);
export const Actions = () => {
  const f = useCurrentFrame();
  const second = f >= 90;
  return (
    <Guide
      title={second ? "点选动作，编辑右侧设置。" : "Add action，装入常用操作。"}
      step="02 / 动作设置"
      lines={
        second
          ? [
              "改名称、图标和类型。",
              "录入要执行的快捷键。",
              "目标和动作一眼看清。",
            ]
          : [
              "快捷键、应用、文件。",
              "网址、指令，也能加入。",
              "子菜单负责分类收纳。",
            ]
      }
      asset={second ? "submenu-editor.png" : "tour-add.png"}
      focus={
        second
          ? { x: 970, y: 166, w: 310, h: 235 }
          : { x: 20, y: 124, w: 234, h: 320 }
      }
    />
  );
};
export const Arrange = () => (
  <Guide
    title="拖动排序，按你的习惯排列。"
    step="03 / 排列动作"
    lines={[
      "打开 Arrange actions。",
      "拖动整行调整顺序。",
      "列表与圆形预览对应。",
    ]}
    asset="tour-arrange.png"
    focus={{ x: 16, y: 645, w: 927, h: 197 }}
  />
);
export const Trigger = () => {
  const f = useCurrentFrame();
  return (
    <Guide
      title="怎么呼出，在哪个应用使用？"
      step="05 / 触发与应用匹配"
      lines={[
        "Open with：选择触发方式。",
        "Keyboard shortcut：设键。",
        "Available in：指定应用。",
        "留空，则适用于所有应用。",
      ]}
      asset="tour-trigger.png"
      focus={
        f < 75
          ? { x: 976, y: 310, w: 305, h: 247 }
          : { x: 976, y: 617, w: 305, h: 170 }
      }
    />
  );
};
export const Settings = () => (
  <Guide
    title="全局设置，也保持简单。"
    step="07 / 设置与备份"
    lines={[
      "开机启动。",
      "Import / Export 导入导出。",
      "打开自定义图标文件夹。",
    ]}
    asset="tour-settings.png"
    focus={{ x: 431, y: 270, w: 436, h: 320 }}
  />
);
export const Apply = () => (
  <Guide
    title="预览满意，再点 Apply。"
    step="08 / 保存生效"
    lines={[
      "改动先留在预览。",
      "Apply：保存并生效。",
      "Discard：放弃未保存改动。",
    ]}
    asset="preset-frost.png"
    focus={{ x: 1064, y: 50, w: 170, h: 49 }}
  />
);

const presets = ["Frost", "Slate", "Ocean", "Obsidian"];
export const Style = () => {
  const f = useCurrentFrame();
  const index = Math.min(3, Math.floor(f / 60));
  const local = f - index * 60;
  return (
    <MotionStage>
      <Words text="四种风格，一键切换。" top={135} size={80} />
      <div
        style={{
          position: "absolute",
          left: 96,
          top: 330,
          color: "#507fcf",
          fontSize: 24,
          letterSpacing: 2,
          fontWeight: 700,
        }}
      >
        06 / APPEARANCE
      </div>
      <div
        style={{
          position: "absolute",
          left: 96,
          top: 410,
          fontSize: 66,
          fontWeight: 750,
          color: "#253348",
          translate: `0 ${snap(local, 0, 12, 30, 0)}px`,
          opacity: snap(local, 0, 8, 0, 1),
        }}
      >
        {presets[index]}
      </div>
      <div
        style={{
          position: "absolute",
          left: 96,
          top: 535,
          fontSize: 32,
          lineHeight: 1.8,
          color: "#62738b",
        }}
      >
        改变菜单外观。
        <br />
        设置界面始终清爽。
        <br />
        Size 调大小；Advanced 微调。
      </div>
      <div
        style={{
          position: "absolute",
          left: 96,
          top: 800,
          display: "flex",
          gap: 10,
        }}
      >
        {presets.map((p, i) => (
          <div
            key={p}
            style={{
              padding: "12px 14px",
              borderRadius: 10,
              fontSize: 20,
              color: index === i ? "white" : "#62738b",
              background: index === i ? "#507fcf" : "#e9eff7",
            }}
          >
            {p}
          </div>
        ))}
      </div>
      <div
        style={{
          position: "absolute",
          left: 645,
          top: 250,
          width: 1180,
          height: 783,
          overflow: "hidden",
          borderRadius: 18,
          border: "1px solid #dce4ef",
          boxShadow: "0 18px 60px #33547a18",
        }}
      >
        {presets.map((p, i) => (
          <Img
            key={p}
            src={staticFile(`assets/preset-${p.toLowerCase()}.png`)}
            style={{
              position: "absolute",
              width: "100%",
              opacity: i === index ? 1 : i === index - 1 ? 1 : 0,
              zIndex: i === index ? 2 : 0,
              clipPath:
                i === index
                  ? `inset(0 ${snap(local, 0, 13, 100, 0)}% 0 0)`
                  : undefined,
            }}
          />
        ))}
        <div
          style={{
            position: "absolute",
            zIndex: 3,
            left: 890,
            top: 220,
            width: 248,
            height: 42,
            border: "3px solid #507fcf",
            borderRadius: 10,
          }}
        />
      </div>
      <div
        style={{
          position: "absolute",
          left: 96,
          bottom: 65,
          fontSize: 20,
          color: "#7e8b9d",
        }}
      >
        实际预设截图 · 动画擦除切换
      </div>
    </MotionStage>
  );
};
