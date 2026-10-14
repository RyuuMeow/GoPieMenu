import { Copy } from "./Language";
import type { CSSProperties, ReactNode } from "react";
import { interpolateColors, useCurrentFrame } from "remotion";
import { MotionStage, snap } from "./motion";

export const panel: CSSProperties = {
  position: "absolute",
  background: "white",
  border: "2px solid #dce4ef",
  borderRadius: 24,
  boxShadow: "0 22px 65px #33547a15",
  color: "#253348",
};
export const field: CSSProperties = {
  background: "#f7f9fc",
  border: "2px solid #dce4ef",
  borderRadius: 14,
  padding: "22px 28px",
  fontSize: 36,
};
export const DemoStage = ({ children }: { children: ReactNode }) => (
  <MotionStage>
    {children}
    <div
      style={{
        position: "absolute",
        right: 80,
        bottom: 35,
        fontSize: 19,
        color: "#8b97a8",
      }}
    >
      <Copy>操作示意</Copy>
    </div>
  </MotionStage>
);
export const Keycap = ({
  children,
  active = false,
}: {
  children: ReactNode;
  active?: boolean;
}) => (
  <span
    style={{
      display: "inline-block",
      padding: "15px 24px",
      borderRadius: 13,
      background: active ? "#507fcf" : "white",
      color: active ? "white" : "#253348",
      border: "2px solid #cbd8e9",
      boxShadow: "0 5px 0 #dce4ef",
      fontSize: 38,
    }}
  >
    {children}
  </span>
);

// Colors use StyleConfig/EditorSession presets. The geometry is a simplified animated demonstration.
export const presets = [
  {
    name: "Frost",
    sector: "rgba(230,235,242,0.98)",
    hover: "rgba(108,150,218,0.99)",
    center: "#f9fafc",
    text: "#252f3e",
  },
  {
    name: "Slate",
    sector: "rgba(78,88,103,0.88)",
    hover: "rgba(105,140,188,0.94)",
    center: "#2d2d37",
    text: "#f0f0f5",
  },
  {
    name: "Ocean",
    sector: "rgba(40,66,99,0.88)",
    hover: "rgba(60,130,210,0.94)",
    center: "#2d2d37",
    text: "#f0f0f5",
  },
  {
    name: "Obsidian",
    sector: "rgba(55,55,65,0.78)",
    hover: "rgba(80,120,200,0.86)",
    center: "#2d2d37",
    text: "#f0f0f5",
  },
];
export const LivePie = ({
  x = 650,
  y = 560,
  size = 650,
  preset = 0,
  changeAt = 0,
  active = -1,
  labels = ["变形", "调色", "图层"],
}: {
  x?: number;
  y?: number;
  size?: number;
  preset?: number;
  changeAt?: number;
  active?: number;
  labels?: string[];
}) => {
  const f = useCurrentFrame();
  const p = presets[preset],
    old = presets[Math.max(0, preset - 1)];
  const progress = snap(f, changeAt, changeAt + 10, 0, 1);
  const color = (key: "sector" | "hover" | "center" | "text") =>
    interpolateColors(progress, [0, 1], [old[key], p[key]]);
  const point = (r: number, a: number) => [
    300 + r * Math.cos((a * Math.PI) / 180),
    300 + r * Math.sin((a * Math.PI) / 180),
  ];
  return (
    <svg
      style={{
        position: "absolute",
        left: x - size / 2,
        top: y - size / 2,
        overflow: "visible",
        filter: "drop-shadow(0 15px 22px #33547a18)",
      }}
      width={size}
      height={size}
      viewBox="0 0 600 600"
    >
      {labels.map((label, i) => {
        const mid = -90 + (i * 360) / labels.length,
          start = mid - 180 / labels.length + 2,
          end = mid + 180 / labels.length - 2;
        const a = point(272, start),
          b = point(272, end),
          c = point(94, end),
          d = point(94, start),
          t = point(185, mid);
        return (
          <g
            key={label}
            style={{
              transformOrigin: "300px 300px",
              scale: active === i ? 1.035 : 1,
            }}
          >
            <path
              d={`M${a} A272 272 0 0 1 ${b} L${c} A94 94 0 0 0 ${d} Z`}
              fill={color(active === i ? "hover" : "sector")}
              stroke="#a4b1c2"
              strokeWidth="1.5"
            />
            <text
              x={t[0]}
              y={t[1] + 12}
              textAnchor="middle"
              fontSize="32"
              fill={active === i ? "white" : color("text")}
            >
              <Copy>{label}</Copy>
            </text>
          </g>
        );
      })}
      <circle
        cx="300"
        cy="300"
        r="82"
        fill={color("center")}
        stroke="#a4b1c2"
      />
      <circle cx="300" cy="300" r="8" fill="#507fcf" />
    </svg>
  );
};
