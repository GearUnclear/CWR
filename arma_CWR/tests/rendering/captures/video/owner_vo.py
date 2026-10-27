"""The owner's own narration: from his Audacity recording to one finished WAV per line,
and the trailer's picture fitted to his voice ("the picture follows the voice").

    python owner_vo.py all     [--rec <wav|folder> ...] [--work <dir>] [--fav <file>] [--take 4=2 ...]
    python owner_vo.py finish  [--work <dir>] [--name <stem>] [--montage <dir>] [--final-dir <dir>]

The two commands above are the whole job when the recording arrives; the steps they run:

    split    transcribe the recording(s) (faster-whisper large-v3, word timestamps), find every
             take of every line by aligning the words to the script (main and shorter wordings),
             use spoken line numbers ("five", "line five") as anchors when they are there, and cut
             each take at the pauses around it (never inside a word). Writes <work>/session/:
             takes/*.wav (one candidate per take), takes.json, takes.txt (the report: line, take,
             where in the source, what was heard, word match, length, peak, clipping, wording).
    pick     the best take per line (all words, no clipping, clean start and end, the most
             natural pace, the main wording first); the owner's favourites win: --fav <file> with
             lines like "4 2" (line 4, take 2), "4 short" (his shorter wording) or --take 4=2.
             Writes <work>/picks.json, picks.txt.
    clean    light, natural cleaning, never pitch or tempo: high-pass 80 Hz, gentle noise
             reduction from a noise profile of the room tone (or the quietest pauses), de-ess,
             light compression; trimmed to 0.10 s before the first word and 0.25 s after the
             last, short fades. Writes <work>/lines/<id>.wav (48 kHz mono 24-bit, all 16 ids,
             what mix_vo.py --wav-dir reads) and lines/LINES.txt.
    fit      which lines fit their scene windows (mix_vo.py's rule: start 0.35 s in, end 0.6 s
             before the window ends; up to 0.5 s over is accepted) and, for those that do not,
             the picture-side remedy: a slightly shorter transition next to the scene when that
             is enough, else more clip frames (a longer reshoot of that clip on both sides, the
             same camera path at a slower pace). Never a faster voice. Writes <work>/fit/:
             FIT_PLAN.txt, timing_owner.json (for trailer_v2.py --timing) and timeline.json (the
             proposed plan).
    finish   mix_vo.py --wav-dir <work>/lines --fit, then verify, then preview.
    verify   ffprobe (60/1, the frame count of the timeline), the loudnorm print of the master,
             a whisper listen-check of the final mix line by line, check_vo.py on the lines.
    preview  a 720p60 copy of the subtitled master under 30 MB for the phone.

Interpreters: split and the listen-check need faster-whisper, which lives in the resolve-edit
venv (UD_WHISPER_PYTHON, used read-only); fit and finish need Pillow (plain "python").
Every subcommand runs from either: the script starts itself again with the other interpreter
when a module is missing. Whisper runs on CUDA when nvidia-smi reports 6 GB free, else on the
CPU (int8).
"""
import argparse
import difflib
import json
import math
import os
import re
import shutil
import statistics
import subprocess
import sys
import wave
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import udpaths  # noqa: E402  (where things live; see udpaths.py)
V2 = udpaths.WORK
OWNER = V2 / "vo_owner"
REC_DIR = V2 / "reading" / "recordings"
SHORTER_TXT = V2 / "reading" / "04_SHORTER_LINES_optional.txt"
LINES_JSON = V2 / "lines.json"
VENV_PY = udpaths.WHISPER_PY
FF = udpaths.FFMPEG
FFPROBE = udpaths.FFPROBE
SR = 48000
HOP = 480                  # 10 ms envelope frames
FPS = 60
LEAD, BREATH = 0.35, 0.6   # vo_v2.py / mix_vo.py placement rule
OVER_OK = 0.5              # a line may run this far past (window end - BREATH): about 0.5 s into the transition
HEAD, TAIL = 0.10, 0.25    # the finished line: this much before the first word and after the last
PRE_PAD, POST_PAD = 0.30, 0.50   # candidate cuts: at most this much of the pause around a take
LINE_LEVEL = -20.0         # active speech level the cleaner brings a take to before the compressor
# The calm transitions may be shortened only this far (seconds); each 0.1 s shorter gives both
# neighbouring scenes 0.05 s more window.
TRANS_FLOOR = {"fade": 0.8, "wipe_left": 0.8, "wipe_right": 0.8, "wipe_up": 0.8, "dip": 1.2, "flash": 0.6}
PROMPT = "Uslu dur, Arma: Cold War Assault. Malden, Everon, Sinai. Guerrillas, Guerrilla Mode, the AI."
# The split's hint is short on purpose: every chunk gets it, and a chunk with only a cough in it
# may come back as a copy of the hint, which must never look like half a line.
SPLIT_PROMPT = "Uslu dur. Malden, Everon, Sinai. Guerrillas."
TRANSCRIPT_VERSION = "chunks-1"
NUMBERS = ["one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve",
           "thirteen", "fourteen", "fifteen", "sixteen"]
NUM = {w: i + 1 for i, w in enumerate(NUMBERS)}
DIGITS = {str(i + 1): w for i, w in enumerate(NUMBERS)}
BRAND = "usludur"
HOMOPHONES = {"gorillas": "guerrillas", "gorilla": "guerrilla", "averon": "everon", "evron": "everon",
              "everyone": "everon", "sinaii": "sinai", "armour": "arma", "armor": "arma", "arm": "arma"}
# The optional shorter wordings (reading/04_SHORTER_LINES_optional.txt), by line number; the file
# is read when it exists, this is the fallback.
SHORTER = {
    3: "A resistance squad moves out at first light. The sun burns off the valley mist.",
    4: "Morning sun falls through the pines in shafts. The guerrillas keep to cover.",
    5: "On the bay, an occupier patrol boat crosses water that mirrors the hills and sky.",
    7: "Over Everon, the clouds catch the sun. Their shadows drift across the hills.",
    12: "At night, lamps and the fire light the occupied town. A patrol comes down the street.",
}


# ---------------------------------------------------------------- small helpers
def say(*a):
    print(*a, flush=True)


def have(mod):
    try:
        __import__(mod)
        return True
    except ImportError:
        return False


def rerun_with(py, argv):
    """Run this script again with another interpreter (the whisper venv or plain python)."""
    if os.environ.get("OWNER_VO_CHILD") == str(py):
        raise SystemExit(f"{py} lacks a module this step needs")
    env = dict(os.environ, OWNER_VO_CHILD=str(py))
    say(f"  (running with {py})")
    return subprocess.run([str(py), str(Path(__file__).resolve())] + [str(x) for x in argv], env=env).returncode


def in_venv():
    try:
        return Path(sys.executable).resolve() == VENV_PY.resolve()
    except OSError:
        return False


def whisper_elsewhere():
    """True when whisper should run in the resolve-edit venv rather than this interpreter."""
    return VENV_PY.exists() and not in_venv()


def plain_python():
    return shutil.which("python") or "python"


def load_audio(path, sr=SR):
    """Any audio file -> mono float32 at sr (ffmpeg)."""
    r = subprocess.run([FF, "-v", "error", "-i", str(path), "-f", "f32le", "-ac", "1", "-ar", str(sr), "-"],
                       capture_output=True, check=True)
    return np.frombuffer(r.stdout, np.float32).copy()


def probe_rate(path):
    r = subprocess.run([FFPROBE, "-v", "error", "-select_streams", "a:0", "-show_entries",
                        "stream=sample_rate,channels,bits_per_raw_sample,bits_per_sample,codec_name",
                        "-of", "json", str(path)], capture_output=True, text=True, check=True)
    s = json.loads(r.stdout)["streams"][0]
    return int(s["sample_rate"]), int(s.get("channels", 1)), s.get("codec_name", "?")


def write24(path, x, sr=SR):
    y = np.clip(np.asarray(x, np.float64), -1.0, 1.0 - 1.0 / 8388608)
    i = np.round(y * 8388607).astype("<i4")
    b = i.view(np.uint8).reshape(-1, 4)[:, :3].tobytes()
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(3)
        w.setframerate(sr)
        w.writeframes(b)


def wav_seconds(p):
    with wave.open(str(p), "rb") as w:
        return w.getnframes() / float(w.getframerate())


def db(v):
    return 20 * math.log10(max(float(v), 1e-9))


def natural_key(p):
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", str(p))]


def load_lines():
    """[(number 1..16, id, main text, shorter text or None)]"""
    lines = json.loads(LINES_JSON.read_text(encoding="utf-8"))
    short = dict(SHORTER)
    if SHORTER_TXT.exists():
        cur, parsed = None, {}
        for raw in SHORTER_TXT.read_text(encoding="utf-8").splitlines():
            m = re.match(r"^(\d{1,2})\s+\S", raw)
            if m:
                cur = int(m.group(1))
                continue
            m = re.match(r"^\s+shorter:\s*(.+)$", raw)
            if m and cur:
                parsed[cur] = m.group(1).strip()
        if parsed:
            short = parsed
    return [(i + 1, ln["id"], ln["text"], short.get(i + 1)) for i, ln in enumerate(lines)]


def fmt_t(s):
    return f"{int(s // 60)}:{s % 60:05.2f}"


# ---------------------------------------------------------------- words and tokens
def _pieces(text):
    t = text.lower().replace("\u2019", "'").replace("\u2018", "'")
    t = re.sub(r"[^a-z0-9' ]+", " ", t)
    return [p.strip("'") for p in t.split() if p.strip("'")]


