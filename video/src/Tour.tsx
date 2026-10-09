import { FocalText } from "./FocalText";
import { Img, staticFile, useCurrentFrame } from "remotion";
import { Cursor, MotionStage, snap } from "./motion";

type Focus = { x: number; y: number; w: number; h: number };
// One headline and one cropped interface target per shot.
const Guide = ({
  title,
  asset,
  focus,
  delay = 0,
}: {
  title: string;
  asset: string;
  focus: Focus;
  delay?: number;
}) => {
  const frame = useCurrentFrame();
  const f = frame - delay;
  const zoom = Math.min(2.2, 1320 / (focus.w + 90), 620 / (focus.h + 70));
  const x = Math.min(
    0,
    Math.max(1440 - 1298 * zoom, 720 - (focus.x + focus.w / 2) * zoom),
  );
  const y = Math.min(
    0,
    Math.max(720 - 861 * zoom, 360 - (focus.y + focus.h / 2) * zoom),
  );
  const cx = x + (focus.x + focus.w * 0.65) * zoom;
  const cy = y + (focus.y + focus.h * 0.65) * zoom;
  return (
    <MotionStage>
      <div
        style={{
          position: "absolute",
          left: 240,
          top: 200,
          width: 1440,
          height: 720,
          overflow: "hidden",
          borderRadius: 22,
          border: "1px solid #dce4ef",
          boxShadow: "0 18px 60px #33547a18",
          scale: snap(f, 0, 14, 0.97, 1),
        }}
      >
        <div
          style={{
            position: "absolute",
            left: x,
            top: y,
            width: 1298 * zoom,
            height: 861 * zoom,
          }}
        >
          <Img src={staticFile(`assets/${asset}`)} style={{ width: "100%" }} />
        </div>
        <div
          style={{
            position: "absolute",
            left: x + focus.x * zoom,
            top: y + focus.y * zoom,
            width: focus.w * zoom,
            height: focus.h * zoom,
            border: "3px solid #507fcf",
            borderRadius: 12,
            boxShadow: "0 0 0 1800px #f7f9fc55",
            opacity: snap(f, 8, 17, 0, 1),
          }}
        />
        <Cursor
          x={snap(f, 8, 27, cx + 80, cx)}
          y={snap(f, 8, 27, cy + 55, cy)}
        />
      </div>
      <FocalText
        text={title}
        x={Math.max(
          460,
          Math.min(1460, 240 + x + (focus.x + focus.w / 2) * zoom),
        )}
        y={Math.max(115, 200 + y + focus.y * zoom - 75)}
        size={70}
        delay={delay}
        badge
      />
    </MotionStage>
  );
};
export const Overview = () => (
  <Guide
    title="选个菜单"
    asset="tour-menus.png"
    focus={{ x: 25, y: 45, w: 495, h: 240 }}
  />
);
export const Actions = () => {
  const f = useCurrentFrame();
  return f < 60 ? (
    <Guide
      title="添加动作"
      asset="tour-add.png"
      focus={{ x: 20, y: 124, w: 234, h: 320 }}
    />
  ) : (
    <Guide
      title="设个快捷键"
      asset="submenu-editor.png"
      focus={{ x: 970, y: 304, w: 310, h: 94 }}
      delay={60}
    />
  );
};
export const Arrange = () => (
  <Guide
    title="拖动排序"
    asset="tour-arrange.png"
    focus={{ x: 16, y: 645, w: 927, h: 197 }}
  />
);
export const Trigger = () => {
  const f = useCurrentFrame();
  return (
    <Guide
      title={f < 60 ? "设置触发键" : "指定应用"}
      asset="tour-trigger.png"
      focus={
        f < 60
          ? { x: 976, y: 492, w: 305, h: 66 }
          : { x: 976, y: 617, w: 305, h: 122 }
      }
      delay={f < 60 ? 0 : 60}
    />
  );
};
export const Settings = () => {
  const f = useCurrentFrame();
  return (
    <Guide
      title={f < 45 ? "开机启动" : "导入 · 导出"}
      asset="tour-settings.png"
      focus={
        f < 45
          ? { x: 446, y: 341, w: 290, h: 40 }
          : { x: 446, y: 396, w: 220, h: 57 }
      }
      delay={f < 45 ? 0 : 45}
    />
  );
};
export const Apply = () => (
  <Guide
    title="Apply，完成"
    asset="preset-frost.png"
    focus={{ x: 1140, y: 50, w: 95, h: 49 }}
  />
);
const presets = ["Frost", "Slate", "Ocean", "Obsidian"];
export const Style = () => {
  const f = useCurrentFrame();
  const index = Math.min(3, Math.floor(f / 45));
  const local = f - index * 45;
  return (
    <MotionStage>
      <FocalText
        text={presets[index]}
        x={1400}
        y={350}
        size={112}
        delay={index * 45}
      />
      <div
        style={{
          position: "absolute",
          left: 180,
          top: 200,
          width: 820,
          height: 760,
          overflow: "hidden",
          borderRadius: 30,
          boxShadow: "0 18px 60px #33547a12",
        }}
      >
        {presets.map((p, i) => (
          <Img
            key={p}
            src={staticFile(`assets/preset-${p.toLowerCase()}.png`)}
            style={{
              position: "absolute",
              width: 2596,
              maxWidth: "none",
              left: -546,
              top: -594,
              opacity: i === index || i === index - 1 ? 1 : 0,
              zIndex: i === index ? 2 : 0,
              clipPath:
                i === index
                  ? `inset(0 ${snap(local, 0, 14, 100, 0)}% 0 0)`
                  : undefined,
            }}
          />
        ))}
      </div>
      <div
        style={{
          position: "absolute",
          left: 1100,
          top: 450,
          width: 630,
          height: 150,
          overflow: "hidden",
          borderRadius: 18,
          border: "1px solid #dce4ef",
        }}
      >
        <Img
          src={staticFile(`assets/preset-${presets[index].toLowerCase()}.png`)}
          style={{
            position: "absolute",
            width: 2855.6,
            maxWidth: "none",
            left: -2145,
            top: -510,
          }}
        />
      </div>
      <div
        style={{
          position: "absolute",
          left: 1130,
          top: 700,
          display: "flex",
          gap: 20,
        }}
      >
        {presets.map((p, i) => (
          <div
            key={p}
            style={{
              width: i === index ? 60 : 12,
              height: 12,
              borderRadius: 6,
              background: i === index ? "#507fcf" : "#d4deed",
            }}
          />
        ))}
      </div>
    </MotionStage>
  );
};
