import { Audio, interpolate, Sequence, staticFile } from "remotion";
import { FPS, scenes, TOTAL_FRAMES, type SceneId } from "./timeline";

type Sound = "pop" | "tick" | "swipe" | "confirm";
// Frame offsets follow the visual events within each scene, independent of scene starts.
const cues: Partial<Record<SceneId, Array<[number, Sound]>>> = {
  hook: [[4, "pop"]],
  gesture: [
    [0, "tick"],
    [30, "swipe"],
    [60, "confirm"],
  ],
  overview: [[9, "pop"]],
  actions: [
    [8, "pop"],
    [67, "tick"],
  ],
  arrange: [[10, "swipe"]],
  submenu: [
    [5, "pop"],
    [32, "pop"],
    [67, "tick"],
    [87, "confirm"],
    [107, "swipe"],
  ],
  trigger: [
    [9, "tick"],
    [69, "tick"],
  ],
  style: [
    [4, "pop"],
    [49, "pop"],
    [94, "pop"],
    [139, "pop"],
  ],
  settings: [
    [8, "tick"],
    [53, "tick"],
  ],
  apply: [[15, "confirm"]],
  workflow: [[3, "swipe"]],
  outro: [[8, "confirm"]],
};
const lengths: Record<Sound, number> = {
  pop: 7,
  tick: 3,
  swipe: 8,
  confirm: 20,
};
export const Soundtrack = ({ voiceIds }: { voiceIds: string[] }) => (
  <>
    <Audio
      src={staticFile("audio/music.wav")}
      volume={(frame) => {
        const fade = interpolate(
          frame,
          [0, 12, TOTAL_FRAMES - 45, TOTAL_FRAMES - 1],
          [0, 1, 1, 0],
          { extrapolateLeft: "clamp", extrapolateRight: "clamp" },
        );
        const duck = scenes.reduce((level, scene) => {
          if (!voiceIds.includes(scene.id)) return level;
          const start = scene.start * FPS,
            end = (scene.start + scene.seconds) * FPS;
          const amount = interpolate(
            frame,
            [start - 8, start, end - 1, end + 10],
            [0, 1, 1, 0],
            { extrapolateLeft: "clamp", extrapolateRight: "clamp" },
          );
          return Math.min(level, 1 - amount * 0.55);
        }, 1);
        return 0.48 * fade * duck;
      }}
    />
    {scenes.flatMap((scene) =>
      (cues[scene.id] ?? []).map(([offset, sound]) => (
        <Sequence
          key={`${scene.id}-${offset}`}
          name={`音效 ${sound}`}
          from={scene.start * FPS + offset}
          durationInFrames={lengths[sound]}
        >
          <Audio
            src={staticFile(`audio/${sound}.wav`)}
            volume={sound === "swipe" ? 0.3 : 0.5}
          />
        </Sequence>
      )),
    )}
  </>
);
