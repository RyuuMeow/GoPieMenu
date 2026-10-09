import {
  AbsoluteFill,
  Audio,
  getStaticFiles,
  Sequence,
  Series,
  useCurrentFrame,
} from "remotion";
import { Actions, Gesture, Hook, Outro, Style, Workflow } from "./scenes";
import { FPS, scenes, TOTAL_FRAMES } from "./timeline";

export const Promo = () => {
  const frame = useCurrentFrame();
  const files = getStaticFiles();
  return (
    <AbsoluteFill>
      <Series>
        <Series.Sequence name="01 开场" durationInFrames={180}>
          <Hook />
        </Series.Sequence>
        <Series.Sequence name="02 操作手势" durationInFrames={240}>
          <Gesture />
        </Series.Sequence>
        <Series.Sequence name="03 自定义动作" durationInFrames={270}>
          <Actions />
        </Series.Sequence>
        <Series.Sequence name="04 应用工作流" durationInFrames={270}>
          <Workflow />
        </Series.Sequence>
        <Series.Sequence name="05 外观" durationInFrames={240}>
          <Style />
        </Series.Sequence>
        <Series.Sequence name="06 项目入口" durationInFrames={240}>
          <Outro />
        </Series.Sequence>
      </Series>
      {scenes.map((scene) => {
        const voice = files.find(
          (file) => file.name === `voice/${scene.id}.wav`,
        );
        return voice ? (
          <Sequence
            key={scene.id}
            name={`旁白 ${scene.id}`}
            from={scene.start * FPS}
            durationInFrames={scene.seconds * FPS}
          >
            <Audio src={voice.src} />
          </Sequence>
        ) : null;
      })}
      <div
        style={{
          position: "absolute",
          bottom: 0,
          height: 5,
          width: `${((frame + 1) / TOTAL_FRAMES) * 100}%`,
          background: "#568ce1",
        }}
      />
    </AbsoluteFill>
  );
};
