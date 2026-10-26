"""A1 look check of the dev overlay's Lighting > Features section: one view,
the panel shown on its Features section (triDevPanelLighting), shot at present
(after the overlay) in three states: remastered, remastered with two switches
off and the gates forced, and baseline. Run from this folder (capture_lib.sqf
copied beside it)."""
import sys
from pathlib import Path

sys.path.insert(0, r"D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\tests\rendering\captures")
import capgen  # noqa: E402
import views  # noqa: E402

HERE = Path(__file__).resolve().parent
v = next(x for x in views.HERO if x["name"] == "shade_lowsun_fp")
lines = [capgen.header("Abel", v, "A1 overlay check: Lighting tab, Features section")]
lines += capgen.begin(v, "overlay_lowsun_fp", sim_time=2000)
lines.append("triSimFrames 150")
for i, (setting, label) in enumerate([("remastered", "overlay__remastered"),
                                      ("remastered,taa=0,combatLights=0,gates=0", "overlay__two_off_gates_forced"),
                                      ("baseline", "overlay__baseline")]):
    lines += [f'"{setting}" call capMode', "[] call capPin", "call { capPanel = triDevPanelLighting true }",
              "triSimFrames 10", f'triScreenshot "{label}"', f'"{label}" call capRecord']
lines += ["call { capPanel = triDevPanelLighting false }"]
lines += capgen.end(v)
lines += ['\ncall { "view=done;event=done" call capLog }', "triEndTest"]
capgen.write(HERE / "overlay_check.test.sqf", "\n".join(lines) + "\n")
# The dev panel draws only in dev mode (AppConfig::DevMode, --dev).
toml = capgen.toml("Abel", "A1 overlay check", views.WORLDS, timeout=1800).replace('"--no-strict"', '"--no-strict", "--dev"')
capgen.write(HERE / "overlay_check.test.toml", toml)
print("ok")
