import { useLanguage } from "./Language";
import {
  AbsoluteFill,
  Audio,
  getStaticFiles,
  Sequence,
  Series,
} from "remotion";
import { Cut } from "./motion";
import { FPS, scenes } from "./timeline";
import { sceneComponents } from "./sceneRegistry";
import { Soundtrack } from "./Soundtrack";
export const Promo = () => {
  const files = getStaticFiles();
  const voiceFolder = useLanguage() === "en" ? "voice/en" : "voice";
  return (
    <AbsoluteFill>
      <Soundtrack
        voiceIds={scenes
          .filter((scene) =>
            files.some(
              (file) => file.name === `${voiceFolder}/${scene.id}.wav`,
            ),
          )
          .map((scene) => scene.id)}
      />
      <Series>
        {scenes.map((scene, index) => {
          const Component = sceneComponents[scene.id];
          return (
            <Series.Sequence
              key={scene.id}
              name={`${String(index + 1).padStart(2, "0")} ${scene.label}`}
              durationInFrames={scene.seconds * FPS}
            >
              {scene.id === "outro" ? (
                <Component />
              ) : (
                <Cut duration={scene.seconds * FPS}>
                  <Component />
                </Cut>
              )}
            </Series.Sequence>
          );
        })}
      </Series>
      {scenes.map((scene) => {
        const voice = files.find(
          (file) => file.name === `${voiceFolder}/${scene.id}.wav`,
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
    </AbsoluteFill>
  );
};
