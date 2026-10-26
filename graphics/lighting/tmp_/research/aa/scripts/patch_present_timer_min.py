"""Research track aa: add the per-window minimum to the present-pass timer
(contention from other games on the shared GPU only ever adds time, so the
minimum over the 90 settle frames is the cleanest cost estimate)."""
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gfx-aa\arma_CWR\engine\PoseidonGL33")


def rep(s, old, new):
    assert old in s, old[:80]
    return s.replace(old, new)


p = ROOT / "EngineGL33_VertexBuffer.cpp"
s = p.read_text(encoding="utf-8")
s = rep(s, "    double avg = 0.0;\n    int n = 0;\n", "    double avg = 0.0;\n    double minMs = 0.0;\n    int n = 0;\n")
s = rep(s, "                avg = n == 0 ? ms : avg + 0.1 * (ms - avg);\n",
        "                avg = n == 0 ? ms : avg + 0.1 * (ms - avg);\n                minMs = n == 0 ? ms : (ms < minMs ? ms : minMs);\n")
s = rep(s, "    s_presentTimer.avg = 0.0;\n", "    s_presentTimer.avg = 0.0;\n    s_presentTimer.minMs = 0.0;\n")
s = rep(s, '''bool AaPresentTimerMs(float& ms)
{
    ms = static_cast<float>(s_presentTimer.avg);''', '''bool AaPresentTimerMs(float& ms, float& minMs)
{
    ms = static_cast<float>(s_presentTimer.avg);
    minMs = static_cast<float>(s_presentTimer.minMs);''')
p.write_text(s, encoding="utf-8", newline="")

p = ROOT / "EngineGL33_GpuTimer.cpp"
s = p.read_text(encoding="utf-8")
s = rep(s, "bool AaPresentTimerMs(float& ms);", "bool AaPresentTimerMs(float& ms, float& minMs);")
s = rep(s, '''    float presentMs = 0.0f;
    if (AaPresentTimerMs(presentMs))
    {
        char buf[48];
        std::snprintf(buf, sizeof(buf), " | present %.3f", presentMs);''', '''    float presentMs = 0.0f, presentMin = 0.0f;
    if (AaPresentTimerMs(presentMs, presentMin))
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), " | present %.3f min %.3f", presentMs, presentMin);''')
p.write_text(s, encoding="utf-8", newline="")
print("patched")
