import {
  AnimatedImage,
  Img,
  interpolate,
  staticFile,
  useCurrentFrame,
} from "remotion";
import { clamp, Cursor, MotionStage, pop, Ring, snap, Words } from "./motion";

export const Hook = () => {
  const f = useCurrentFrame();
  return (
    <MotionStage>
      <div
        style={{
          position: "absolute",
          inset: 0,
          opacity: 0.38,
          scale: 1.25 + f * 0.001,
          filter: "blur(2px)",
        }}
      >
        <AnimatedImage
          src={staticFile("assets/drawing-demo.gif")}
          width={1920}
          height={1475}
          fit="cover"
          playbackRate={2}
        />
      </div>
      <div
        style={{
          position: "absolute",
          inset: 0,
          background: "linear-gradient(90deg,#09172aec,transparent)",
        }}
      />
      <Words
        text={f < 42 ? "还在找菜单？" : "直接，叫到手边。"}
        top={190}
        size={110}
        delay={f < 42 ? 0 : 42}
      />
      <div
        style={{
          scale: interpolate(f, [0, 10, 83, 108], [0.7, 1, 1, 1.4], clamp),
          transformOrigin: "1250px 590px",
        }}
      >
        <Ring
          x={1250}
          y={585}
          size={590}
          active={f < 47 ? 0 : f < 74 ? 3 : 2}
          delay={9}
        />
      </div>
      <Cursor
        x={interpolate(
          f,
          [0, 30, 49, 72, 96],
          [1550, 1250, 1010, 1250, 1250],
          clamp,
        )}
        y={interpolate(
          f,
          [0, 30, 49, 72, 96],
          [820, 400, 585, 805, 805],
          clamp,
        )}
        click={f > 70}
      />
      <div
        style={{
          position: "absolute",
          left: 95,
          top: 520,
          fontSize: 40,
          lineHeight: 1.8,
          color: "#c1d8ff",
        }}
      >
        快捷键、应用、文件
        <br />
        一个动作，触手可及。
      </div>
      <div
        style={{
          position: "absolute",
          left: 95,
          top: 805,
          padding: "16px 30px",
          background: "#5798ff",
          borderRadius: 10,
          fontSize: 31,
          scale: pop(f, 30),
        }}
      >
        Ctrl + 鼠标右键
      </div>
      <div
        style={{
          position: "absolute",
          right: 110,
          bottom: 75,
          fontSize: 21,
          color: "#bfd0e9",
        }}
      >
        菜单动效示意 / 背景为实际操作
      </div>
    </MotionStage>
  );
};

export const Gesture = () => {
  const f = useCurrentFrame();
  const phase = f < 45 ? 0 : f < 90 ? 1 : 2;
  const words = ["按住。", "划过去。", "松开，执行。"];
  return (
    <MotionStage light>
      <div
        style={{
          position: "absolute",
          left: 80,
          top: 145,
          fontSize: 330,
          fontWeight: 900,
          opacity: 0.055,
          lineHeight: 1,
        }}
      >
        0{phase + 1}
      </div>
      <Words
        key={phase}
        text={words[phase]}
        top={270}
        size={116}
        delay={phase * 45}
      />
      <div
        style={{
          position: "absolute",
          left: 96,
          top: 485,
          fontSize: 33,
          color: "#587294",
        }}
      >
        Ctrl + 右键 · 默认按住模式
      </div>
      <div
        style={{
          opacity: snap(f, 90, 103, 1, 0),
          scale: 1 + snap(f, 90, 105, 0, 0.2),
          transformOrigin: "1280px 520px",
        }}
      >
        <Ring x={1290} y={535} size={690} active={phase === 0 ? -1 : 0} />
      </div>
      <Cursor
        x={snap(f, 30, 65, 1490, 1290)}
        y={snap(f, 30, 65, 720, 318)}
        click={f < 28 || f > 92}
      />
      {phase === 2 && (
        <div
          style={{
            position: "absolute",
            left: 1060,
            top: 350,
            fontSize: 80,
            color: "#477ddd",
            scale: pop(f, 94),
            background: "white",
            padding: "45px 55px",
            borderRadius: 24,
            boxShadow: "0 22px 65px #29466c20",
          }}
        >
          ✓ 已调用
        </div>
      )}
      <div
        style={{
          position: "absolute",
          left: 95,
          bottom: 145,
          display: "flex",
          gap: 20,
        }}
      >
        {["按住", "移向", "松开"].map((t, i) => (
          <div
            key={t}
            style={{
              padding: "14px 30px",
              fontSize: 27,
              background: i === phase ? "#477ddd" : "#dce5f3",
              color: i === phase ? "white" : "#7a8da9",
              borderRadius: 40,
            }}
          >
            {t}
          </div>
        ))}
      </div>
      <div
        style={{
          position: "absolute",
          right: 100,
          bottom: 85,
          fontSize: 22,
          color: "#7a8da9",
        }}
      >
        操作示意 · Escape 取消
      </div>
    </MotionStage>
  );
};