def tokens(words):
    """whisper words [{w, s, e, p}] -> normalised tokens [{t, s, e, p}]: lower case, no punctuation,
    digits spelt out, the brand in any spelling ("Uslu dur", "Oslo door", "Usludur") as one token."""
    raw = []
    for w in words:
        ps = _pieces(w["w"])
        for k, p in enumerate(ps):
            d = (w["e"] - w["s"]) / len(ps)
            raw.append(dict(t=p, s=w["s"] + k * d, e=w["s"] + (k + 1) * d, p=w.get("p", 1.0)))
    out, i = [], 0
    while i < len(raw):
        w = raw[i]["t"]
        n1 = raw[i + 1]["t"] if i + 1 < len(raw) else ""
        n2 = raw[i + 2]["t"] if i + 2 < len(raw) else ""
        n = 0
        if re.fullmatch(r"(u|o)s(l|t)?(u|o)", w) and re.fullmatch(r"d(u|o)+(r|re|a)?", n1):
            n = 2
        elif re.fullmatch(r"(u|o)s", w) and re.fullmatch(r"l(u|o)", n1) and re.fullmatch(r"d(u|o)+r", n2):
            n = 3
        elif re.fullmatch(r"(u|o)sl?u?d(u|o)+r", w):
            n = 1
        if n:
            out.append(dict(t=BRAND, s=raw[i]["s"], e=raw[i + n - 1]["e"], p=min(r["p"] for r in raw[i:i + n])))
            i += n
            continue
        out.append(dict(raw[i], t=HOMOPHONES.get(w, DIGITS.get(w, w))))
        i += 1
    return out


def phantom(tk):
    """A word whisper wrote with an implausibly short time (it fills in "street" before "lamps" from
    its language prior): it still counts as heard, but a reading without it scores a little higher."""
    return len(tk["t"]) >= 4 and (tk["e"] - tk["s"]) < max(0.07, 0.025 * len(tk["t"]))


def ref_tokens(text):
    return [t["t"] for t in tokens([dict(w=text, s=0.0, e=0.0, p=1.0)])]


_SIM = {}


def sim(a, b):
    """Word similarity for the alignment: 2 the same word, 1.5 a near spelling, 0.5 a loose one, -1 none."""
    k = (a, b)
    if k in _SIM:
        return _SIM[k]
    if a == b:
        v = 2.0
    elif a == BRAND and re.match(r"^[uo]s", b):
        v = 1.5
    else:
        r = difflib.SequenceMatcher(None, a, b).ratio()
        v = 1.5 if r >= 0.8 else (0.5 if r >= 0.67 and min(len(a), len(b)) >= 4 else -1.0)
    _SIM[k] = v
    return v


def align(R, T, ph=None):
    """Fitting alignment of the reference words R into the heard tokens T (free start and end in T).
    H[i][j]: best score of R[:i] ending at T[:j]; moves 0 = word matched, 1 = reference word missing,
    2 = an extra heard word. A tie prefers the match, so a restart's earlier, broken attempt ends up
    as the extra words."""
    m, n = len(R), len(T)
    S = [[sim(r, t) for t in T] for r in R]
    M = [[v > 0 for v in row] for row in S]          # a match, whatever its score
    if ph:
        for i in range(m):
            for j in range(n):
                if ph[j] and S[i][j] > 0:
                    S[i][j] = -0.25
    ins = [0.0 if (ph and ph[j]) else 1.0 for j in range(n)]
    H = [[0.0] * (n + 1) for _ in range(m + 1)]
    P = [[0] * (n + 1) for _ in range(m + 1)]
    for i in range(1, m + 1):
        Hi, Hp, Si, Pi = H[i], H[i - 1], S[i - 1], P[i]
        Hi[0] = -1.0 * i
        Pi[0] = 1
        last = i == m          # once the whole line is matched an extra word can only end it, not extend it
        for j in range(1, n + 1):
            d = Hp[j - 1] + Si[j - 1]
            u = Hp[j] - 1.0
            le = -1e9 if last else Hi[j - 1] - ins[j - 1]
            if d >= u and d >= le:
                Hi[j], Pi[j] = d, 0
            elif u >= le:
                Hi[j], Pi[j] = u, 1
            else:
                Hi[j], Pi[j] = le, 2
    return H, P, S, M


def trace(P, m, j):
    i, pairs = m, []
    while i > 0:
        mv = P[i][j]
        if mv == 0:
            pairs.append((i - 1, j - 1))
            i, j = i - 1, j - 1
        elif mv == 1:
            pairs.append((i - 1, None))
            i -= 1
        else:
            pairs.append((None, j - 1))
            j -= 1
    pairs.reverse()
    return pairs


def find_hits(variants, T):
    """Every place a line (or its shorter wording) is read in T: [{line, id, wording, j0, j1, score, ...}].
    Per wording: the best reading in a stretch of tokens is taken, then the stretches before and after it
    are searched again, so a complete take never hides a broken one next to it."""
    hits = []
    words = [t["t"] for t in T]
    ph = [phantom(t) for t in T]
    for (num, rid, wording, text) in variants:
        R = ref_tokens(text)
        m = len(R)
        todo = [(0, len(words))]
        while todo:
            lo, hi = todo.pop()
            if hi - lo < 2:
                continue
            H, P, S, M = align(R, words[lo:hi], ph[lo:hi])
            row = H[m]
            j = max(range(1, hi - lo + 1), key=lambda k: row[k])
            if row[j] <= 0:
                continue
            pairs = trace(P, m, j)
            # a take starts and ends on a heard word that matches: a mismatch at either end (often the
            # last word of the take before) counts as a missing reference word instead
            for rng in (range(len(pairs)), range(len(pairs) - 1, -1, -1)):
                for k in rng:
                    a_, b_ = pairs[k]
                    if b_ is None:
                        continue
                    if a_ is not None and M[a_][b_]:
                        break
                    pairs[k] = (a_, None) if a_ is not None else (None, None)
            pairs = [pr for pr in pairs if pr != (None, None)]
            idx = [b_ for a_, b_ in pairs if b_ is not None]
            if not idx:
                continue
            matched = sum(1 for a_, b_ in pairs if a_ is not None and b_ is not None and M[a_][b_])
            if matched < max(2, 0.5 * m):
                continue
            missing = [R[a_] for a_, b_ in pairs if a_ is not None and (b_ is None or not M[a_][b_])]
            extra = [lo + b_ for a_, b_ in pairs if a_ is None]
            score = (sum(S[a_][b_] if M[a_][b_] else -1.0 for a_, b_ in pairs if a_ is not None and b_ is not None)
                     - sum(1 for a_, b_ in pairs if b_ is None) - sum(0.0 if ph[lo + b_] else 1.0 for a_, b_ in pairs if a_ is None))
            lead = 0
            for a_, b_ in pairs:
                if b_ is not None:
                    break
                lead += 1
            trail = 0
            for a_, b_ in reversed(pairs):
                if b_ is not None:
                    break
                trail += 1
            j0, j1 = lo + min(idx), lo + max(idx)
            hits.append(dict(line=num, id=rid, wording=wording, j0=j0, j1=j1, score=score,
                             matched=matched, ref_len=m, missing=missing, extra=extra, lead=lead, trail=trail))
            todo += [(lo, j0), (j1 + 1, hi)]
    return hits


def select(hits, bonus=None):
    """Weighted interval scheduling: the set of non-overlapping hits with the best total score."""
    hs = sorted(hits, key=lambda h: (h["j1"], h["j0"]))
    w = [h["score"] + (bonus(h) if bonus else 0.0) for h in hs]
    ends = [h["j1"] for h in hs]
    import bisect
    best = [0.0] * (len(hs) + 1)
    take = [False] * (len(hs) + 1)
    prev = []
    for k, h in enumerate(hs):
        p = bisect.bisect_left(ends, h["j0"], 0, k)   # hits ending before this one starts
        prev.append(p)
        a = best[k]
        b = w[k] + best[p]
        best[k + 1], take[k + 1] = (b, True) if b > a else (a, False)
    out, k = [], len(hs)
    while k > 0:
        if take[k]:
            out.append(hs[k - 1])
            k = prev[k - 1]
        else:
            k -= 1
    return sorted(out, key=lambda h: h["j0"])


def number_anchors(T, chosen):
    """Spoken line numbers outside the takes: [(token index, line number, said)]."""
    covered = set()
    for h in chosen:
        covered.update(range(h["j0"], h["j1"] + 1))
    out = []
    for i, tk in enumerate(T):
        if i in covered or tk["t"] not in NUM:
            continue
        prev = T[i - 1]["t"] if i else ""
        if prev in ("take", "part", "times"):
            continue
        pre = prev in ("line", "number", "no") and (i - 1) not in covered
        said = (prev + " " if pre else "") + tk["t"]
        out.append((i, NUM[tk["t"]], said, i - 1 if pre else i))
    return out


# ---------------------------------------------------------------- the energy map
def _runs(mask):
    d = np.diff(np.concatenate([[0], mask.astype(np.int8), [0]]))
    return list(zip(np.where(d == 1)[0].tolist(), np.where(d == -1)[0].tolist()))


def _fill(v, val, maxlen):
    v = v.copy()
    for s, e in _runs(v == val):
        if s > 0 and e < len(v) and e - s < maxlen:
            v[s:e] = not val
    return v


