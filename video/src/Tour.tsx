import { useCurrentFrame } from "remotion";
import { FocalText } from "./FocalText";
import { Cursor, pop, snap } from "./motion";
import { DemoStage, LivePie, Keycap, field, panel, presets } from "./DemoUI";

export const Overview = () => {
  const f = useCurrentFrame();
  const chosen = f >= 40;
  return (
    <DemoStage>
      <FocalText text="选个菜单" x={600} y={520} size={105} />
      <div style={{ ...panel, left: 1050, top: 285, width: 570, padding: 28 }}>
        <div style={{ ...field, borderColor: "#507fcf" }}>
          {chosen ? "ClipStudioPaint" : "New Pie Menu"}
          <span style={{ float: "right" }}>⌄</span>
        </div>
        {f >= 9 && f < 48 && (
          <div
            style={{
              marginTop: 15,
              overflow: "hidden",
              height: snap(f, 9, 20, 0, 190),
            }}
          >
            {["New Pie Menu", "ClipStudioPaint"].map((t, i) => (
              <div
                key={t}
                style={{
                  padding: "22px 24px",
                  fontSize: 32,
                  borderRadius: 12,
                  background: i === 1 && f >= 30 ? "#e6eefb" : "white",
                }}
              >
                {t}
              </div>
            ))}
          </div>
        )}
      </div>
      {chosen && (
        <div style={{ scale: pop(f, 40), transformOrigin: "1330px 730px" }}>
          <LivePie x={1330} y={720} size={390} />
        </div>
      )}
      <Cursor
        x={snap(f, 12, 35, 1580, 1320)}
        y={snap(f, 12, 35, 345, 530)}
        click={f >= 38 && f < 45}
      />
    </DemoStage>
  );
};
export const Actions = () => {
  const f = useCurrentFrame();
  const added = f >= 27,
    record = f >= 67;
  return (
    <DemoStage>
      <FocalText
        text={f < 60 ? "添加动作" : "设个快捷键"}
        x={1250}
        y={235}
        size={92}
        delay={f < 60 ? 0 : 60}
      />
      <LivePie
        x={560}
        y={560}
        size={600}
        labels={
          added ? ["变形", "调色", "图层", "新图层"] : ["变形", "调色", "图层"]
        }
        active={added ? 3 : -1}
      />
      <div style={{ ...panel, left: 970, top: 350, width: 630, padding: 36 }}>
        <div
          style={{
            ...field,
            background: "#507fcf",
            color: "white",
            textAlign: "center",
          }}
        >
          ＋ Add action
        </div>
        {f >= 8 && f < 27 && (
          <div style={{ ...field, marginTop: 20, scale: pop(f, 8) }}>
            ⌘ Keyboard shortcut
          </div>
        )}
        {added && (
          <div
            style={{
              opacity: snap(f, 27, 35, 0, 1),
              translate: `0 ${snap(f, 27, 38, 35, 0)}px`,
            }}
          >
            <div style={{ ...field, marginTop: 24 }}>新图层</div>
            <div
              style={{ fontSize: 24, color: "#79889d", margin: "25px 0 15px" }}
            >
              Shortcut
            </div>
            <div
              style={{
                ...field,
                display: "flex",
                alignItems: "center",
                gap: 14,
                minHeight: 90,
                borderColor: record ? "#507fcf" : "#dce4ef",
              }}
            >
              {record ? (
                <>
                  <Keycap active>Ctrl</Keycap>
                  <Keycap active={f >= 72}>Shift</Keycap>
                  <Keycap active={f >= 77}>N</Keycap>
                </>
              ) : (
                <span style={{ color: "#8b97a8" }}>Record shortcut</span>
              )}
            </div>
          </div>
        )}
      </div>
      <Cursor
        x={snap(f, 8, 26, 1530, 1250)}
        y={f < 60 ? snap(f, 8, 26, 420, 550) : snap(f, 60, 67, 550, 675)}
        click={(f >= 25 && f < 31) || (f >= 67 && f < 74)}
      />
    </DemoStage>
  );
};
export const Arrange = () => {
  const f = useCurrentFrame();
  const move = snap(f, 10, 38, 0, 1);
  return (
    <DemoStage>
      <FocalText text="拖动排序" x={560} y={510} size={100} />
      <div
        style={{
          ...panel,
          left: 1030,
          top: 285,
          width: 610,
          height: 480,
          padding: 22,
        }}
      >
        {["变形", "调色", "图层"].map((t, i) => {
          const y = i === 2 ? 292 - move * 256 : 36 + i * 128 + move * 128;
          return (
            <div
              key={t}
              style={{
                ...field,
                position: "absolute",
                left: i === 2 ? 24 + Math.sin(move * Math.PI) * 28 : 24,
                top: y,
                width: 550,
                height: 104,
                boxSizing: "border-box",
                zIndex: i === 2 ? 3 : 1,
                background: i === 2 ? "#e6eefb" : "white",
                borderColor: i === 2 ? "#507fcf" : "#dce4ef",
                boxShadow: i === 2 ? "0 15px 35px #33547a25" : undefined,
                scale: i === 2 ? 1 + Math.sin(move * Math.PI) * 0.045 : 1,
              }}
            >
              <span style={{ marginRight: 32, color: "#8b97a8" }}>☰</span>
              {t}
            </div>
          );
        })}
      </div>
      <Cursor
        x={1120 + Math.sin(move * Math.PI) * 28}
        y={285 + 342 - move * 256}
        click={f >= 8 && f < 14}
      />
    </DemoStage>
  );
};
export const Trigger = () => {
  const f = useCurrentFrame();
  const app = f >= 60;
  const typed = "CLIPStudioPaint.exe".slice(
    0,
    Math.floor(Math.max(0, f - 68) / 1.5),
  );
  return (
    <DemoStage>
      <FocalText
        text={app ? "指定应用" : "设置触发键"}
        x={960}
        y={265}
        size={104}
        delay={app ? 60 : 0}
      />
      <div
        style={{
          ...panel,
          left: 550,
          top: 415,
          width: 820,
          padding: 40,
          scale: pop(f, app ? 60 : 0),
        }}
      >
        <div style={{ fontSize: 28, color: "#79889d", marginBottom: 24 }}>
          {app ? "Available in" : "Keyboard shortcut"}
        </div>
        <div
          style={{
            ...field,
            height: 90,
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
            borderColor: "#507fcf",
            fontSize: 42,
          }}
        >
          {app ? (
            typed
          ) : (
            <Keycap active={f >= 9}>{f >= 9 ? "F12" : "…"}</Keycap>
          )}
        </div>
      </div>
      <Cursor x={1320} y={620} click={f >= 7 && f < 14} />
    </DemoStage>
  );
};
export const Settings = () => {
  const f = useCurrentFrame();
  const backup = f >= 45;
  return (
    <DemoStage>
      <FocalText
        text={backup ? "导入 · 导出" : "开机启动"}
        x={960}
        y={300}
        size={105}
        delay={backup ? 45 : 0}
      />
      <div
        style={{
          ...panel,
          left: 590,
          top: 460,
          width: 740,
          height: 240,
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          gap: 40,
        }}
      >
        {backup ? (
          <>
            <div style={{ ...field }}>↓ Import</div>
            <div
              style={{
                ...field,
                borderColor: f >= 53 ? "#507fcf" : "#dce4ef",
                background: f >= 53 ? "#e6eefb" : "white",
                scale: f >= 53 ? pop(f, 49) : 1,
              }}
            >
              ↑ Export
            </div>
          </>
        ) : (
          <>
            <div style={{ fontSize: 35 }}>Start with Windows</div>
            <div
              style={{
                width: 130,
                height: 66,
                borderRadius: 40,
                background: f >= 8 ? "#2a865e" : "#d5deea",
                position: "relative",
              }}
            >
              <div
                style={{
                  position: "absolute",
                  left: snap(f, 8, 16, 7, 71),
                  top: 7,
                  width: 52,
                  height: 52,
                  borderRadius: "50%",
                  background: "white",
                }}
              />
            </div>
          </>
        )}
      </div>
      <Cursor
        x={backup ? 1130 : 1160}
        y={580}
        click={(f >= 8 && f < 15) || (f >= 53 && f < 60)}
      />
    </DemoStage>
  );
};
export const Apply = () => {
  const f = useCurrentFrame();
  const done = f >= 15;
  return (
    <DemoStage>
      <div
        style={{
          ...panel,
          left: 710,
          top: 420,
          width: 500,
          height: 150,
          background: done ? "#2a865e" : "#507fcf",
          color: "white",
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          fontSize: 72,
          scale: 1 + Math.sin(snap(f, 15, 28, 0, 1) * Math.PI) * 0.07,
        }}
      >
        {done ? "✓" : "Apply"}
      </div>
      {done && <FocalText text="完成" x={960} y={685} size={100} delay={15} />}
      <Cursor
        x={snap(f, 0, 14, 1360, 1120)}
        y={snap(f, 0, 14, 700, 520)}
        click={f >= 15 && f < 22}
      />
    </DemoStage>
  );
};
export const Style = () => {
  const f = useCurrentFrame();
  const index = Math.min(3, Math.floor(f / 45));
  const local = f - index * 45;
  return (
    <DemoStage>
      <LivePie
        x={650}
        y={560}
        size={730}
        preset={index}
        changeAt={index * 45}
        active={local >= 20 ? index % 3 : -1}
      />
      <FocalText
        text={presets[index].name}
        x={1400}
        y={370}
        size={116}
        delay={index * 45}
      />
      <div
        style={{
          ...panel,
          left: 1070,
          top: 530,
          width: 680,
          padding: 22,
          display: "flex",
          gap: 10,
        }}
      >
        {presets.map((p, i) => (
          <div
            key={p.name}
            style={{
              ...field,
              padding: "18px 16px",
              fontSize: 27,
              background: i === index ? "#507fcf" : "white",
              color: i === index ? "white" : "#253348",
              borderColor: i === index ? "#507fcf" : "#dce4ef",
            }}
          >
            {p.name}
          </div>
        ))}
      </div>
      <Cursor x={1130 + index * 162} y={610} click={local >= 3 && local < 10} />
    </DemoStage>
  );
};