export const Actions = () => {
  const f = useCurrentFrame();
  const shot = f < 72 ? 0 : f < 126 ? 1 : 2;
  const scale =
    shot === 0
      ? snap(f, 0, 18, 0.7, 1)
      : shot === 1
        ? snap(f, 72, 85, 1, 1.48)
        : snap(f, 126, 138, 1.48, 1.1);
  return (
    <MotionStage light>
      <div
        style={{
          position: "absolute",
          left: 250,
          top: 175,
          width: 1400,
          height: 917,
          scale,
          transformOrigin: shot === 1 ? "88% 38%" : "70% 10%",
          translate: `${shot === 1 ? -140 : 0}px ${shot === 2 ? 80 : 0}px`,
          boxShadow: "0 25px 100px #29466c30",
          borderRadius: 20,
          overflow: "hidden",
        }}
      >
        <Img src={staticFile("assets/editor.png")} style={{ width: "100%" }} />
      </div>
      <div
        style={{
          position: "absolute",
          left: 0,
          right: 0,
          top: 0,
          height: 340,
          background: "linear-gradient(#edf2fb 65%,transparent)",
        }}
      />
      <Words
        text={
          [
            "把操作，装进一个圈。",
            "点选。改名。设动作。",
            "Apply，让设置生效。",
          ][shot]
        }
        top={128}
        size={88}
        delay={shot === 0 ? 0 : shot === 1 ? 72 : 126}
      />
      {shot === 0 &&
        ["快捷键", "应用", "文件 / 网址", "指令"].map((t, i) => (
          <div
            key={t}
            style={{
              position: "absolute",
              left: 100 + i * 440,
              top: 865,
              background: "#13284d",
              color: "white",
              fontSize: 34,
              padding: "22px 30px",
              borderRadius: 12,
              translate: `0 ${snap(f, i * 9, i * 9 + 16, 220, 0)}px`,
              rotate: `${snap(f, i * 9, i * 9 + 16, -12, 0)}deg`,
            }}
          >
            {t}
          </div>
        ))}
      {shot === 1 && (
        <div
          style={{
            position: "absolute",
            left: 1015,
            top: 310,
            width: 545,
            height: 405,
            border: "5px solid #478aff",
            borderRadius: 16,
            opacity: Math.min(1, (f - 72) / 8),
          }}
        />
      )}
      {shot === 2 && (
        <Cursor
          x={snap(f, 126, 149, 1100, 1510)}
          y={snap(f, 126, 149, 600, 350)}
          click={f > 149}
        />
      )}
      <div
        style={{
          position: "absolute",
          right: 80,
          bottom: 45,
          fontSize: 20,
          color: "#61718a",
          background: "#edf2fbdc",
          padding: 10,
        }}
      >
        真实界面截图 / 镜头与指针为演示标注
      </div>
    </MotionStage>
  );
};

export const Workflow = () => {
  const f = useCurrentFrame();
  const beat = Math.min(2, Math.floor(f / 60));
  return (
    <MotionStage>
      <div
        style={{
          position: "absolute",
          left: 370,
          top: -150,
          scale: 1.15 + beat * 0.08,
          translate: `${-beat * 85}px ${beat * 35}px`,
          transformOrigin: "center",
        }}
      >
        <AnimatedImage
          src={staticFile("assets/drawing-demo.gif")}
          width={1350}
          height={1037}
          playbackRate={1.8}
          fit="contain"
        />
      </div>
      <div
        style={{
          position: "absolute",
          inset: 0,
          background:
            "linear-gradient(90deg,#0a1528 2%,#0a1528ed 24%,transparent 70%),linear-gradient(0deg,#0a1528,transparent 45%)",
        }}
      />
      <Words
        text={["图层。", "工具。", "专属工作流。"][beat]}
        top={270}
        size={118}
        delay={beat * 60}
      />
      <div
        style={{
          position: "absolute",
          left: 95,
          top: 460,
          fontSize: 35,
          color: "#a8caff",
          lineHeight: 1.7,
        }}
      >
        手不必离开画面。
        <br />
        常用操作就在旁边。
      </div>
      <div
        style={{
          position: "absolute",
          left: 95,
          bottom: 160,
          fontSize: 31,
          background: "#356fd6",
          padding: "16px 28px",
          borderRadius: 10,
          translate: `${snap(f, 65, 80, -700, 0)}px 0`,
        }}
      >
        按应用匹配专属菜单
      </div>
      <div
        style={{
          position: "absolute",
          right: 90,
          bottom: 85,
          fontSize: 22,
          color: "#b8c9e6",
        }}
      >
        实际操作素材 · 加速演示
      </div>
    </MotionStage>
  );
};

