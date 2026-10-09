import {
  AnimatedImage,
  Img,
  Interactive,
  interpolate,
  staticFile,
  useCurrentFrame,
} from "remotion";
import { clamp, ease, Heading, Picture, Pill, Reveal, Stage } from "./visuals";

export const Hook = () => {
  const frame = useCurrentFrame();
  return (
    <Stage>
      <Reveal style={{ position: "absolute", left: 96, top: 285, zIndex: 2 }}>
        <div
          style={{
            fontSize: 24,
            letterSpacing: 5,
            color: "#477ddd",
            marginBottom: 28,
          }}
        >
          把常用操作，带到手边
        </div>
        <Interactive.H1
          name="开场标题"
          style={{
            fontSize: 102,
            lineHeight: 1.18,
            letterSpacing: -5,
            margin: 0,
          }}
        >
          让操作，
          <br />
          跟上灵感。
        </Interactive.H1>
        <div style={{ fontSize: 32, color: "#61718a", marginTop: 35 }}>
          一个圆形菜单，接住你的工作流。
        </div>
      </Reveal>
      <Interactive.Div
        name="真实操作画面"
        style={{
          position: "absolute",
          left: 805,
          top: 193,
          width: 1060,
          height: 680,
          borderRadius: 30,
          overflow: "hidden",
          boxShadow: "0 38px 100px #25466b30",
          scale: interpolate(frame, [0, 180], [1.02, 1.08], {
            extrapolateRight: "clamp",
          }),
        }}
      >
        <Img
          src={staticFile("assets/drawing.png")}
          style={{
            width: "100%",
            height: "100%",
            objectFit: "cover",
            objectPosition: "62% center",
          }}
        />
      </Interactive.Div>
      <Reveal delay={32} style={{ position: "absolute", left: 100, top: 775 }}>
        <Pill>Ctrl ＋ 鼠标右键</Pill>
      </Reveal>
    </Stage>
  );
};

// An explicitly labelled motion diagram explains the gesture; actual UI is shown in the other scenes.
export const Gesture = () => {
  const frame = useCurrentFrame();
  const phase = frame < 75 ? 0 : frame < 155 ? 1 : 2;
  const cursorX = interpolate(
    frame,
    [35, 65, 95, 125],
    [1350, 1310, 1310, 1310],
    clamp,
  );
  const cursorY = interpolate(frame, [35, 65, 95, 125], [695, 550, 550, 375], {
    ...clamp,
    easing: ease,
  });
  return (
    <Stage>
      <Heading eyebrow="01 / 随手调用">按住。移向。松开。</Heading>
      <div style={{ position: "absolute", left: 98, top: 430, width: 680 }}>
        {["按住 Ctrl + 鼠标右键", "移向需要的动作", "松开，执行"].map(
          (text, i) => (
            <div
              key={text}
              style={{
                display: "flex",
                gap: 28,
                alignItems: "center",
                marginBottom: 28,
                opacity: phase === i ? 1 : 0.38,
                color: phase === i ? "#477ddd" : "#152845",
              }}
            >
              <div
                style={{
                  fontSize: 23,
                  border: "1px solid currentColor",
                  borderRadius: "50%",
                  width: 56,
                  height: 56,
                  display: "grid",
                  placeItems: "center",
                }}
              >
                0{i + 1}
              </div>
              <div style={{ fontSize: 37, fontWeight: 600 }}>{text}</div>
            </div>
          ),
        )}
        <div style={{ fontSize: 24, color: "#61718a", marginTop: 48 }}>
          默认按住模式 · Escape 取消
        </div>
      </div>
      <div
        style={{
          position: "absolute",
          left: 1030,
          top: 278,
          opacity:
            phase === 2
              ? interpolate(frame, [155, 173], [1, 0], clamp)
              : interpolate(frame, [25, 50], [0, 1], clamp),
        }}
      >
        <svg width="560" height="560" viewBox="0 0 560 560">
          {[0, 90, 180, 270].map((angle, i) => (
            <path
              key={angle}
              transform={`rotate(${angle} 280 280)`}
              d="M 101 92 A 260 260 0 0 1 459 92 L 342 216 A 90 90 0 0 0 218 216 Z"
              fill={phase === 1 && i === 0 ? "#5489e6" : "#e1e8f3"}
              stroke="#a6bad7"
              strokeWidth="2"
            />
          ))}
          <text
            x="280"
            y="123"
            textAnchor="middle"
            fontSize="34"
            fill={phase === 1 ? "white" : "#294363"}
          >
            应用
          </text>
          <text
            x="447"
            y="292"
            textAnchor="middle"
            fontSize="34"
            fill="#294363"
          >
            网址
          </text>
          <text
            x="280"
            y="463"
            textAnchor="middle"
            fontSize="34"
            fill="#294363"
          >
            文件
          </text>
          <text
            x="110"
            y="292"
            textAnchor="middle"
            fontSize="30"
            fill="#294363"
          >
            快捷键
          </text>
        </svg>
      </div>
      <svg
        width="55"
        height="65"
        style={{
          position: "absolute",
          left: cursorX,
          top: cursorY,
          opacity: phase === 2 ? 0 : 1,
          filter: "drop-shadow(0 4px 6px #0003)",
        }}
      >
        <path
          d="M5 3 L7 48 L20 36 L30 58 L39 53 L28 32 L47 29 Z"
          fill="#fff"
          stroke="#264f86"
          strokeWidth="3"
        />
      </svg>
      {phase === 2 && (
        <Reveal
          style={{
            position: "absolute",
            left: 1090,
            top: 455,
            fontSize: 70,
            color: "#477ddd",
          }}
        >
          ✓ 执行动作
        </Reveal>
      )}
      <div
        style={{
          position: "absolute",
          left: 1160,
          top: 866,
          fontSize: 21,
          color: "#8491a5",
        }}
      >
        操作示意 · 菜单内容可自定义
      </div>
    </Stage>
  );
};

