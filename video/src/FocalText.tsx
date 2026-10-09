import { useCurrentFrame } from "remotion";
import { snap } from "./motion";

// Coordinates describe the visual center, so short titles stay aligned to their subject.
export const FocalText = ({
  text,
  x = 960,
  y = 540,
  size = 100,
  delay = 0,
  badge = false,
}: {
  text: string;
  x?: number;
  y?: number;
  size?: number;
  delay?: number;
  badge?: boolean;
}) => {
  const frame = useCurrentFrame() - delay;
  return (
    <div
      style={{
        position: "absolute",
        left: x,
        top: y,
        zIndex: 12,
        transform: `translate(-50%, -50%) translateY(${snap(frame, 0, 6, 18, 0)}px) scale(${snap(frame, 0, 7, 0.92, 1)})`,
        opacity: snap(frame, 0, 4, 0, 1),
        fontSize: size,
        fontWeight: 850,
        lineHeight: 1.1,
        letterSpacing: -2,
        whiteSpace: "nowrap",
        color: "#253348",
        padding: badge ? "18px 32px" : 0,
        borderRadius: 18,
        background: badge ? "#ffffffed" : undefined,
        boxShadow: badge ? "0 8px 32px #33547a14" : undefined,
      }}
    >
      {text}
    </div>
  );
};
