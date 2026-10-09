import { Img, staticFile, useCurrentFrame } from "remotion";
import { Cursor, MotionStage, Ring, Words, pop, snap } from "./motion";

// Runtime behavior follows PieMenuWidget: hover a folder, then release on a row.
export const Submenu = () => {
  const f = useCurrentFrame();
  const expanded = f >= 48;
  const selected = f >= 100;
  const executed = f >= 130;
  const editor = f >= 160;
  return (
    <MotionStage>
      <Words
        text={
          editor
            ? "双击编辑"
            : executed
              ? "松开执行"
              : expanded
                ? "再选动作"
                : "打开分类"
        }
        top={155}
        size={86}
        delay={editor ? 160 : executed ? 130 : expanded ? 48 : 0}
      />
      {!editor && (
        <>
          <div
            style={{
              transform: `translateX(${snap(f, 30, 60, 160, 0)}px) scale(${executed ? snap(f, 130, 148, 1, 1.22) : 1})`,
              transformOrigin: "720px 600px",
              opacity: executed ? snap(f, 130, 147, 1, 0) : 1,
            }}
          >
            <Ring
              x={710}
              y={605}
              size={560}
              active={expanded ? 1 : -1}
              tint="#507fcf"
              labels={["应用", "图层 ›", "文件", "网址"]}
            />
          </div>
          {expanded && !executed && (
            <div
              style={{
                position: "absolute",
                left: 1040,
                top: 430,
                width: 625,
                padding: 14,
                borderRadius: 22,
                background: "#edf3fc",
                boxShadow: "0 30px 100px #355b8520",
                transform: `perspective(1200px) rotateY(${snap(f, 48, 65, -55, 0)}deg)`,
                transformOrigin: "left center",
              }}
            >
              {["新图层", "切换图层显示", "锁定透明度"].map((label, i) => (
                <div
                  key={label}
                  style={{
                    height: 96,
                    margin: 5,
                    padding: "0 26px",
                    display: "flex",
                    alignItems: "center",
                    justifyContent: "space-between",
                    borderRadius: 12,
                    background: selected && i === 0 ? "#4f90dc" : "#e7edf6",
                    color: selected && i === 0 ? "white" : "#203752",
                    fontSize: 38,
                    opacity: snap(f, 50 + i * 5, 58 + i * 5, 0, 1),
                    translate: `${snap(f, 50 + i * 5, 70 + i * 5, 130, 0)}px 0`,
                  }}
                >
                  <span>{label}</span>
                  <span style={{ fontSize: 25, opacity: 0.6 }}>
                    {["Ctrl+Shift+N", "", ""][i]}
                  </span>
                </div>
              ))}
            </div>
          )}
          {!executed && (
            <Cursor
              x={
                f < 55 ? snap(f, 10, 53, 720, 900) : snap(f, 70, 100, 900, 1230)
              }
              y={f < 55 ? 605 : snap(f, 70, 100, 605, 490)}
            />
          )}
          {executed && (
            <>
              {[0, 1, 2].map((i) => (
                <div
                  key={i}
                  style={{
                    position: "absolute",
                    left: 1230 - 160,
                    top: 560 - 160,
                    width: 320,
                    height: 320,
                    border: "2px solid #507fcf",
                    borderRadius: "50%",
                    scale: snap(f, 130 + i * 3, 160, 0.2, 4),
                    opacity: snap(f, 130 + i * 3, 160, 0.8, 0),
                  }}
                />
              ))}
              <div
                style={{
                  position: "absolute",
                  left: 1060,
                  top: 465,
                  fontSize: 72,
                  fontWeight: 800,
                  scale: pop(f, 130),
                  color: "#507fcf",
                }}
              >
                ✓ 新图层
              </div>
            </>
          )}
        </>
      )}
      {editor && (
        <>
          <div
            style={{
              position: "absolute",
              left: 300,
              top: 335,
              width: 1320,
              height: 640,
              overflow: "hidden",
              borderRadius: 22,
              boxShadow: "0 30px 100px #355b8520",
              transform: `perspective(1800px) rotateY(${snap(f, 160, 181, -16, 0)}deg) translateY(${snap(f, 160, 180, 140, 0)}px)`,
            }}
          >
            <Img
              src={staticFile(
                f < 190
                  ? "assets/submenu-parent.png"
                  : f < 225
                    ? "assets/submenu-live.png"
                    : "assets/submenu-editor.png",
              )}
              style={{
                width: 1320,
                transform: `scale(${snap(f, 225, 265, 1, 1.12)})`,
                transformOrigin: "65% 45%",
              }}
            />
          </div>
        </>
      )}
      <div
        style={{
          position: "absolute",
          right: 100,
          bottom: 50,
          fontSize: 21,
          color: "#64758b",
        }}
      >
        {editor ? "界面截图" : "操作示意"}
      </div>
    </MotionStage>
  );
};