export const Actions = () => {
  const frame = useCurrentFrame();
  return (
    <Stage>
      <Heading eyebrow="02 / 自定义动作">常用操作，一个圈。</Heading>
      <Reveal
        delay={10}
        style={{
          position: "absolute",
          left: 96,
          top: 394,
          display: "flex",
          flexDirection: "column",
          gap: 23,
        }}
      >
        {[
          "⌘   录制快捷键",
          "↗   启动应用程序",
          "▱   打开文件 / 网址",
          ">_   执行指令",
        ].map((t) => (
          <Pill key={t}>{t}</Pill>
        ))}
        <div style={{ fontSize: 28, color: "#61718a", marginTop: 14 }}>
          点击扇区编辑，Apply 后生效。
        </div>
      </Reveal>
      <Picture
        src="editor.png"
        style={{
          left: 700,
          top: 272,
          width: 1125,
          height: 737,
          scale: interpolate(frame, [0, 270], [0.96, 1], clamp),
          transformOrigin: "top right",
        }}
      />
    </Stage>
  );
};

export const Workflow = () => (
  <Stage dark>
    <Reveal style={{ position: "absolute", left: 96, top: 160 }}>
      <div
        style={{
          fontSize: 23,
          color: "#a9c9ff",
          letterSpacing: 4,
          marginBottom: 18,
        }}
      >
        03 / 应用专属菜单
      </div>
      <Interactive.H1
        name="工作流标题"
        style={{ fontSize: 75, lineHeight: 1.2, letterSpacing: -3, margin: 0 }}
      >
        让工具，围绕你的工作。
      </Interactive.H1>
    </Reveal>
    <div
      style={{
        position: "absolute",
        left: 96,
        top: 358,
        borderRadius: 20,
        overflow: "hidden",
        boxShadow: "0 24px 90px #0005",
      }}
    >
      <AnimatedImage
        src={staticFile("assets/drawing-demo.gif")}
        width={780}
        height={599}
        fit="contain"
      />
    </div>
    <Reveal delay={18} style={{ position: "absolute", left: 1010, top: 414 }}>
      <div style={{ color: "#a9c9ff", fontSize: 26, marginBottom: 27 }}>
        绘画工作流 / 实际操作片段
      </div>
      <div style={{ fontSize: 58, lineHeight: 1.5, fontWeight: 650 }}>
        图层、工具、快捷键
        <br />
        就在鼠标身边。
      </div>
      <div
        style={{
          fontSize: 30,
          color: "#bed0e6",
          lineHeight: 1.7,
          marginTop: 33,
        }}
      >
        按应用设置菜单范围
        <br />
        专属配置优先于全局配置
      </div>
    </Reveal>
  </Stage>
);

export const Style = () => {
  const frame = useCurrentFrame();
  return (
    <Stage>
      <Heading eyebrow="04 / 独立外观">你的菜单，你的风格。</Heading>
      <Picture
        src="colors.png"
        style={{ left: 760, top: 287, width: 1050, height: 688 }}
      />
      <Reveal delay={10} style={{ position: "absolute", left: 96, top: 439 }}>
        <div style={{ fontSize: 43, fontWeight: 650, lineHeight: 1.7 }}>
          颜色 / 透明度 / 尺寸
        </div>
        <div
          style={{
            fontSize: 30,
            color: "#61718a",
            marginTop: 22,
            lineHeight: 1.7,
          }}
        >
          每个菜单独立设置
          <br />
          复制之后，也能各有风格。
        </div>
        <div style={{ display: "flex", gap: 23, marginTop: 60 }}>
          {["#5485d0", "#68a991", "#c392ce", "#dcab73"].map((color, i) => (
            <div
              key={color}
              style={{
                width: 72,
                height: 72,
                borderRadius: "50%",
                background: color,
                scale: interpolate(frame, [18 + i * 7, 35 + i * 7], [0, 1], {
                  ...clamp,
                  easing: ease,
                }),
                boxShadow: "0 8px 24px #2342",
              }}
            />
          ))}
        </div>
      </Reveal>
    </Stage>
  );
};

export const Outro = () => (
  <Stage dark>
    <Reveal
      style={{
        position: "absolute",
        inset: "215px 96px auto",
        textAlign: "center",
      }}
    >
      <Img
        src={staticFile("assets/logo.png")}
        style={{
          width: 140,
          height: 140,
          objectFit: "contain",
          marginBottom: 25,
        }}
      />
      <Interactive.H1
        name="片尾标题"
        style={{ fontSize: 99, lineHeight: 1.25, letterSpacing: -4, margin: 0 }}
      >
        少一点寻找，多一点专注。
      </Interactive.H1>
      <div style={{ fontSize: 34, color: "#bfd0e7", marginTop: 35 }}>
        GoPieMenu · Windows 开源圆形快捷菜单
      </div>
    </Reveal>
    <Reveal
      delay={30}
      style={{
        position: "absolute",
        left: 0,
        right: 0,
        top: 725,
        textAlign: "center",
      }}
    >
      <div
        style={{
          display: "inline-block",
          background: "#edf4ff",
          color: "#19385d",
          borderRadius: 15,
          padding: "21px 38px",
          fontSize: 35,
          fontWeight: 600,
        }}
      >
        github.com/RyuuMeow/GoPieMenu
      </div>
      <div style={{ fontSize: 25, color: "#bfd0e7", marginTop: 24 }}>
        下载与项目地址，见视频简介。
      </div>
    </Reveal>
  </Stage>
);
