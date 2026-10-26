"""The shared scenario clock: both runs execute the same script, but each beat
waits on the engine, so a beat lasts a different game time on each side.
The clock aligns the two runs at the script's own milestones (UDNOTE lines
both runs write) and lets each side's game time run at the same rate from
there; the side that finishes a beat first holds until the other catches up.

    c (scenario clock) -> game time on A, game time on B
"""
import re

MILESTONES = [
    r"lever: poolFloor 0.3",
    r"lever: pool restored",
    r"the player steps between",
    r"the player leaves for the hills",
    r"lever: Outpost reserve 0",
    r"the player enters the Outpost bubble",
    r"the player leaves for the hills",
    r"the cells in the empty towns move on",
    r"^end: pool",
]


def anchors(notes, end):
    out = [0.0]
    last = 0.0
    for pat in MILESTONES:
        hit = None
        for e in notes:
            if e["sim"] > last and re.search(pat, e["text"]):
                hit = e["sim"]
                break
        if hit is None:
            raise SystemExit("milestone missing: " + pat)
        out.append(hit)
        last = hit
    if end > out[-1]:
        out.append(end)
    return out


class Clock:
    def __init__(self, notes_a, end_a, notes_b, end_b):
        self.a = anchors(notes_a, end_a)
        self.b = anchors(notes_b, end_b)
        n = min(len(self.a), len(self.b))
        self.a, self.b = self.a[:n], self.b[:n]
        self.c = [0.0]
        for k in range(n - 1):
            self.c.append(self.c[-1] + max(self.a[k + 1] - self.a[k], self.b[k + 1] - self.b[k]))
        self.end = self.c[-1]

    def _side(self, side):
        return self.a if side == "A" else self.b

    def sim(self, side, c):
        s = self._side(side)
        for k in range(len(self.c) - 1):
            if c <= self.c[k + 1] or k == len(self.c) - 2:
                return min(s[k] + max(0.0, c - self.c[k]), s[k + 1])
        return s[-1]

    def to_c(self, side, t):
        s = self._side(side)
        for k in range(len(s) - 1):
            if t <= s[k + 1] or k == len(s) - 2:
                return self.c[k] + max(0.0, t - s[k])
        return self.c[-1]