class Env:
    """10 ms energy frames of a recording, its noise floor and a speech/pause map."""

    def __init__(self, x):
        n = len(x) // HOP
        e = (x[:n * HOP].astype(np.float64).reshape(n, HOP) ** 2).mean(1)
        e = np.convolve(e, [0.5, 0.5])[:n]
        self.db = 10 * np.log10(e + 1e-12)
        self.floor = float(np.percentile(self.db, 5))
        self.top = float(np.percentile(self.db, 97))
        self.thr = self.floor + float(np.clip(0.3 * (self.top - self.floor), 6.0, 15.0))
        v = self.db > self.thr
        v = _fill(v, False, 6)     # pauses under 60 ms (stop closures) count as speech
        v = _fill(v, True, 3)      # blips under 30 ms count as pause
        self.v = v
        self.n = n
        self.sil = _runs(~v)
        self.voiced = _runs(v)

    def pause_before(self, t_word, t_prev):
        """(onset, pause length) for a word starting near t_word: the end of the last pause of 0.1 s
        or more ending between t_prev - 0.3 and t_word + 0.4; with none (the speech runs straight
        on from the word before) the quietest 10 ms between them and 0."""
        fa = int(max(0.0, t_prev - 0.3) * 100)
        fb = int((t_word + 0.4) * 100)
        best = None
        for s, e in self.sil:
            if e - s < 10 or e < fa or e > fb:
                continue
            if best is None or e > best[1]:
                best = (s, e)
        if best:
            return best[1] / 100.0, (best[1] - best[0]) / 100.0
        lo = max(int(t_prev * 100), int(t_word * 100) - 60)
        hi = max(lo + 1, int(t_word * 100) + 1)
        k = lo + int(np.argmin(self.db[lo:hi]))
        return k / 100.0, 0.0

    def pause_after(self, t_last, t_next):
        """(offset, pause length) after a word ending near t_last: the start of the first pause of
        0.2 s or more starting from t_last - 0.12 on (before the next word); else the quietest 10 ms."""
        fa = int(max(0.0, t_last - 0.12) * 100)
        fb = int((t_next + 0.25) * 100) if t_next is not None else self.n
        for s, e in self.sil:
            if s < fa or s > fb:
                continue
            if e - s >= 20 or e >= self.n:
                return s / 100.0, (e - s) / 100.0
        lo = int(t_last * 100)
        hi = max(lo + 1, int((t_next if t_next is not None else t_last + 0.3) * 100))
        k = lo + int(np.argmin(self.db[lo:hi]))
        return k / 100.0, 0.0

    def adjacent_pause(self, f, before, min_len=0.15):
        """The pause that ends (before=True) or starts at frame f, if it is min_len or longer: (s0, s1) seconds."""
        for s, e in self.sil:
            if (e == f if before else s == f) and e - s >= min_len * 100:
                return s / 100.0, e / 100.0
        return None

    def gap_pause(self, t0, t1, min_len=0.15):
        """The longest pause of min_len or more between t0 and t1 (frames overlapping), or None."""
        fa, fb = int(t0 * 100), int(t1 * 100)
        best = None
        for s, e in self.sil:
            if e <= fa or s >= fb or (e - s) < min_len * 100:
                continue
            if best is None or (e - s) > (best[1] - best[0]):
                best = (s, e)
        return None if best is None else (best[0] / 100.0, best[1] / 100.0)


# ---------------------------------------------------------------- split
def snap(words, env):
    """Whisper's word times can sit a few tenths early or late in a whole chunk; shift each chunk's
    words (up to 0.45 s) so they cover the most speech in the energy map."""
    cv = np.concatenate([[0], np.cumsum(env.v.astype(np.int32))])
    n = len(env.v)

    def voiced(s, e):
        a, b = max(0, min(n, int(s * 100))), max(0, min(n, int(math.ceil(e * 100))))
        return (cv[b] - cv[a]) / max(1, b - a)
    by = {}
    for w in words:
        by.setdefault(w.get("c", -1), []).append(w)
    out, shifts = [], []
    for c, ws in by.items():
        if c == -1 or not ws:
            out += ws
            continue
        def score(d):
            return float(np.mean([voiced(w["s"] + d, w["e"] + d) for w in ws]))
        base = score(0.0)
        best = max((score(d / 100.0) - 0.05 * abs(d / 100.0), d / 100.0) for d in range(-45, 46))
        d = best[1] if best[0] > base + 0.02 else 0.0
        if d:
            shifts.append(d)
        out += [dict(w, s=round(w["s"] + d, 3), e=round(w["e"] + d, 3)) for w in ws]
    out.sort(key=lambda w: w["s"])
    return out, shifts


def pick_device(want):
    if want in ("cuda", "cpu"):
        return want
    try:
        r = subprocess.run(["nvidia-smi", "--query-gpu=memory.free", "--format=csv,noheader,nounits"],
                           capture_output=True, text=True, timeout=30)
        free = max(int(x) for x in r.stdout.split())
        say(f"  GPU: {free} MiB free")
        return "cuda" if free >= 6144 else "cpu"
    except Exception:
        return "cpu"


def speech_chunks(env, secs, min_gap=0.6, pad=0.25, max_len=28.0):
    """[(start, end)] seconds: the speech cut at every pause of min_gap or more. Whisper hears each
    chunk on its own; given one 30 s window with the same sentence read three times it writes it
    once and drifts its word times, so takes must never share a window."""
    runs = []
    for s, e in env.voiced:
        if runs and s - runs[-1][1] < min_gap * 100:
            runs[-1][1] = e
        else:
            runs.append([s, e])
    runs = [r for r in runs if r[1] - r[0] >= 20]

    def split_long(a, b):
        if (b - a) / 100.0 <= max_len:
            return [(a, b)]
        inner = [(s, e) for s, e in env.sil if s > a and e < b]
        if not inner:
            mid = (a + b) // 2
            return split_long(a, mid) + split_long(mid, b)
        s, e = max(inner, key=lambda r: r[1] - r[0])
        return split_long(a, s) + split_long(e, b)
    flat = [c for r in runs for c in split_long(r[0], r[1])]
    out = []
    for k, (a, b) in enumerate(flat):
        lo = flat[k - 1][1] if k else 0
        hi = flat[k + 1][0] if k + 1 < len(flat) else int(secs * 100)
        out.append((max(0.0, a / 100.0 - min(pad, (a - lo) / 200.0)), min(secs, b / 100.0 + min(pad, (hi - b) / 200.0))))
    return out


def transcribe_all(files, cache_dir, device, retranscribe):
    """{file: [words]} with faster-whisper large-v3, word timestamps, one pause-separated chunk at a
    time; cached per file size and time."""
    out, todo = {}, []
    for f in files:
        st = f.stat()
        c = cache_dir / f"transcript_{f.stem}.json"
        if c.exists() and not retranscribe:
            j = json.loads(c.read_text(encoding="utf-8"))
            if (j.get("size") == st.st_size and j.get("mtime") == int(st.st_mtime)
                    and j.get("version") == TRANSCRIPT_VERSION):
                out[f] = j["words"]
                say(f"  transcript cached: {f.name} ({len(j['words'])} words)")
                continue
        todo.append(f)
    if not todo:
        return out
    from faster_whisper import WhisperModel
    dev = pick_device(device)
    try:
        model = WhisperModel("large-v3", device=dev, compute_type="float16" if dev == "cuda" else "int8",
                             local_files_only=True)
    except Exception as e:
        say(f"  whisper on {dev} failed ({e}); using the CPU")
        dev = "cpu"
        model = WhisperModel("large-v3", device="cpu", compute_type="int8", local_files_only=True)
    say(f"  whisper large-v3 on {dev}")
    for f in todo:
        x = load_audio(f)
        env = Env(x)
        chunks = speech_chunks(env, len(x) / SR)
        a16 = load_audio(f, 16000)
        words = []
        for k, (c0, c1) in enumerate(chunks):
            segs, _ = model.transcribe(a16[int(c0 * 16000):int(c1 * 16000)], language="en", beam_size=5,
                                       word_timestamps=True, vad_filter=False, condition_on_previous_text=False,
                                       initial_prompt=SPLIT_PROMPT)
            for s in segs:
                for w in (s.words or []):
                    words.append(dict(w=w.word, s=round(c0 + float(w.start), 3), e=round(c0 + float(w.end), 3),
                                      p=round(float(w.probability), 3), c=k))
            if k % 20 == 19:
                say(f"    {k + 1}/{len(chunks)} chunks")
        st = f.stat()
        (cache_dir / f"transcript_{f.stem}.json").write_text(
            json.dumps(dict(source=str(f), size=st.st_size, mtime=int(st.st_mtime), version=TRANSCRIPT_VERSION,
                            chunks=[[round(a, 3), round(b, 3)] for a, b in chunks], words=words), indent=0),
            encoding="utf-8")
        say(f"  transcribed {f.name}: {len(x) / SR:.1f} s in {len(chunks)} chunks, {len(words)} words")
        out[f] = words
    return out


def find_recordings(recs):
    files = []
    for r in recs:
        p = Path(r)
        if p.is_dir():
            files += [q for q in p.iterdir() if q.suffix.lower() in (".wav", ".flac", ".aif", ".aiff", ".mp3", ".m4a")]
        elif p.exists():
            files.append(p)
        else:
            raise SystemExit(f"no such recording: {p}")
    files = sorted(set(files), key=natural_key)
    if not files:
        raise SystemExit(f"no recordings found in {recs}")
    return files


