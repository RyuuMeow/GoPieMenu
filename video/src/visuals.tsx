import type { CSSProperties, ReactNode } from "react";
import {
  AbsoluteFill,
  Easing,
  Img,
  Interactive,
  interpolate,
  staticFile,
  useCurrentFrame,
} from "remotion";

export const palette = {
  ink: "#152845",
  muted: "#61718a",
  blue: "#477ddd",
  paper: "#f4f7fc",
};
export const ease = Easing.bezier(0.16, 1, 0.3, 1);
export const clamp = {
  extrapolateLeft: "clamp",
  extrapolateRight: "clamp",
} as const;

export const Stage = ({
  children,
  dark = false,
}: {
  children: ReactNode;
  dark?: boolean;
}) => (
  <AbsoluteFill
    style={{
      background: dark ? "#142844" : "#f4f7fc",
      color: dark ? "#fff" : palette.ink,
      fontFamily: '"Microsoft YaHei", "Noto Sans CJK SC", sans-serif',
      overflow: "hidden",
    }}
  >
    <AbsoluteFill
      style={{
        background: dark
          ? "radial-gradient(ellipse at 80% 30%, #365d85 0%, transparent 60%)"
          : "radial-gradient(ellipse at 90% 10%, #dfeaff 0%, transparent 65%)",
      }}
    />
    <div
      style={{
        position: "absolute",
        width: 880,
        height: 880,
        border: `1px solid ${dark ? "#ffffff12" : "#477ddd15"}`,
        borderRadius: "50%",
        right: -160,
        top: -350,
      }}
    />
    <div
      style={{
        position: "absolute",
        left: 96,
        top: 52,
        display: "flex",
        alignItems: "center",
        gap: 13,
        fontSize: 27,
        letterSpacing: 1,
        fontWeight: 650,
      }}
    >
      <Img
        src={staticFile("assets/logo.png")}
        style={{ width: 42, height: 42, objectFit: "contain" }}
      />{" "}
      GoPieMenu{" "}
      <span style={{ fontSize: 17, opacity: 0.5, marginLeft: 10 }}>
        WINDOWS / v2.0
      </span>
    </div>
    {children}
  </AbsoluteFill>
);

export const Reveal = ({
  children,
  delay = 0,
  style,
}: {
  children: ReactNode;
  delay?: number;
  style?: CSSProperties;
}) => {
  const frame = useCurrentFrame();
  return (
    <div
      style={{
        ...style,
        opacity: interpolate(frame, [delay, delay + 20], [0, 1], clamp),
        translate: `0 ${interpolate(frame, [delay, delay + 28], [35, 0], { ...clamp, easing: ease })}px`,
      }}
    >
      {children}
    </div>
  );
};

export const Heading = ({
  eyebrow,
  children,
  detail,
}: {
  eyebrow: string;
  children: ReactNode;
  detail?: string;
}) => (
  <Reveal style={{ position: "absolute", left: 96, top: 154 }}>
    <div
      style={{
        fontSize: 23,
        color: "#477ddd",
        fontWeight: 700,
        letterSpacing: 4,
        marginBottom: 20,
      }}
    >
      {eyebrow}
    </div>
    <Interactive.H1
      name="场景标题"
      style={{
        fontSize: 76,
        lineHeight: 1.2,
        letterSpacing: -3,
        margin: 0,
        fontWeight: 750,
      }}
    >
      {children}
    </Interactive.H1>
    {detail && (
      <div style={{ fontSize: 30, marginTop: 22, color: "#61718a" }}>
        {detail}
      </div>
    )}
  </Reveal>
);

export const Picture = ({
  src,
  style,
}: {
  src: string;
  style: CSSProperties;
}) => (
  <Img
    src={staticFile(`assets/${src}`)}
    style={{
      position: "absolute",
      objectFit: "contain",
      borderRadius: 18,
      boxShadow: "0 30px 90px #183c6422",
      ...style,
    }}
  />
);

export const Pill = ({ children }: { children: ReactNode }) => (
  <div
    style={{
      display: "inline-flex",
      alignItems: "center",
      border: "1px solid #c9d7ed",
      borderRadius: 14,
      padding: "15px 25px",
      background: "#ffffffb0",
      fontSize: 28,
      color: palette.ink,
    }}
  >
    {children}
  </div>
);