export const Style = () => {
  const f = useCurrentFrame();
  const index = Math.min(3, Math.floor(f / 40));
  const colors = ["#5798ff", "#56bca2", "#bb83df", "#f0a650"];
  return (
    <MotionStage>
      <div
        style={{
          position: "absolute",
          inset: 0,
          background: `radial-gradient(ellipse at 70% 50%,${colors[index]}38,transparent 65%)`,
        }}
      />
      <Words
        text={
          ["换个配色。", "换个心情。", "每个菜单，", "都能有自己的风格。"][
            index
          ]
        }
        top={185}
        size={index === 3 ? 84 : 104}
        delay={index * 40}
      />
      <div
        style={{
          rotate: `${snap(f % 40, 0, 15, -10, 0)}deg`,
          scale: 0.92 + Math.sin(f / 17) * 0.035,
          transformOrigin: "1260px 570px",
        }}
      >
        <Ring x={1260} y={575} size={620} tint={colors[index]} active={index} />
      </div>
      <div
        style={{
          position: "absolute",
          left: 95,
          top: 410,
          width: 615,
          height: 385,
          overflow: "hidden",
          borderRadius: 18,
          rotate: "-3deg",
          boxShadow: "0 30px 80px #0005",
        }}
      >
        <Img
          src={staticFile("assets/colors.png")}
          style={{ position: "absolute", width: 1100, left: -255, top: -95 }}
        />
      </div>
      <div
        style={{
          position: "absolute",
          left: 110,
          top: 850,
          display: "flex",
          gap: 28,
        }}
      >
        {colors.map((c, i) => (
          <div
            key={c}
            style={{
              width: 70,
              height: 70,
              borderRadius: "50%",
              background: c,
              scale: i === index ? 1.3 : 1,
              border: i === index ? "4px solid white" : "4px solid transparent",
            }}
          />
        ))}
      </div>
      <div
        style={{
          position: "absolute",
          right: 150,
          bottom: 85,
          fontSize: 22,
          color: "#aac1df",
        }}
      >
        配色动效示意 · 各菜单外观独立设置
      </div>
    </MotionStage>
  );
};

export const Outro = () => {
  const f = useCurrentFrame();
  return (
    <MotionStage>
      {[0, 1, 2, 3].map((i) => (
        <div
          key={i}
          style={{
            position: "absolute",
            left: 960 - (250 + i * 100),
            top: 460 - (250 + i * 100),
            width: 500 + i * 200,
            height: 500 + i * 200,
            border: "1px solid #75a5ff",
            borderRadius: "50%",
            scale: snap(f, i * 6, 90, 0, 2),
            opacity: snap(f, i * 6, 110, 0.4, 0),
          }}
        />
      ))}
      <Img
        src={staticFile("assets/logo.png")}
        style={{
          position: "absolute",
          left: 875,
          top: 190,
          width: 170,
          height: 170,
          scale: pop(f),
          rotate: `${snap(f, 0, 23, -100, 0)}deg`,
        }}
      />
      <Words
        text="让操作，跟上你的手。"
        top={425}
        left={370}
        size={108}
        delay={8}
      />
      <div
        style={{
          position: "absolute",
          top: 600,
          left: 0,
          right: 0,
          textAlign: "center",
          fontSize: 36,
          color: "#accbff",
          opacity: snap(f, 25, 34, 0, 1),
        }}
      >
        GoPieMenu / Windows 开源圆形快捷菜单
      </div>
      <div
        style={{
          position: "absolute",
          top: 760,
          left: 455,
          width: 1010,
          textAlign: "center",
          fontSize: 38,
          padding: "22px 0",
          background: "#5c96fc",
          borderRadius: 15,
          scale: pop(f, 40),
        }}
      >
        github.com/RyuuMeow/GoPieMenu
      </div>
      <div
        style={{
          position: "absolute",
          top: 875,
          left: 0,
          right: 0,
          textAlign: "center",
          fontSize: 25,
          color: "#9bb7df",
          opacity: snap(f, 45, 55, 0, 1),
        }}
      >
        项目地址见简介 · 做一个属于你的菜单
      </div>
    </MotionStage>
  );
};
