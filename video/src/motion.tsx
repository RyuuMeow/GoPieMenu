import type { ReactNode } from "react";
import {
  AbsoluteFill,
  Easing,
  interpolate,
  spring,
  useCurrentFrame,
} from "remotion";

export const clamp = {
  extrapolateLeft: "clamp",
  extrapolateRight: "clamp",
} as const;
export const snap = (
  f: number,
  start: number,
  end: number,
  from: number,
  to: number,
) =>
  interpolate(f, [start, end], [from, to], {
    ...clamp,
    easing: Easing.bezier(0.16, 1, 0.3, 1),
  });
export const pop = (f: number, delay = 0) =>
  spring({
    fps: 30,
    frame: Math.max(0, f - delay),
    config: { damping: 15, stiffness: 180, mass: 0.65 },
  });

export const MotionStage = ({
  children,
  light = false,
}: {
  children: ReactNode;
  light?: boolean;
}) => {
  const f = useCurrentFrame();
  return (
    <AbsoluteFill
      style={{
        background: light ? "#edf2fb" : "#0a1528",
        color: light ? "#13284d" : "#f8fbff",
        fontFamily: '"Microsoft YaHei",sans-serif',
        overflow: "hidden",
      }}
    >
      <AbsoluteFill
        style={{
          backgroundImage: `radial-gradient(${light ? "#477ddd26" : "#88b7ff20"} 1px,transparent 1px)`,
          backgroundSize: "48px 48px",
          backgroundPosition: `${f * 0.35}px ${f * 0.2}px`,
        }}
      />
      <div
        style={{
          position: "absolute",
          width: 1100,
          height: 1100,
          left: 750,
          top: -160,
          borderRadius: "50%",
          background: light ? "#6a9ef720" : "#467dff20",
          filter: "blur(100px)",
          translate: `${Math.sin(f / 70) * 90}px ${Math.cos(f / 90) * 40}px`,
        }}
      />
      {children}
      <div
        style={{
          position: "absolute",
          left: 80,
          top: 45,
          fontSize: 25,
          fontWeight: 750,
          letterSpacing: 1,
        }}
      >
        GoPieMenu{" "}
        <span style={{ opacity: 0.5, fontSize: 17, marginLeft: 16 }}>
          WINDOWS
        </span>
      </div>
      <div
        style={{
          position: "absolute",
          right: 80,
          top: 45,
          fontSize: 18,
          letterSpacing: 3,
          opacity: 0.5,
        }}
      >
        MAKE IT YOURS.
      </div>
    </AbsoluteFill>
  );
};

export const Words = ({
  text,
  top = 165,
  left = 90,
  size = 106,
  delay = 0,
  color,
}: {
  text: string;
  top?: number;
  left?: number;
  size?: number;
  delay?: number;
  color?: string;
}) => {
  const f = useCurrentFrame();
  return (
    <div
      style={{
        position: "absolute",
        top,
        left,
        fontSize: size,
        fontWeight: 850,
        lineHeight: 1.12,
        letterSpacing: -4,
        color,
        display: "flex",
        whiteSpace: "pre",
      }}
    >
      {Array.from(text).map((c, i) => (
        <span
          key={i}
          style={{
            display: "inline-block",
            opacity: snap(f, delay + i * 2, delay + i * 2 + 5, 0, 1),
            translate: `0 ${snap(f, delay + i * 2, delay + i * 2 + 12, 80, 0)}px`,
            rotate: `${snap(f, delay + i * 2, delay + i * 2 + 12, 8, 0)}deg`,
          }}
        >
          {c}
        </span>
      ))}
    </div>
  );
};

