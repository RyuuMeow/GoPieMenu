import { LanguageContext } from "./Language";
import { Composition, Folder } from "remotion";
import { Promo } from "./Promo";
import { sceneComponents } from "./sceneRegistry";
import { FPS, scenes, TOTAL_FRAMES } from "./timeline";
const EnglishPromo = () => (
  <LanguageContext.Provider value="en">
    <Promo />
  </LanguageContext.Provider>
);
export const RemotionRoot = () => (
  <>
    <Composition
      id="GoPieMenu-Bilibili"
      component={Promo}
      width={1920}
      height={1080}
      fps={FPS}
      durationInFrames={TOTAL_FRAMES}
    />
    <Composition
      id="GoPieMenu-English"
      component={EnglishPromo}
      width={1920}
      height={1080}
      fps={FPS}
      durationInFrames={TOTAL_FRAMES}
    />
    <Folder name="Scenes">
      {scenes.map((scene) => (
        <Composition
          key={scene.id}
          id={scene.id[0].toUpperCase() + scene.id.slice(1)}
          component={sceneComponents[scene.id]}
          width={1920}
          height={1080}
          fps={FPS}
          durationInFrames={scene.seconds * FPS}
        />
      ))}
    </Folder>
  </>
);
