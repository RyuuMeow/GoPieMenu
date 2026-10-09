import { Composition, Folder } from "remotion";
import { Promo } from "./Promo";
import { Submenu } from "./Submenu";
import { TOTAL_FRAMES } from "./timeline";
import { Actions, Gesture, Hook, Outro, Style, Workflow } from "./scenes";

export const RemotionRoot: React.FC = () => {
  return (
    <>
      <Composition
        id="GoPieMenu-Bilibili"
        component={Promo}
        width={1920}
        height={1080}
        fps={30}
        durationInFrames={TOTAL_FRAMES}
      />
      <Folder name="Scenes">
        <Composition
          id="Submenu"
          component={Submenu}
          width={1920}
          height={1080}
          fps={30}
          durationInFrames={270}
        />
        <Composition
          id="Hook"
          component={Hook}
          width={1920}
          height={1080}
          fps={30}
          durationInFrames={120}
        />
        <Composition
          id="Gesture"
          component={Gesture}
          width={1920}
          height={1080}
          fps={30}
          durationInFrames={150}
        />
        <Composition
          id="Actions"
          component={Actions}
          width={1920}
          height={1080}
          fps={30}
          durationInFrames={180}
        />
        <Composition
          id="Workflow"
          component={Workflow}
          width={1920}
          height={1080}
          fps={30}
          durationInFrames={180}
        />
        <Composition
          id="Style"
          component={Style}
          width={1920}
          height={1080}
          fps={30}
          durationInFrames={180}
        />
        <Composition
          id="Outro"
          component={Outro}
          width={1920}
          height={1080}
          fps={30}
          durationInFrames={150}
        />
      </Folder>
    </>
  );
};
