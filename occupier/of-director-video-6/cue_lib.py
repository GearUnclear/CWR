"""Helpers for make_cues.py: find engine lines and scenario notes in a
trace.json (parse_trace.py output) by pattern, in game-time order."""
import json
import re


class T:
    def __init__(self, path):
        self.t = json.load(open(path))
        self.info = sorted(self.t["info"], key=lambda e: e["sim"])
        self.anom = sorted(self.t.get("anomalies", []), key=lambda e: e["sim"])
        self.end = self.t["trace"][-1][0] if self.t["trace"] else 0

    def find(self, pat, after=-1.0, before=1e12, kind=None):
        for e in self.info:
            if after <= e["sim"] <= before and (kind is None or e["kind"] == kind) and re.search(pat, e["text"]):
                return e
        return None

    def find_anom(self, pat, after=-1.0, before=1e12):
        for e in self.anom:
            if after <= e["sim"] <= before and re.search(pat, e["text"]):
                return e
        return None

    def all(self, pat, after=-1.0, before=1e12):
        return [e for e in self.info if after <= e["sim"] <= before and re.search(pat, e["text"])]

    def note(self, pat, after=-1.0):
        return self.find(pat, after, kind="note")


def text(e, prefix=""):
    if e is None:
        return ""
    t = e["text"]
    if e.get("kind") == "note":
        t = "scenario note: " + t
    return prefix + "[%d s] " % round(e["sim"]) + t