export const Cursor = ({
  x,
  y,
  click = false,
}: {
  x: number;
  y: number;
  click?: boolean;
}) => {
  const f = useCurrentFrame();
  return (
    <div style={{ position: "absolute", left: x, top: y, zIndex: 8 }}>
      {click && (
        <div
          style={{
            position: "absolute",
            width: 90,
            height: 90,
            left: -45,
            top: -45,
            border: "3px solid #78c7ff",
            borderRadius: "50%",
            scale: 1 + (f % 18) / 12,
            opacity: 1 - (f % 18) / 18,
          }}
        />
      )}
      <svg
        width="54"
        height="65"
        style={{ filter: "drop-shadow(0 5px 7px #0005)" }}
      >
        <path
          d="M4 3 L6 46 L18 35 L30 59 L40 53 L28 32 L48 29 Z"
          fill="white"
          stroke="#294a7e"
          strokeWidth="3"
        />
      </svg>
    </div>
  );
};

export const Ring = ({
  x = 960,
  y = 540,
  size = 650,
  active = 0,
  tint = "#5798ff",
  labels = ["应用", "网址", "文件", "快捷键"],
  delay = 0,
}: {
  x?: number;
  y?: number;
  size?: number;
  active?: number;
  tint?: string;
  labels?: string[];
  delay?: number;
}) => {
  const f = useCurrentFrame();
  return (
    <div
      style={{
        position: "absolute",
        left: x - size / 2,
        top: y - size / 2,
        width: size,
        height: size,
      }}
    >
      <svg
        width={size}
        height={size}
        viewBox="0 0 600 600"
        style={{
          overflow: "visible",
          filter: `drop-shadow(0 14px 30px ${tint}22)`,
        }}
      >
        {[0, 90, 180, 270].map((a, i) => (
          <g
            key={a}
            style={{
              transformOrigin: "300px 300px",
              scale: pop(f, delay + i * 3),
              translate: `0 ${snap(f, delay + i * 3, delay + 28 + i * 3, -110, 0)}px`,
              rotate: `${a + snap(f, delay + i * 3, delay + 28 + i * 3, -75, 0)}deg`,
            }}
          >
            <path
              d="M106 97 A281 281 0 0 1 494 97 L372 224 A104 104 0 0 0 228 224 Z"
              fill={active === i ? tint : "#c9d9f1"}
              stroke={active === i ? "#ffffff" : "#92abce"}
              strokeWidth="3"
            />
            <text
              x="300"
              y="133"
              textAnchor="middle"
              fontSize="34"
              fontWeight="650"
              fill={active === i ? "white" : "#21385d"}
              transform={`rotate(${-a} 300 120)`}
            >
              {labels[i]}
            </text>
          </g>
        ))}
        <circle
          cx="300"
          cy="300"
          r="68"
          fill="#0c1b32"
          stroke="#79a8ee"
          strokeWidth="2"
        />
        <circle cx="300" cy="300" r="11" fill={tint} />
      </svg>
    </div>
  );
};

export const Cut = ({
  children,
  duration,
}: {
  children: ReactNode;
  duration: number;
}) => {
  const f = useCurrentFrame();
  const end = snap(f, duration - 8, duration - 1, 0, 1);
  return (
    <AbsoluteFill style={{ overflow: "hidden" }}>
      <AbsoluteFill
        style={{
          scale: snap(f, 0, 14, 1.24, 1) + end * 0.14,
          rotate: `${snap(f, 0, 14, -2, 0) - end * 3}deg`,
          translate: `${-end * 260}px 0`,
          filter: `blur(${end * 9}px)`,
        }}
      >
        {children}
      </AbsoluteFill>
      {[0, 1, 2, 3, 4].map((i) => (
        <div
          key={i}
          style={{
            position: "absolute",
            left: "-10%",
            width: "120%",
            top: i * 216 - 30,
            height: 250,
            background: ["#61d9bc", "#5798ff", "#9bafff", "#5798ff", "#61d9bc"][
              i
            ],
            transform: `translateX(${snap(f, duration - 12 + i, duration - 5 + i, 110, 0)}%) skewX(-12deg)`,
          }}
        />
      ))}
    </AbsoluteFill>
  );
};
