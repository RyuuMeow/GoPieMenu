import {
  AbsoluteFill,
  Audio,
  getStaticFiles,
  Sequence,
  Series,
} from "remotion";
import { Actions, Gesture, Hook, Outro, Style, Workflow } from "./scenes";
import { Cut } from "./motion";
import { Submenu } from "./Submenu";
import { FPS, scenes } from "./timeline";

export const Promo = () => {
  const files = getStaticFiles();
  return (
    <AbsoluteFill>
      <Series>
        <Series.Sequence name="01 直接叫到手边" durationInFrames={120}>
          <Cut duration={120}>
            <Hook />
          </Cut>
        </Series.Sequence>
        <Series.Sequence name="02 按住划过松开" durationInFrames={150}>
          <Cut duration={150}>
            <Gesture />
          </Cut>
        </Series.Sequence>
        <Series.Sequence name="03 二级菜单" durationInFrames={270}>
          <Cut duration={270}>
            <Submenu />
          </Cut>
        </Series.Sequence>
        <Series.Sequence name="04 动作与编辑" durationInFrames={180}>
          <Cut duration={180}>
            <Actions />
          </Cut>
        </Series.Sequence>
        <Series.Sequence name="05 工作流快切" durationInFrames={180}>
          <Cut duration={180}>
            <Workflow />
          </Cut>
        </Series.Sequence>
        <Series.Sequence name="06 连续换色" durationInFrames={180}>
          <Cut duration={180}>
            <Style />
          </Cut>
        </Series.Sequence>
        <Series.Sequence name="07 项目入口" durationInFrames={150}>
          <Outro />
        </Series.Sequence>
      </Series>
      {scenes.map((s) => {
        const voice = files.find((file) => file.name === `voice/${s.id}.wav`);
        return voice ? (
          <Sequence
            key={s.id}
            name={`旁白 ${s.id}`}
            from={s.start * FPS}
            durationInFrames={s.seconds * FPS}
          >
            <Audio src={voice.src} />
          </Sequence>
        ) : null;
      })}
    </AbsoluteFill>
  );
};
