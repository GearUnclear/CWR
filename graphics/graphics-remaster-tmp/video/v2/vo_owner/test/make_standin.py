"""Build a fake "owner session" for testing owner_vo.py: the rejected ElevenLabs takes
(Sarah, Lily) laid out the way the owner was asked to record, plus deliberate mess.

    <resolve-edit venv python> make_standin.py [--device cuda|cpu]

Writes, next to this file:
    session_plain.wav     48 kHz mono 24-bit, no spoken numbers
    session_numbered.wav  the same with number words (Windows SAPI voice, numbers/*.wav)
    truth_plain.json, truth_numbered.json   where every take really is

Mess (both versions unless noted):
  - 5 s of room tone first; a quiet noise bed and a 50 Hz hum (with 100 and 150 Hz) throughout
  - line 1 has three takes (an extra take)
  - line 4: take 2 stops halfway, mid-word, then 1.1 s later the whole take again (a restart)
  - line 6: plain: starts 0.3 s after line 5's last take (no gap); numbered: "six" runs straight into it
  - line 7: a cough 0.7 s after take 1
  - line 10 (convoy): a cough 0.6 s before its first take
  - lines 9 and 10 are recorded in swapped order (10 before 9)
  - line 13: take 2 slips in its second sentence, pauses 0.9 s and says the second sentence again
  - line 12: a third take in the shorter wording ("street" trimmed out)
  - line 2: take 1 (Sarah) is hard-clipped
  - numbered only: line 5 announced "line five", line 8 not announced, line 14 announced "thirteen"
"""
import argparse
import json
import subprocess
import wave
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
V2 = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2")
FF = "C:/ffmpeg/bin/ffmpeg.exe"
SR = 48000
NUMW = ["one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve",
        "thirteen", "fourteen", "fifteen", "sixteen"]


def load(p):
    r = subprocess.run([FF, "-v", "error", "-i", str(p), "-f", "f32le", "-ac", "1", "-ar", str(SR), "-"],
                       capture_output=True, check=True)
    return np.frombuffer(r.stdout, np.float32).copy()


