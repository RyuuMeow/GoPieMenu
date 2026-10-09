import type { ComponentType } from "react";
import { Hook, Gesture, Workflow, Outro } from "./scenes";
import {
  Overview,
  Actions,
  Arrange,
  Trigger,
  Style,
  Settings,
  Apply,
} from "./Tour";
import { Submenu } from "./Submenu";
import type { SceneId } from "./timeline";
export const sceneComponents: Record<SceneId, ComponentType> = {
  hook: Hook,
  gesture: Gesture,
  overview: Overview,
  actions: Actions,
  arrange: Arrange,
  submenu: Submenu,
  trigger: Trigger,
  style: Style,
  settings: Settings,
  apply: Apply,
  workflow: Workflow,
  outro: Outro,
};
