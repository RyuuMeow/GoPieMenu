import { AnimatedImage, Img, staticFile, useCurrentFrame } from "remotion";
import { Cursor, MotionStage, Ring, Words, pop, snap } from "./motion";
const note = {
  position: "absolute" as const,
  left: 96,
  top: 470,
  fontSize: 38,
  lineHeight: 1.8,
  color: "#68788e",
};
export const Hook = () => {
  const f = useCurrentFrame();
  return (
    <MotionStage>
      <Words text="就在手边" top={230} size={96} />
      <div
        style={{
          transformOrigin: "1310px 620px",
          scale: snap(f, 75, 119, 1, 1.12),
        }}
      >
        <Ring
          x={1320}
          y={600}
          size={590}
          active={Math.min(3, Math.floor(f / 30))}
        />
      </div>
      <Cursor x={snap(f, 20, 65, 1550, 1310)} y={snap(f, 20, 65, 800, 410)} />
      <div style={{ ...note, top: 830, fontSize: 24 }}>动效示意</div>
    </MotionStage>
  );
};
export const Gesture = () => {
  const f = useCurrentFrame();
  const phase = Math.min(2, Math.floor(f / 40));
  return (
    <MotionStage>
      <Words
        text={["按住。", "划过去。", "松开，执行。"][phase]}
        top={240}
        size={104}
        delay={phase * 40}
      />
      <div
        style={{
          opacity: snap(f, 85, 98, 1, 0),
          transformOrigin: "1300px 560px",
          scale: snap(f, 85, 100, 1, 1.2),
        }}
      >
        <Ring x={1300} y={560} size={640} active={phase ? 0 : -1} />
      </div>
      <Cursor
        x={snap(f, 28, 62, 1430, 1300)}
        y={snap(f, 28, 62, 720, 355)}
        click={f > 85}
      />
      {phase === 2 && (
        <div
          style={{
            position: "absolute",
            left: 1100,
            top: 480,
            fontSize: 70,
            color: "#507fcf",
            scale: pop(f, 84),
          }}
        >
          ✓
        </div>
      )}
      <div style={{ ...note, top: 850, fontSize: 24 }}>
        Ctrl + 右键 · 操作示意
      </div>
    </MotionStage>
  );
};
export const Workflow = () => (
  <MotionStage>
    <Words text="回到创作" top={150} size={86} />
    <div
      style={{
        position: "absolute",
        left: 390,
        top: 280,
        width: 1140,
        height: 680,
        overflow: "hidden",
        borderRadius: 24,
        boxShadow: "0 20px 70px #355b8520",
      }}
    >
      <AnimatedImage
        src={staticFile("assets/drawing-demo.gif")}
        width={1140}
        height={876}
        fit="cover"
        playbackRate={1.6}
      />
    </div>
    <div style={{ ...note, top: 860, fontSize: 22 }}>
      实际操作 GIF · 加速演示
    </div>
  </MotionStage>
);
export const Outro = () => {
  const f = useCurrentFrame();
  return (
    <MotionStage>
      {[0, 1, 2].map((i) => (
        <div
          key={i}
          style={{
            position: "absolute",
            left: 660 - i * 100,
            top: 100 - i * 100,
            width: 600 + i * 200,
            height: 600 + i * 200,
            border: "1px solid #b7cdeb",
            borderRadius: "50%",
            scale: snap(f, i * 7, 100, 0.5, 1.5),
            opacity: snap(f, i * 7, 110, 0.7, 0),
          }}
        />
      ))}
      <Img
        src={staticFile("assets/logo.png")}
        style={{
          position: "absolute",
          left: 885,
          top: 160,
          width: 150,
          height: 150,
          scale: pop(f),
        }}
      />
      <Words text="GoPieMenu" top={410} left={370} size={108} delay={7} />
      <div
        style={{
          position: "absolute",
          top: 595,
          width: "100%",
          textAlign: "center",
          fontSize: 34,
          color: "#68788e",
        }}
      >
        Windows 开源快捷菜单
      </div>
      <div
        style={{
          position: "absolute",
          left: 450,
          top: 745,
          width: 1140,
          padding: "25px 0",
          borderRadius: 14,
          background: "#507fcf",
          color: "white",
          textAlign: "center",
          fontSize: 38,
          scale: pop(f, 24),
        }}
      >
        github.com/RyuuMeow/GoPieMenu
      </div>
      <div
        style={{
          position: "absolute",
          top: 885,
          width: "100%",
          textAlign: "center",
          fontSize: 24,
          color: "#68788e",
        }}
      >
        项目地址见简介
      </div>
    </MotionStage>
  );
};