def noise_sample(x, env):
    """(samples, where): the room tone at the start (5 s asked for) or, without one, the quietest pauses."""
    first = env.voiced[0][0] / 100.0 if env.voiced else len(x) / SR
    if first >= 2.0:
        a, b = 0.25, min(first - 0.25, 4.25)
        return x[int(a * SR):int(b * SR)], f"room tone {a:.2f}-{b:.2f} s"
    parts = []
    for s, e in sorted(env.sil, key=lambda r: float(np.mean(env.db[r[0]:r[1]])) if r[1] > r[0] else 0):
        if e - s < 30:
            continue
        parts.append(x[(s + 8) * HOP:(e - 8) * HOP])
        if sum(len(p) for p in parts) >= 3 * SR:
            break
    if not parts:
        return np.zeros(SR // 2, np.float32), "none found (silence used)"
    return np.concatenate(parts)[:4 * SR], "the quietest pauses"


def cut_take(x, env, T, h, file_end):
    """Cut points and splices of one hit; returns a dict of times (s, in the source)."""
    t_first, t_last = T[h["j0"]]["s"], T[h["j1"]]["e"]
    t_prev = T[h["j0"] - 1]["e"] if h["j0"] > 0 else 0.0
    t_next = T[h["j1"] + 1]["s"] if h["j1"] + 1 < len(T) else None
    notes, taken = [], 0
    # words of the line whisper did not hear at its start ("Uslu dur." before a pause) or end: take in
    # the unheard speech right next to the take (pauses under 1.2 s, about 0.6 s of speech a word)
    if h.get("lead"):
        budget, cur = 0.6 * h["lead"] + 0.3, t_first
        for s, e in reversed(env.voiced):
            ts, te = s / 100.0, e / 100.0
            if te > cur - 0.05:
                continue
            if cur - te > 1.2 or ts < t_prev + 0.15 or te - ts > budget:
                break
            budget -= te - ts
            cur = ts
        if cur < t_first - 0.05:
            taken += h["lead"]
            notes.append(f"took in {t_first - cur:.2f} s of unheard speech before the first heard word "
                         f"(the missing \"{' '.join(h['missing'][:h['lead']])}\")")
            t_first = cur
    if h.get("trail"):
        budget, cur = 0.6 * h["trail"] + 0.3, t_last
        lim = t_next - 0.15 if t_next is not None else len(env.v) / 100.0
        for s, e in env.voiced:
            ts, te = s / 100.0, e / 100.0
            if ts < cur + 0.05:
                continue
            if ts - cur > 1.2 or te > lim or te - ts > budget:
                break
            budget -= te - ts
            cur = te
        if cur > t_last + 0.05:
            taken += h["trail"]
            notes.append(f"took in {cur - t_last:.2f} s of unheard speech after the last heard word")
            t_last = cur
    on, pre = env.pause_before(t_first, t_prev)
    off, post = env.pause_after(t_last, t_next)
    if off <= on:
        off = max(t_last, on + 0.2)
    start = max(0.0, on - min(PRE_PAD, 0.5 * pre))
    end = min(file_end, off + min(POST_PAD, 0.5 * post))
    # extra words inside the take (a restart): cut them out when pauses sit on both sides
    splices = []
    ext = sorted(h["extra"])
    groups = []
    for j in ext:
        if groups and j == groups[-1][-1] + 1:
            groups[-1].append(j)
        else:
            groups.append([j])
    for g in groups:
        r0, r1 = g[0], g[-1]
        dur = T[r1]["e"] - T[r0]["s"]
        heard = " ".join(T[k]["t"] for k in g)
        if len(g) < 2 and dur < 0.25:
            notes.append(f"extra word \"{heard}\"")
            continue
        b = env.gap_pause(T[r0 - 1]["e"] - 0.1, T[r0]["s"] + 0.2) if r0 > h["j0"] else None
        a = env.gap_pause(T[r1]["e"] - 0.2, T[r1 + 1]["s"] + 0.1) if r1 < h["j1"] else None
        if b and a:
            s0 = b[0] + min(0.30, 0.6 * (b[1] - b[0]))
            s1 = a[1] - min(0.12, 0.5 * (a[1] - a[0]))
            if s1 > s0:
                splices.append([round(s0, 3), round(s1, 3)])
                notes.append(f"restart cut out: \"{heard}\" ({s1 - s0:.2f} s)")
                continue
        notes.append(f"extra words not cut (no pauses around them): \"{heard}\"")
    # a loud sound inside the take that whisper did not hear as a word (a cough, a false start)
    spans = [(T[k]["s"] - 0.15, T[k]["e"] + 0.15) for k in range(h["j0"], h["j1"] + 1)]
    spans += [(t_first - 0.05, T[h["j0"]]["s"]), (T[h["j1"]]["e"], t_last + 0.05)]   # unheard words taken in
    sp_db = env.db[int(on * 100):max(int(on * 100) + 1, int(off * 100))]
    loud = float(np.percentile(sp_db, 90)) if len(sp_db) else env.top
    noise_inside = []
    for s, e in env.voiced:
        ts, te = s / 100.0, e / 100.0
        if ts < on or te > off or te - ts < 0.15:
            continue
        if any(ts < b1 and te > b0 for b0, b1 in spans):
            continue
        if any(ts >= sp[0] and te <= sp[1] for sp in splices):
            continue
        if float(env.db[s:e].max()) < loud - 15:
            continue
        b = env.adjacent_pause(s, before=True)
        a = env.adjacent_pause(e, before=False)
        if b and a:
            s0 = b[0] + min(0.30, 0.6 * (b[1] - b[0]))
            s1 = a[1] - min(0.12, 0.5 * (a[1] - a[0]))
            splices.append([round(s0, 3), round(s1, 3)])
            notes.append(f"an unheard sound cut out at {ts:.2f} s ({te - ts:.2f} s; a cough or a false start)")
        else:
            noise_inside.append([round(ts, 2), round(te, 2)])
            notes.append(f"a sound inside the take at {ts:.2f} s that is not a word")
    merged = []
    for sp in sorted(splices):
        if merged and sp[0] <= merged[-1][1]:
            merged[-1][1] = max(merged[-1][1], sp[1])
        else:
            merged.append(list(sp))
    splices = [sp for sp in merged if on < sp[0] and sp[1] < off]
    return dict(start=round(start, 3), end=round(end, 3), onset=round(on, 3), offset=round(off, 3),
                pre=round(pre, 3), post=round(post, 3), splices=splices, noise_inside=noise_inside, notes=notes,
                unheard_taken_in=taken)


def assemble(x, c):
    """The candidate audio: start..end with the splices removed (10 ms crossfades); onset/offset in it."""
    keep, t = [], c["start"]
    for s0, s1 in c["splices"]:
        keep.append((t, s0))
        t = s1
    keep.append((t, c["end"]))
    pieces = [x[int(a * SR):int(b * SR)].copy() for a, b in keep if b > a]
    fade = int(0.01 * SR)
    out = pieces[0]
    overlap = [0]                       # samples lost to crossfades before each piece
    for p in pieces[1:]:
        if len(out) > fade and len(p) > fade:
            ramp = np.linspace(0, 1, fade, dtype=np.float32)
            mid = out[-fade:] * (1 - ramp) + p[:fade] * ramp
            out = np.concatenate([out[:-fade], mid, p[fade:]])
            overlap.append(overlap[-1] + fade)
        else:
            out = np.concatenate([out, p])
            overlap.append(overlap[-1])
    spans = [(a, b) for a, b in keep if b > a]

    def mapped(t):
        acc = 0.0
        for k, (a, b) in enumerate(spans):
            if t <= b or k == len(spans) - 1:
                return acc + max(0.0, min(t, b) - a) - overlap[k] / SR
            acc += b - a
        return acc
    cut = sum(s1 - s0 for s0, s1 in c["splices"])
    return out, mapped(c["onset"]), mapped(c["offset"]), cut


def peak_and_clip(xn, sr, a, b):
    seg = xn[int(a * sr):int(b * sr)]
    if not len(seg):
        return -120.0, 0
    pk = float(np.abs(seg).max())
    hot = np.abs(seg) >= 0.9995
    runs = [r for r in _runs(hot) if r[1] - r[0] >= 3]
    return round(db(pk), 2), len(runs)


def cmd_split(a):
    if whisper_elsewhere() or not have("faster_whisper"):
        return rerun_with(VENV_PY, ["split"] + split_args(a))
    work = Path(a.work)
    sess = work / "session"
    (sess / "takes").mkdir(parents=True, exist_ok=True)
    for old in (sess / "takes").glob("*.wav"):
        old.unlink()
    files = find_recordings(a.rec or [str(REC_DIR)])
    say(f"split: {len(files)} recording(s): " + ", ".join(f.name for f in files))
    lines = load_lines()
    variants = [(n, rid, "main", text) for n, rid, text, _ in lines]
    variants += [(n, rid, "shorter", short) for n, rid, _, short in lines if short]
    words = transcribe_all(files, sess, a.device, a.retranscribe)
    report = dict(sources=[], numbers=[], takes=[], unassigned=[])
    all_takes = []
    for fi, f in enumerate(files):
        sr, ch, codec = probe_rate(f)
        x = load_audio(f)
        xn = x if sr == SR else load_audio(f, sr)
        env = Env(x)
        snapped, shifts = snap(words[f], env)
        if shifts:
            say(f"  {f.name}: word times of {len(shifts)} chunk(s) shifted to the speech "
                f"({', '.join(f'{d:+.2f}' for d in shifts)} s)")
        T = tokens(snapped)
        hits = find_hits(variants, T)
        chosen = select(hits)
        anchors = number_anchors(T, chosen)
        m = re.search(r"(?<!\d)(\d{1,2})(?!\d)", f.stem)
        file_line = int(m.group(1)) if (m and 1 <= int(m.group(1)) <= 16 and len(files) > 1) else None

        def announced(h, anc):
            k = [n for (i, n, _s, _i0) in anc if i < h["j0"]]
            return k[-1] if k else file_line

        if anchors or file_line:
            first = anchors
            chosen = select(hits, bonus=lambda h: 3.0 if announced(h, first) == h["line"] else 0.0)
            anchors = number_anchors(T, chosen)
        nz, where = noise_sample(x, env)
        nz_path = sess / f"noise_{f.stem}.wav"
        write24(nz_path, nz)
        nz_db = 10 * math.log10(float(np.mean(nz.astype(np.float64) ** 2)) + 1e-12) if len(nz) else -120
        report["sources"].append(dict(file=str(f), rate=sr, channels=ch, codec=codec, seconds=round(len(x) / SR, 2),
                                      floor_db=round(env.floor, 1), speech_db=round(env.top, 1),
                                      threshold_db=round(env.thr, 1), noise=str(nz_path), noise_from=where,
                                      noise_db=round(nz_db, 1)))
        for (i, n, said, i0) in anchors:
            report["numbers"].append(dict(file=f.name, t=round(T[i0]["s"], 2), said=said, line=n))
        if ch > 1:
            say(f"  note: {f.name} has {ch} channels; mixed down to mono")
        file_end = len(x) / SR
        used = []
        for h in chosen:
            c = cut_take(x, env, T, h, file_end)
            ratio = h["matched"] / h["ref_len"]
            kind = "take" if ratio >= 0.9 else ("incomplete" if ratio >= 0.75 else "fragment")
            heard = " ".join(T[k]["t"] for k in range(h["j0"], h["j1"] + 1))
            pk, clips = peak_and_clip(xn, sr, c["onset"], c["offset"])
            nwords = h["ref_len"] - len(h["missing"])
            speech = c["offset"] - c["onset"] - sum(s1 - s0 for s0, s1 in c["splices"])
            prob = float(np.mean([T[k]["p"] for k in range(h["j0"], h["j1"] + 1)]))
            ann = announced(h, anchors)
            all_takes.append(dict(line=h["line"], id=h["id"], wording=h["wording"], kind=kind, file=str(f),
                                  file_index=fi, **c, heard=heard, match=round(ratio, 3),
                                  missing=h["missing"], extra=[T[k]["t"] for k in h["extra"]],
                                  peak_db=pk, clipped=clips > 0, clip_runs=clips,
                                  speech_secs=round(speech, 3), wps=round(nwords / max(0.5, speech), 3),
                                  prob=round(prob, 3), announced=ann, score=round(h["score"], 1)))
            used.append((c["start"], c["end"]))
        for (i, n, said, i0) in anchors:
            used.append((T[i0]["s"] - 0.1, T[i]["e"] + 0.1))
        for s, e in env.voiced:
            ts, te = s / 100.0, e / 100.0
            if te - ts < 0.15 or any(ts < b and te > a_ for a_, b in used):
                continue
            txt = " ".join(t["t"] for t in T if t["e"] > ts and t["s"] < te)
            report["unassigned"].append(dict(file=f.name, start=round(ts, 2), end=round(te, 2),
                                             heard=txt or "(no words: a noise, a cough or a breath)"))
    # number the takes per line in recording order; fragments get x1, x2 ...
    by_line = {}
    for t in sorted(all_takes, key=lambda t: (t["file_index"], t["start"])):
        by_line.setdefault(t["line"], []).append(t)
    for n, ts in by_line.items():
        k = f = 0
        for t in ts:
            if t["kind"] == "fragment":
                f += 1
                t["take"] = f"x{f}"
            else:
                k += 1
                t["take"] = k
            t["label"] = f"{n}.{t['take']}"
    cache = {}
    for t in all_takes:
        if t["kind"] == "fragment":
            t["wav"] = None
            continue
        f = Path(t["file"])
        if f not in cache:
            cache = {f: load_audio(f)}
        y, on, off, cut = assemble(cache[f], t)
        suffix = "_short" if t["wording"] == "shorter" else ""
        name = f"L{t['line']:02d}_{t['id']}__t{t['take']}{suffix}.wav"
        write24(sess / "takes" / name, y)
        t.update(wav=str(sess / "takes" / name), onset_in_wav=round(on, 3), offset_in_wav=round(off, 3),
                 wav_secs=round(len(y) / SR, 3), finished_secs=round(off - on + HEAD + TAIL, 3))
    report["takes"] = sorted(all_takes, key=lambda t: (t["line"], t["file_index"], t["start"]))
    (sess / "takes.json").write_text(json.dumps(report, indent=1), encoding="utf-8")
    txt = takes_text(report, lines)
    (sess / "takes.txt").write_text(txt, encoding="utf-8")
    say(txt)
    say(f"-> {sess / 'takes.txt'}\n-> {sess / 'takes.json'}\n-> {sess / 'takes'}")
    return 0


def takes_text(rep, lines):
    out = ["TAKES FOUND IN THE RECORDING", "============================", ""]
    for s in rep["sources"]:
        out.append(f"source {Path(s['file']).name}: {s['seconds']:.1f} s, {s['rate']} Hz, {s['channels']} ch; "
                   f"floor {s['floor_db']} dBFS, speech {s['speech_db']} dBFS; noise profile from {s['noise_from']}")
    if rep["numbers"]:
        out.append("spoken line numbers: " + ", ".join(f"\"{n['said']}\" at {n['t']:.1f} s" for n in rep["numbers"]))
    else:
        out.append("spoken line numbers: none found (the takes were found by their words alone)")
    out.append("")
    out.append(f"{'take':6s} {'wording':8s} {'start':>8s} {'end':>8s} {'len':>5s} {'match':>5s} {'peak':>6s} "
               f"{'clip':4s} {'wps':>4s}  heard / notes")
    by = {}
    for t in rep["takes"]:
        by.setdefault(t["line"], []).append(t)
    for n, rid, text, short in lines:
        out.append("")
        out.append(f"line {n} ({rid}): \"{text}\"")
        ts = by.get(n, [])
        if not ts:
            out.append("  NOT FOUND in the recording")
            continue
        for t in ts:
            ln = t.get("finished_secs") or round(t["offset"] - t["onset"], 2)
            flag = []
            if t["kind"] != "take":
                flag.append(t["kind"].upper())
            if t["missing"]:
                flag.append("missing: " + " ".join(t["missing"]))
            if t["announced"] and t["announced"] != n:
                flag.append(f"announced as {t['announced']}")
            flag += t["notes"]
            out.append(f"  {t['label']:5s} {t['wording']:8s} {fmt_t(t['start']):>8s} {fmt_t(t['end']):>8s} "
                       f"{ln:5.2f} {100 * t['match']:4.0f}% {t['peak_db']:6.1f} {'YES' if t['clipped'] else 'no':4s} "
                       f"{t['wps']:4.1f}  {t['heard'][:70]}")
            if flag:
                out.append("        " + "; ".join(flag))
    if rep["unassigned"]:
        out.append("")
        out.append("sounds that belong to no take:")
        for u in rep["unassigned"]:
            out.append(f"  {u['file']} {fmt_t(u['start'])}-{fmt_t(u['end'])}: {u['heard'][:80]}")
    out.append("")
    out.append("len = the finished line (0.10 s before the first word, 0.25 s after the last); peak in dBFS; "
               "wps = words a second.")
    return "\n".join(out) + "\n"


# ---------------------------------------------------------------- pick
def parse_favs(text, lines):
    ids = {rid: n for n, rid, _, _ in lines}
    out = {}
    for raw in text.splitlines():
        s = raw.split("#")[0].strip().lower()
        if not s:
            continue
        m = re.match(r"^(?:line\s*)?(\d{1,2}|v2_\d\d_[a-z_]+)\s*(?:[:=,.]\s*|\s+)(?:take\s*)?(\d+|short(?:er)?|main)$", s)
        if not m:
            say(f"  favourites: cannot read \"{raw.strip()}\" (write it like \"4 2\" or \"4 short\")")
            continue
        n = int(m.group(1)) if m.group(1).isdigit() else ids.get(m.group(1))
        if not n or n < 1 or n > 16:
            say(f"  favourites: no line {m.group(1)}")
            continue
        v = m.group(2)
        out[n] = int(v) if v.isdigit() else ("shorter" if v.startswith("short") else "main")
    return out


def take_score(t, med, room):
    s, why = 0.0, []
    if t["kind"] != "take":
        s -= 100
        why.append("incomplete")
    if t["missing"]:
        guessed = min(len(t["missing"]), t.get("unheard_taken_in", 0))
        s -= 30 * (len(t["missing"]) - guessed) + 5 * guessed
        why.append(f"{len(t['missing'])} word(s) not heard" + (f" ({guessed} taken in from unheard speech)" if guessed else ""))
    if t["clipped"]:
        s -= 60
        why.append("clipped")
    if t["pre"] < 0.15:
        s -= 4
        why.append("no clean start")
    if t["post"] < 0.25:
        s -= 4
        why.append("no clean end")
    if t["noise_inside"]:
        s -= 10
        why.append("a noise inside")
    if t["splices"]:
        s -= 8
        why.append("repaired restart")
    if t["extra"] and not t["splices"]:
        s -= 3 * len(t["extra"])
    if t["wording"] == "shorter":
        s -= 6
    pace = abs(math.log(max(0.3, t["wps"]) / med))
    s -= 20 * pace
    s += 10 * (t["prob"] - 0.9)
    if t["peak_db"] < -30:
        s -= 5
        why.append("very quiet")
    if room is not None and t.get("finished_secs", 0) > room + OVER_OK:
        s -= 3
        why.append("longer than its scene")
    return s, why


def windows_now(montage):
    tl = Path(montage) / "timeline.json"
    if not tl.exists():
        return {}
    j = json.loads(tl.read_text(encoding="utf-8"))
    return {k: tuple(v) for k, v in j.get("windows", {}).items()}


def cmd_pick(a):
    work = Path(a.work)
    rep = json.loads((work / "session" / "takes.json").read_text(encoding="utf-8"))
    lines = load_lines()
    favs = {}
    fav_file = Path(a.fav) if a.fav else work / "favourites.txt"
    if fav_file.exists():
        favs.update(parse_favs(fav_file.read_text(encoding="utf-8"), lines))
        say(f"pick: favourites from {fav_file}: {favs}")
    for t in a.take or []:
        favs.update(parse_favs(t.replace("=", " "), lines))
    wins = windows_now(a.montage)
    usable = [t for t in rep["takes"] if t["kind"] != "fragment" and t.get("wav")]
    rates = [t["wps"] for t in usable if t["kind"] == "take" and not t["missing"]] or [2.4]
    med = statistics.median(rates)
    picks, out = {}, ["PICKED TAKES", "============", f"the owner's median pace: {med:.2f} words a second", ""]
    for n, rid, text, _ in lines:
        cands = [t for t in usable if t["line"] == n]
        key = re.sub(r"^v2_\d\d_", "", rid)
        w = wins.get(key)
        room = (w[1] - w[0] - LEAD - BREATH) if w else None
        if not cands:
            out.append(f"{n:2d} {rid:24s} NOTHING RECORDED: read this line again")
            continue
        scored = sorted(((take_score(t, med, room), t) for t in cands), key=lambda st: -st[0][0])
        (sc, why), best = scored[0]
        how = "best"
        if n in favs:
            f = favs[n]
            if isinstance(f, int):
                hit = [t for t in cands if t["take"] == f]
                if hit:
                    best, how = hit[0], "owner's favourite"
                    sc, why = take_score(best, med, room)
                else:
                    say(f"  favourite {n} {f}: line {n} has no take {f}; picked the best instead")
            else:
                hit = [st for st in scored if st[1]["wording"] == f]
                if hit:
                    (sc, why), best = hit[0]
                    how = f"owner's favourite ({f} wording)"
        picks[rid] = dict(line=n, label=best["label"], wav=best["wav"], wording=best["wording"], how=how,
                          score=round(sc, 1), why=why, finished_secs=best["finished_secs"],
                          others=[st[1]["label"] for st in scored if st[1] is not best])
        note = f"  ({', '.join(why)})" if why else ""
        out.append(f"{n:2d} {rid:24s} take {best['label']:5s} {best['wording']:8s} {best['finished_secs']:5.2f} s  "
                   f"{how}{note}; others: {', '.join(picks[rid]['others']) or 'none'}")
    (work / "picks.json").write_text(json.dumps(picks, indent=1), encoding="utf-8")
    out.append("")
    out.append(f"Change a pick: write lines like \"4 2\" (line 4, take 2) or \"4 short\" into {work / 'favourites.txt'}"
               " (or pass --take 4=2) and run pick and clean again.")
    txt = "\n".join(out) + "\n"
    (work / "picks.txt").write_text(txt, encoding="utf-8")
    say(txt)
    return 0 if len(picks) == len(lines) else 1


# ---------------------------------------------------------------- clean
CHAIN = ("highpass=f=80:poles=2,afftdn=nr=10:nf={nf:.1f}:tn=0,deesser=i=0.35:m=0.5:f=0.5,volume={g:.2f}dB,"
         "acompressor=threshold=-21dB:ratio=2:attack=12:release=180:knee=6:makeup=1")


def run_chain(x, chain, noise_len=None):
    pre = ""
    if noise_len:
        pre = f"asendcmd=c='0.02 afftdn sn start;{noise_len - 0.02:.3f} afftdn sn stop',"
    r = subprocess.run([FF, "-v", "error", "-f", "f32le", "-ar", str(SR), "-ac", "1", "-i", "-", "-af", pre + chain,
                        "-f", "f32le", "-ar", str(SR), "-ac", "1", "-"],
                       input=np.asarray(x, np.float32).tobytes(), capture_output=True, check=True)
    return np.frombuffer(r.stdout, np.float32).copy()


def chain_latency():
    """Samples the cleaning chain delays its output by (afftdn's window), measured on a test burst."""
    rng = np.random.default_rng(3)
    x = (rng.standard_normal(SR * 3) * 0.001).astype(np.float32)
    b = rng.standard_normal(int(0.2 * SR)).astype(np.float32) * np.hanning(int(0.2 * SR)).astype(np.float32) * 0.2
    x[2 * SR:2 * SR + len(b)] += b
    y = run_chain(x, CHAIN.format(nf=-60, g=0.0).split(",volume")[0])
    a = x[2 * SR - 2000:2 * SR + len(b) + 2000]
    best = (-1e9, 0)
    for lag in range(0, 4000, 8):
        c = float(np.dot(a, y[2 * SR - 2000 + lag:2 * SR + len(b) + 2000 + lag]))
        if c > best[0]:
            best = (c, lag)
    lo = max(0, best[1] - 8)
    for lag in range(lo, best[1] + 9):
        c = float(np.dot(a, y[2 * SR - 2000 + lag:2 * SR + len(b) + 2000 + lag]))
        if c > best[0]:
            best = (c, lag)
    return best[1]


def active_level(x):
    n = len(x) // HOP
    if n < 3:
        return -120.0
    e = (x[:n * HOP].astype(np.float64).reshape(n, HOP) ** 2).mean(1)
    d = 10 * np.log10(e + 1e-12)
    act = e[d > d.max() - 25]
    return 10 * math.log10(float(act.mean()) + 1e-12)


def cmd_clean(a):
    work = Path(a.work)
    rep = json.loads((work / "session" / "takes.json").read_text(encoding="utf-8"))
    picks = json.loads((work / "picks.json").read_text(encoding="utf-8"))
    out_dir = Path(a.lines_dir) if a.lines_dir else work / "lines"
    out_dir.mkdir(parents=True, exist_ok=True)
    srcs = {s["file"]: s for s in rep["sources"]}
    takes = {t["wav"]: t for t in rep["takes"] if t.get("wav")}
    lag = chain_latency()
    say(f"clean: the chain delays by {lag} samples ({1000 * lag / SR:.1f} ms); compensated")
    rows = []
    for n, rid, text, _ in load_lines():
        p = picks.get(rid)
        if not p:
            rows.append(f"{n:2d} {rid:24s} MISSING (nothing recorded or picked)")
            continue
        t = takes[p["wav"]]
        s = srcs[t["file"]]
        noise = load_audio(s["noise"])[:int(2.5 * SR)]
        if len(noise) < int(0.5 * SR):
            noise = np.concatenate([noise, np.zeros(int(0.5 * SR) - len(noise), np.float32)])
        cand = load_audio(t["wav"])
        on, off = t["onset_in_wav"], t["offset_in_wav"]
        g = float(np.clip(LINE_LEVEL - active_level(cand[int(on * SR):int(off * SR)]), -12, 30))
        nf = float(np.clip(s["noise_db"], -80, -20))
        pad = np.zeros(lag + SR // 2, np.float32)
        y = run_chain(np.concatenate([noise, cand, pad]), CHAIN.format(nf=nf, g=g), noise_len=len(noise) / SR)
        y = y[lag + len(noise):lag + len(noise) + len(cand)]
        a0, a1 = int((on - HEAD) * SR), int((off + TAIL) * SR)
        head = np.zeros(max(0, -a0), np.float32)
        tail = np.zeros(max(0, a1 - len(y)), np.float32)
        z = np.concatenate([head, y[max(0, a0):min(len(y), a1)], tail])
        fi, fo = int(0.03 * SR), int(0.08 * SR)
        z[:fi] *= np.linspace(0, 1, fi, dtype=np.float32)
        z[-fo:] *= np.linspace(1, 0, fo, dtype=np.float32)
        pk = float(np.abs(z).max())
        if pk > 10 ** (-1 / 20):
            z *= 10 ** (-1 / 20) / pk
            pk = 10 ** (-1 / 20)
        dst = out_dir / f"{rid}.wav"
        write24(dst, z)
        nfl = 10 * math.log10(float(np.mean(z[fi:int(0.09 * SR)].astype(np.float64) ** 2)) + 1e-12)
        lvl = active_level(z)
        rows.append(f"{n:2d} {rid:24s} take {p['label']:5s} {p['wording']:8s} {len(z) / SR:5.2f} s  peak {db(pk):5.1f} dBFS"
                    f"  speech {lvl:5.1f}  quiet head {nfl:6.1f} dBFS  (gain {g:+.1f} dB)")
    txt = ("FINISHED LINES (48 kHz mono 24-bit)\n===================================\n"
           "high-pass 80 Hz, noise reduction 10 dB from the noise profile, de-ess 0.35, compressor 2:1 at -21 dB;"
           f" {HEAD:.2f} s before the first word, {TAIL:.2f} s after the last, 30/80 ms fades\n\n" + "\n".join(rows) + "\n")
    (out_dir / "LINES.txt").write_text(txt, encoding="utf-8")
    say(txt)
    say(f"-> {out_dir}")
    return 0 if all("MISSING" not in r for r in rows) else 1


# ---------------------------------------------------------------- fit
def cmd_fit(a):
    if not have("PIL"):
        return rerun_with(plain_python(), ["fit"] + a.raw)
    sys.path.insert(0, str(HERE))
    import mix_vo
    import trailer_v2
    work = Path(a.work)
    wav_dir = Path(a.wav_dir or a.lines_dir or (work / "lines"))
    fit_dir = work / "fit"
    fit_dir.mkdir(parents=True, exist_ok=True)
    lines = json.loads(LINES_JSON.read_text(encoding="utf-8"))
    lengths, src = {}, {}
    picks = json.loads((work / "picks.json").read_text(encoding="utf-8")) if (work / "picks.json").exists() else {}
    for ln in lines:
        w = wav_dir / f"{ln['id']}.wav"
        if w.exists():
            lengths[ln["id"]], src[ln["id"]] = wav_seconds(w), "wav"
        elif ln["id"] in picks:
            lengths[ln["id"]], src[ln["id"]] = picks[ln["id"]]["finished_secs"], "pick"
    missing = [ln["id"] for ln in lines if ln["id"] not in lengths]
    if missing:
        raise SystemExit(f"fit: no take for {missing}")
    montage = Path(a.montage)
    tl = json.loads((montage / "timeline.json").read_text(encoding="utf-8"))
    wins_now = mix_vo.windows(tl)
    spec = json.loads(json.dumps(tl.get("timing", {}).get("spec", {}))) if tl.get("timing") else {}
    base_spec = json.loads(json.dumps(spec))
    _, total0, wins_plan = trailer_v2.plan_with(spec)
    drift = [k for k in wins_now if k in wins_plan and any(abs(x - y) > 0.02 for x, y in zip(wins_now[k], wins_plan[k]))]
    notes = []
    if drift or total0 != tl["total_frames"]:
        notes.append(f"WARNING: the montage's timeline.json ({tl['total_frames']} frames) does not match trailer_v2.py's "
                     f"plan ({total0} frames; windows differ for {drift}); the plan below starts from trailer_v2.py.")
    rows_now = mix_vo.place(lines, wins_now, lengths, True)

    def scene_frames(clip, sp):
        segs, _, _ = trailer_v2.plan_with(sp)
        return next(s["frames"] for s in segs if s["clip"] == clip)

    remedies, reshoot = [], {}
    for _ in range(64):
        _, total, wins = trailer_v2.plan_with(spec)
        rows = mix_vo.place(lines, wins, lengths, True)
        bad = next((r for r in rows if r["over"] > OVER_OK + 1e-6), None)
        if not bad:
            break
        key = mix_vo.key_of(bad["id"])
        need = bad["over"]
        if key == "intro":
            nf = int(math.ceil(need * FPS / 6.0) * 6)
            spec["intro_extra"] = round(spec.get("intro_extra", 0.0) + nf / FPS, 4)
            spec.setdefault("frames", {})["opening"] = scene_frames("opening", spec) + nf
            reshoot["opening"] = spec["frames"]["opening"]
            remedies.append(f"{bad['id']}: the intro runs {need:.2f} s over; hold the intro {nf / FPS:.2f} s longer "
                            f"(intro_extra) and reshoot the opening {nf} frames longer")
            continue
        if key == "outro":
            add = math.ceil(need * 10) / 10
            spec["closing_s"] = round(spec.get("closing_s", trailer_v2.CLOSING_S) + add, 2)
            remedies.append(f"{bad['id']}: the closing card holds {add:.1f} s longer ({spec['closing_s']} s; no reshoot)")
            continue
        # (b1) the transitions next to the scene, if trimming them within their floors is enough
        order = [c["clip"] for c in trailer_v2.G.CLIPS]
        k = order.index(key)
        adj = ([order[k - 1]] if k > 0 else []) + [key]
        trans = dict(trailer_v2.TRANSITIONS)
        trans.update({c: tuple(v) for c, v in spec.get("transitions", {}).items()})
        avail = {c: max(0.0, trans[c][1] - TRANS_FLOOR.get(trans[c][0], 0.8)) for c in adj}
        want = need - OVER_OK
        if key not in reshoot and sum(avail.values()) / 2 >= want - 1e-6:
            cut = {c: 0.0 for c in adj}
            while sum(cut.values()) / 2 < want - 1e-6:
                c = max(adj, key=lambda c: avail[c] - cut[c])
                step = min(1.0 / FPS * 3, avail[c] - cut[c])
                if step <= 1e-9:
                    break
                cut[c] += step
            for c, x in cut.items():
                if x > 0:
                    secs = round(round((trans[c][1] - x) * FPS) / FPS, 4)
                    spec.setdefault("transitions", {})[c] = [trans[c][0], secs]
                    remedies.append(f"{bad['id']}: {need:.2f} s over; the {trans[c][0]} after {c} "
                                    f"{trans[c][1]:.2f} -> {secs:.2f} s (no reshoot)")
            continue
        # (b2) more clip frames: reshoot the clip longer on both sides (same path, slower move)
        nf = int(math.ceil(need * FPS / 6.0) * 6)
        cur = scene_frames(key, spec)
        spec.setdefault("frames", {})[key] = cur + nf
        reshoot[key] = cur + nf
        remedies.append(f"{bad['id']}: {need:.2f} s over; reshoot {key} {nf} frames longer "
                        f"({cur} -> {cur + nf} at 60 fps, both sides)")
    _, total, wins = trailer_v2.plan_with(spec)
    rows_after = mix_vo.place(lines, wins, lengths, True)
    changed = spec != base_spec

    def table(rows, wins_used):
        out = [f"{'id':22s} {'window':>15s} {'room':>5s} {'take':>5s} {'start':>7s} {'end':>7s} {'over':>6s}  verdict"]
        for r in rows:
            k = mix_vo.key_of(r["id"])
            w0, w1 = wins_used[k]
            room = w1 - w0 - LEAD - BREATH
            if r["over"] <= 0:
                v = "fits"
            elif r["over"] <= OVER_OK:
                v = f"accepted: ends {r['over']:.2f} s past the breath ({BREATH - r['over']:.2f} s before the window ends)"
            else:
                v = "TOO LONG"
            if r["start"] > w0 + LEAD + 0.005:
                v += f"; starts {r['start'] - w0 - LEAD:.2f} s late (pushed)"
            out.append(f"{r['id']:22s} {w0:6.2f}-{w1:6.2f}  {room:5.2f} {r['dur']:5.2f} {r['start']:7.2f} "
                       f"{r['end']:7.2f} {r['over']:+6.2f}  {v}")
        return out

    txt = ["FIT PLAN: the picture follows the voice", "======================================", ""]
    txt += notes
    txt += [f"montage: {montage / 'timeline.json'} ({tl['total_frames']} frames, {tl['duration']:.2f} s)",
            f"lines:   {wav_dir} ({'finished WAVs' if all(v == 'wav' for v in src.values()) else 'some lengths from the picks'})",
            f"rule: a line starts {LEAD} s into its window and should end {BREATH} s before it ends; up to {OVER_OK} s "
            "over is accepted (it ends inside the transition); mix_vo.py --fit pushes a line back if the one "
            "before has not ended 0.3 s earlier. The voice is never sped up.", "",
            "NOW (the current montage):"]
    txt += table(rows_now, wins_now)
    txt.append("")
    timing_path = fit_dir / "timing_owner.json"
    if not changed:
        txt.append("Every line fits its scene (or runs over by an accepted amount). No picture change: mix now.")
        if timing_path.exists():
            timing_path.unlink()
    else:
        spec["note"] = ("owner_vo.py fit: the picture follows the owner's voice; frames = 60 fps clip lengths "
                        "(a longer reshoot), transitions = a scene's out-transition [kind, seconds], intro_extra = "
                        "seconds the intro holds longer, closing_s = the closing card")
        timing_path.write_text(json.dumps(spec, indent=1) + "\n", encoding="utf-8")
        txt.append("REMEDIES (in order):")
        txt += ["  " + r for r in remedies]
        txt.append("")
        txt.append(f"AFTER (trailer_v2.py --timing {timing_path}): {total} frames, {total / FPS:.2f} s "
                   f"({total - tl['total_frames']:+d} frames)")
        txt += table(rows_after, wins)
        trailer_v2.main(["--plan-only", "--timing", str(timing_path), "--out", str(fit_dir)])
        txt.append("")
        txt.append("WHAT TO DO (from " + str(HERE) + "):")
        step = 1
        if reshoot:
            import collect_frames
            ud_dir = collect_frames.REQUIRE["ud"][0]
            for clip, nfr in reshoot.items():
                txt.append(f"  {step}. reshoot {clip} with {nfr} frames on both sides (the same camera path, slower):")
                txt.append(f"       python gen_video_v2.py --clip {clip} --frames {nfr} --side both")
                txt.append(f"       powershell -NoProfile -File run_v2_lane.ps1 -Side vanilla -Clips {clip} -Expect {nfr}")
                txt.append(f"       powershell -NoProfile -File run_v2_lane.ps1 -Side ud -Clips {clip} -Expect {nfr} "
                           f"-GameDir <worktrees>\\{ud_dir} -DataJunction RemasterUD-video-reshoot")
                step += 1
        txt.append(f"  {step}. re-render both montages with the timing file:")
        txt.append(f"       python trailer_v2.py --timing {timing_path}")
        txt.append(f"       python trailer_v2.py --layout sbs --timing {timing_path}")
        step += 1
        txt.append(f"  {step}. then: python owner_vo.py finish --work {work}")
    out = "\n".join(txt) + "\n"
    (fit_dir / "FIT_PLAN.txt").write_text(out, encoding="utf-8")
    say(out)
    say(f"-> {fit_dir / 'FIT_PLAN.txt'}")
    return 0


# ---------------------------------------------------------------- finish, verify, preview
def cmd_finish(a):
    if not have("PIL"):
        return rerun_with(plain_python(), ["finish"] + a.raw)
    work = Path(a.work)
    montage = Path(a.montage)
    tl = json.loads((montage / "timeline.json").read_text(encoding="utf-8"))
    tp = work / "fit" / "timing_owner.json"
    if tp.exists() and not a.anyway:
        want = {k: v for k, v in json.loads(tp.read_text(encoding="utf-8")).items() if k != "note"}
        have_ = {k: v for k, v in (tl.get("timing") or {}).get("spec", {}).items() if k != "note"}
        if want != have_:
            raise SystemExit(f"finish: the fit plan ({tp}) changes the picture but the montage in {montage} was not "
                             "rendered with it. Do the reshoots/re-render in fit/FIT_PLAN.txt first (or pass --anyway).")
    wav_dir = work / "lines"
    cmd = [plain_python(), str(HERE / "mix_vo.py"), "--wav-dir", str(wav_dir), "--fit", "--name", a.name,
           "--out", str(montage), "--final-dir", str(a.final_dir)]
    if a.also is not None:
        cmd += ["--also", a.also]
    say("finish: " + " ".join(cmd))
    r = subprocess.run(cmd)
    if r.returncode:
        return r.returncode
    rc = cmd_verify(a)
    if not a.no_preview:
        rc |= cmd_preview(a)
    return rc


def listen_check(mix, plan_rows, device):
    """Transcribe the final mix and compare each line's words where mix_vo.py placed it."""
    from faster_whisper import WhisperModel
    dev = pick_device(device)
    model = WhisperModel("large-v3", device=dev, compute_type="float16" if dev == "cuda" else "int8",
                         local_files_only=True)
    segs, _ = model.transcribe(str(mix), language="en", beam_size=5, word_timestamps=True, vad_filter=True,
                               condition_on_previous_text=False, initial_prompt=PROMPT)
    words = [dict(w=w.word, s=float(w.start), e=float(w.end), p=float(w.probability)) for s in segs for w in s.words]
    T = tokens(words)
    lines = {rid: text for _, rid, text, _ in load_lines()}
    out, bad = [], 0
    for r in plan_rows:
        got = [t["t"] for t in T if r["start"] - 0.5 <= (t["s"] + t["e"]) / 2 <= r["end"] + 0.6]
        ref = ref_tokens(lines[r["id"]])
        sm = difflib.SequenceMatcher(a=ref, b=got)
        miss = [w for op, i1, i2, j1, j2 in sm.get_opcodes() if op in ("delete", "replace") for w in ref[i1:i2]]
        ok = sm.ratio() >= 0.85 and len(miss) <= 1
        bad += 0 if ok else 1
        out.append(f"  {r['id']:22s} {'OK  ' if ok else 'FAIL'} {sm.ratio():.2f} at {r['start']:6.2f}-{r['end']:6.2f}"
                   + (f"  not heard: {' '.join(miss)}" if miss else ""))
    return out, bad


def read_plan(montage):
    rows = []
    p = Path(montage) / "vo_plan.txt"
    for ln in p.read_text(encoding="utf-8").splitlines():
        m = re.match(r"^(v2_\S+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)", ln)
        if m:
            rows.append(dict(id=m.group(1), start=float(m.group(2)), end=float(m.group(3)), dur=float(m.group(4))))
    return rows


def cmd_verify(a):
    work = Path(a.work)
    fin = Path(a.final_dir)
    master = fin / f"{a.name}_youtube_1080p60.mp4"
    tl = json.loads((Path(a.montage) / "timeline.json").read_text(encoding="utf-8"))
    rep = ["VERIFY " + a.name, "=" * (7 + len(a.name))]
    bad = 0
    for f in (master, fin / f"{a.name}_subtitled.mp4", fin / f"{a.name}_side_by_side_3840x1080p60.mp4", fin / f"{a.name}.srt"):
        if not f.exists():
            rep.append(f"MISSING {f}")
            bad += 1 if "side_by_side" not in f.name else 0
            continue
        if f.suffix == ".mp4":
            r = subprocess.run([FFPROBE, "-v", "error", "-select_streams", "v:0", "-show_entries",
                                "stream=r_frame_rate,nb_frames,width,height", "-of", "json", str(f)],
                               capture_output=True, text=True, check=True)
            s = json.loads(r.stdout)["streams"][0]
            ok = s["r_frame_rate"] == "60/1" and int(s.get("nb_frames", 0)) == tl["total_frames"]
            bad += 0 if ok else 1
            rep.append(f"{'OK  ' if ok else 'FAIL'} {f.name}: {s['width']}x{s['height']} {s['r_frame_rate']} "
                       f"{s.get('nb_frames')} frames (timeline {tl['total_frames']}), {f.stat().st_size / 1e6:.0f} MB")
    if master.exists():
        r = subprocess.run([FF, "-hide_banner", "-nostats", "-i", str(master), "-vn", "-af",
                            "loudnorm=I=-16:TP=-1.5:LRA=11:print_format=json", "-f", "null", "-"],
                           capture_output=True, text=True)
        j = json.loads(r.stderr[r.stderr.rindex("{"):r.stderr.rindex("}") + 1])
        li, tp = float(j["input_i"]), float(j["input_tp"])
        ok = abs(li + 16) <= 0.5 and tp <= -1.0
        bad += 0 if ok else 1
        rep.append(f"{'OK  ' if ok else 'FAIL'} loudness of the master: {li:.1f} LUFS integrated, true peak {tp:.1f} dBTP, "
                   f"LRA {j['input_lra']} LU (loudnorm print; target -16 LUFS, under -1 dBTP)")
        # the listen-check runs in the whisper venv
        wav = work / "_mix_listen.wav"
        subprocess.run([FF, "-y", "-v", "error", "-i", str(master), "-vn", "-ac", "1", "-ar", "16000", str(wav)], check=True)
        rows = read_plan(a.montage)
        (work / "_plan_rows.json").write_text(json.dumps(rows), encoding="utf-8")
        cp = subprocess.run([str(VENV_PY), str(Path(__file__).resolve()), "_listen", "--mix", str(wav), "--rows",
                             str(work / "_plan_rows.json"), "--device", a.device], capture_output=True, text=True)
        rep.append("whisper listen-check of the final mix (each line where it was placed):")
        rep += [ln for ln in cp.stdout.splitlines() if ln.startswith("  v2_")]
        if cp.returncode:
            bad += 1
            rep.append(f"  listen-check exit {cp.returncode} {cp.stderr.strip()[-300:]}")
        wav.unlink(missing_ok=True)
        (work / "_plan_rows.json").unlink(missing_ok=True)
    # check_vo.py on the finished lines (transcript per line and its own fit rule)
    cv = subprocess.run([str(VENV_PY), str(HERE / "check_vo.py"), "--wav-dir", str(work / "lines"), "--timeline",
                         str(Path(a.montage) / "timeline.json"), "--out", str(work / "vo_check_owner.json"),
                         "--device", pick_device(a.device)], capture_output=True, text=True)
    rep.append("check_vo.py on the lines (its fit test is the strict 0.6 s rule; an accepted overrun shows as FAIL there):")
    rep += ["  " + ln for ln in cv.stdout.splitlines() if ln.strip() and not ln.startswith("    heard")]
    txt = "\n".join(rep) + "\n"
    (work / "VERIFY.txt").write_text(txt, encoding="utf-8")
    say(txt)
    say(f"-> {work / 'VERIFY.txt'}")
    return 1 if bad else 0


def cmd_listen(a):
    rows = json.loads(Path(a.rows).read_text(encoding="utf-8"))
    out, bad = listen_check(a.mix, rows, a.device)
    print("\n".join(out))
    return 1 if bad else 0


def cmd_preview(a):
    fin = Path(a.final_dir)
    src = fin / f"{a.name}_subtitled.mp4"
    if not src.exists():
        src = fin / f"{a.name}_youtube_1080p60.mp4"
    dst = fin / f"{a.name}_preview_720p60.mp4"
    r = subprocess.run([FFPROBE, "-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", str(src)],
                       capture_output=True, text=True, check=True)
    dur = float(r.stdout.strip())
    kbps = int(27.0e6 * 8 / dur / 1000) - 160          # 27 MB in all, 128 kb/s of it audio
    for attempt in range(3):
        subprocess.run([FF, "-y", "-v", "error", "-i", str(src), "-vf", "scale=1280:720:flags=lanczos", "-c:v", "libx264",
                        "-preset", "slow", "-b:v", f"{kbps}k", "-maxrate", f"{int(kbps * 1.25)}k", "-bufsize",
                        f"{kbps * 2}k", "-pix_fmt", "yuv420p", "-r", "60", "-g", "60", "-c:a", "aac", "-b:a", "128k",
                        "-movflags", "+faststart", str(dst)], check=True)
        mb = dst.stat().st_size / 1e6
        if mb < 30:
            break
        kbps = int(kbps * 0.85)
    say(f"preview: {dst} ({mb:.1f} MB, {kbps} kb/s video){'' if mb < 30 else ' STILL OVER 30 MB'}")
    return 0 if mb < 30 else 1


# ---------------------------------------------------------------- all
def cmd_all(a):
    rc = cmd_split(a)
    if rc:
        return rc
    rc = cmd_pick(a)
    rc2 = cmd_clean(a)
    rc3 = cmd_fit(a) if have("PIL") else rerun_with(plain_python(), ["fit", "--work", a.work, "--montage", a.montage])
    say("")
    say("NEXT: read fit/FIT_PLAN.txt. When it says \"mix now\" (or after its reshoot and re-render):")
    say(f"  python owner_vo.py finish --work {a.work}")
    return rc or rc2 or rc3


def split_args(a):
    out = ["--work", a.work, "--device", a.device]
    for r in a.rec or []:
        out += ["--rec", r]
    if a.retranscribe:
        out.append("--retranscribe")
    return out


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--work", default=str(OWNER), help="the working folder (default tmp/video/v2/vo_owner)")
    common.add_argument("--device", default="auto", help="whisper device: auto (CUDA with 6 GB free), cuda or cpu")
    common.add_argument("--montage", default=str(V2 / "out-split"), help="the montage folder with timeline.json")
    for name in ("split", "pick", "clean", "fit", "all", "finish", "verify", "preview", "_listen"):
        p = sub.add_parser(name, parents=[common])
        if name in ("split", "all"):
            p.add_argument("--rec", action="append", help="a WAV or a folder of WAVs (default reading/recordings)")
            p.add_argument("--retranscribe", action="store_true")
        if name in ("pick", "all"):
            p.add_argument("--fav", default="", help="favourites file (default <work>/favourites.txt)")
            p.add_argument("--take", action="append", help="line=take, e.g. 4=2 or 4=short")
        if name in ("clean", "all"):
            p.add_argument("--lines-dir", default="", help="where the finished <id>.wav go (default <work>/lines)")
        if name in ("fit", "all"):
            p.add_argument("--wav-dir", default="", help="the finished lines (default <work>/lines)")
        if name in ("finish", "verify", "preview"):
            p.add_argument("--name", default="uslu_dur_part1_graphics_owner")
            p.add_argument("--final-dir", default=str(V2 / "final"))
        if name == "finish":
            p.add_argument("--also", default=None, help="the side-by-side montage (mix_vo.py's default when omitted)")
            p.add_argument("--anyway", action="store_true", help="mix even if the montage does not follow the fit plan")
            p.add_argument("--no-preview", action="store_true")
        if name == "_listen":
            p.add_argument("--mix", required=True)
            p.add_argument("--rows", required=True)
    a = ap.parse_args(argv)
    a.raw = argv[1:]
    for k in ("lines_dir", "wav_dir", "fav", "take", "rec", "retranscribe"):
        if not hasattr(a, k):
            setattr(a, k, None)
    return dict(split=cmd_split, pick=cmd_pick, clean=cmd_clean, fit=cmd_fit, all=cmd_all, finish=cmd_finish,
                verify=cmd_verify, preview=cmd_preview, _listen=cmd_listen)[a.cmd](a)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