def write24(p, x):
    y = np.clip(x, -1.0, 1.0 - 1.0 / 8388608)
    i = np.round(y * 8388607).astype("<i4")
    b = i.view(np.uint8).reshape(-1, 4)[:, :3].tobytes()
    with wave.open(str(p), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(3)
        w.setframerate(SR)
        w.writeframes(b)


def env_db(x, hop=480):
    n = len(x) // hop
    f = x[:n * hop].reshape(n, hop)
    return 10 * np.log10(np.mean(f * f, axis=1) + 1e-12)


def trim_digital(x, rel=-50):
    """Strip the TTS's own leading and trailing near-silence (keeps 20 ms)."""
    d = env_db(x)
    on = np.where(d > d.max() + rel)[0]
    a = max(0, on[0] * 480 - 960)
    b = min(len(x), (on[-1] + 1) * 480 + 960)
    return x[a:b]


def active_rms_db(x):
    d = env_db(x)
    return float(np.mean(d[d > d.max() - 30]))


def pink(n, rng):
    w = rng.standard_normal(n).astype(np.float32)
    f = np.fft.rfft(w)
    k = np.arange(len(f))
    k[0] = 1
    f = f / np.sqrt(k)
    y = np.fft.irfft(f, n).astype(np.float32)
    return y / (np.sqrt(np.mean(y * y)) + 1e-12)


def cough(rng):
    out = []
    for dur, amp in ((0.22, 1.0), (0.18, 0.7)):
        n = int(dur * SR)
        nz = rng.standard_normal(n).astype(np.float32)
        f = np.fft.rfft(nz)
        fr = np.fft.rfftfreq(n, 1 / SR)
        f[(fr < 250) | (fr > 3500)] = 0
        nz = np.fft.irfft(f, n).astype(np.float32)
        nz /= np.abs(nz).max()
        t = np.arange(n) / SR
        e = np.minimum(1, t / 0.012) * np.exp(-t / 0.07)
        out.append(nz * e * amp)
        out.append(np.zeros(int(0.09 * SR), np.float32))
    y = np.concatenate(out)
    return y * (10 ** (-14 / 20)) / np.abs(y).max()


def cut_word(x, t0, t1):
    """x with [t0, t1) removed, both cut points moved to the quietest 10 ms within 40 ms, 5 ms crossfade."""
    def quiet(t):
        c = int(t * SR)
        lo, hi = max(0, c - int(0.04 * SR)), min(len(x), c + int(0.04 * SR))
        best, bi = 1e9, c
        for s in range(lo, hi - 480, 48):
            e = float(np.mean(x[s:s + 480] ** 2))
            if e < best:
                best, bi = e, s + 240
        return bi
    a, b = quiet(t0), quiet(t1)
    fade = int(0.005 * SR)
    head, tail = x[:a].copy(), x[b:].copy()
    head[-fade:] *= np.linspace(1, 0, fade, dtype=np.float32)
    tail[:fade] *= np.linspace(0, 1, fade, dtype=np.float32)
    return np.concatenate([head, tail])


def build(lines, takes, shorter12, numbered, rng):
    ids = [ln["id"] for ln in lines]
    order = list(range(16))
    order[8], order[9] = order[9], order[8]          # line 10 (convoy) before line 9 (ambush)
    nums = {k: load(HERE / "numbers" / f"{w}.wav") for k, w in enumerate(NUMW)}
    nums["line five"] = load(HERE / "numbers" / "line_five.wav")
    nums = {k: trim_digital(v) for k, v in nums.items()}
    nums = {k: v * min(10 ** ((-22 - active_rms_db(v)) / 20), 10 ** (-6 / 20) / np.abs(v).max()) for k, v in nums.items()}
    parts, truth, t = [], [], 0.0

    def add(x, kind=None, **info):
        nonlocal t
        if kind:
            truth.append(dict(kind=kind, start=round(t, 3), end=round(t + len(x) / SR, 3), **info))
        parts.append(x.astype(np.float32))
        t += len(x) / SR

    def gap(s):
        add(np.zeros(int(s * SR), np.float32))

    gap(5.0)                                            # room tone
    cough_at = []
    for pos, li in enumerate(order):
        rid = ids[li]
        a, b = takes[rid]
        seq = [("take", a, "sarah", "main"), ("take", b, "lily", "main")]
        if li == 0:
            seq.append(("take", a * 0.85, "sarah", "main"))
        if li == 11:
            seq.append(("take", shorter12, "sarah", "shorter"))
        if li == 3:                                     # take 2 cut off mid-word, then restarted
            n = int(len(b) * 0.55)
            frag = b[:n].copy()
            frag[-48:] *= np.linspace(1, 0, 48, dtype=np.float32)
            seq = [seq[0], ("frag", frag, "lily", "main"), ("take", b, "lily", "main")]
        # the number (or none) before the line
        if li == 5:
            if numbered:
                gap(2.0)
                add(nums[5], "number", said="six")
                gap(0.12)                                # "six" runs straight into the line
            else:
                gap(0.3)                                 # no gap after line 5
        else:
            if pos:
                gap(1.6)
            if numbered and li != 7:                     # line 8 is never announced
                if li == 4:
                    add(nums["line five"], "number", said="line five")
                elif li == 13:
                    add(nums[12], "number", said="thirteen (wrong: line 14 follows)")
                else:
                    add(nums[li], "number", said=NUMW[li])
                gap(1.0)
            elif not numbered:
                gap(0.6)
        if li == 9:                                      # a cough 0.6 s before the convoy line
            gap(0.4)
            c = cough(rng)
            cough_at.append(round(t, 3))
            add(c, "cough")
            gap(0.6)
        k = 0
        for j, (kind, x, voice, wording) in enumerate(seq):
            if j:
                gap(2.0 if kind != "take" or seq[j - 1][0] != "frag" else 1.1)
            if kind == "frag":
                add(x, "fragment", line=rid, voice=voice)
                continue
            k += 1
            add(x, "take", line=rid, take=k, voice=voice, wording=wording, secs=round(len(x) / SR, 3),
                restart=(li == 12 and k == 2))
            if li == 6 and k == 1:                       # a cough in the gap after take 1
                gap(0.7)
                c = cough(rng)
                add(c, "cough")
                gap(2.0 - 0.7 - len(c) / SR)
                continue
        gap(0.4)
    gap(3.0)
    voice = np.concatenate(parts)
    n = len(voice)
    tt = np.arange(n) / SR
    bed = pink(n, rng) * (10 ** (-64 / 20))
    hum = (10 ** (-56 / 20)) * np.sin(2 * np.pi * 50 * tt) + (10 ** (-62 / 20)) * np.sin(2 * np.pi * 100 * tt + 0.3) \
        + (10 ** (-66 / 20)) * np.sin(2 * np.pi * 150 * tt + 1.1)
    return voice + bed + hum.astype(np.float32), truth


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--device", default="cuda")
    a = ap.parse_args()
    rng = np.random.default_rng(7)
    lines = json.loads((V2 / "lines.json").read_text(encoding="utf-8"))
    takes = {}
    for ln in lines:
        pair = []
        for voice in ("sarah", "lily"):
            x = trim_digital(load(V2 / "vo_eleven" / voice / f"{ln['id']}.wav"))
            g = 10 ** ((-22 + rng.uniform(-2, 2) - active_rms_db(x)) / 20)
            g = min(g, 10 ** (-4 / 20) / np.abs(x).max())          # peaks at -4 dBFS or lower
            if ln["id"] == "v2_01_opening" and voice == "sarah":
                g = 10 ** (4 / 20) / np.abs(x).max()                # deliberately clipped (+4 dBFS, hard clip)
            x = np.clip(x * g, -1.0, 1.0)
            pair.append(x.astype(np.float32))
        takes[ln["id"]] = pair
    # line 12's shorter wording: Sarah's take with "street" cut out ("At night, lamps and the fire ...")
    from faster_whisper import WhisperModel
    model = WhisperModel("large-v3", device=a.device, compute_type="float16" if a.device == "cuda" else "int8",
                         local_files_only=True)
    src = takes["v2_11_town_patrol"][0]
    tmp = HERE / "_town.wav"
    write24(tmp, src)
    segs, _ = model.transcribe(str(tmp), language="en", word_timestamps=True, beam_size=5)
    words = [w for s in segs for w in s.words]
    tmp.unlink()
    i = next(k for k, w in enumerate(words) if w.word.strip().lower().startswith("street"))
    print("trim:", " ".join(w.word.strip() for w in words[:i + 2]), words[i].start, words[i + 1].start)
    # from just after "night," (whisper starts "street" late, after its "s") to the start of "lamps"
    shorter12 = cut_word(src, words[i - 1].end + 0.02, words[i + 1].start)
    # line 13, take 2 (Lily): "Then the rain." then a slip in the second sentence, a pause, and
    # the second sentence again ("Then the rain. Puddles catch the la- ... Puddles catch the lamps, ...")
    src = takes["v2_12_rain_town"][1]
    write24(tmp, src)
    segs, _ = model.transcribe(str(tmp), language="en", word_timestamps=True, beam_size=5)
    words = [w for s in segs for w in s.words]
    tmp.unlink()
    k = next(j for j, w in enumerate(words) if w.word.strip().lower().startswith("puddles"))
    s2 = int(((words[k - 1].end + words[k].start) / 2) * SR)
    s1 = src[:s2]
    second = src[s2:]
    slip = second[:int(len(second) * 0.35)].copy()
    slip[-48:] *= np.linspace(1, 0, 48, dtype=np.float32)
    rain2 = np.concatenate([s1, slip, np.zeros(int(0.9 * SR), np.float32), second])
    print("rain restart:", " ".join(w.word.strip() for w in words), f"split at {s2 / SR:.2f} s")
    takes["v2_12_rain_town"] = [takes["v2_12_rain_town"][0], rain2]
    for numbered in (False, True):
        x, truth = build(lines, takes, shorter12, numbered, np.random.default_rng(11))
        tag = "numbered" if numbered else "plain"
        write24(HERE / f"session_{tag}.wav", x)
        (HERE / f"truth_{tag}.json").write_text(json.dumps(truth, indent=1), encoding="utf-8")
        print(f"{tag}: {len(x) / SR:.1f} s, {sum(1 for r in truth if r['kind'] == 'take')} takes, "
              f"peak {20 * np.log10(np.abs(x).max()):.1f} dBFS")


if __name__ == "__main__":
    main()
