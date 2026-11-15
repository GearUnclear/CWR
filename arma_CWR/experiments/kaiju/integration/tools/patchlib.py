r"""Tiny helper for the one-off integration patches (tools\patch_*.py): exact text replacement in
the integration COPIES of the feature scripts, refusing anything ambiguous. The patch scripts are
kept as the record of what the integration changed against each feature's own file."""
import os

KJ = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "kj"))


class File:
    def __init__(self, rel):
        self.path = os.path.join(KJ, rel)
        with open(self.path, "r", encoding="utf-8", newline="") as f:
            self.text = f.read()
        self.crlf = "\r\n" in self.text
        if self.crlf:
            self.text = self.text.replace("\r\n", "\n")
        self.n = 0

    def rep(self, old, new, count=1):
        found = self.text.count(old)
        if found != count:
            raise SystemExit("%s: expected %d occurrence(s), found %d, of:\n%s" % (self.path, count, found, old[:300]))
        self.text = self.text.replace(old, new)
        self.n += 1
        return self

    def after(self, anchor, new):
        return self.rep(anchor, anchor + new)

    def before(self, anchor, new):
        return self.rep(anchor, new + anchor)

    def save(self):
        text = self.text.replace("\n", "\r\n") if self.crlf else self.text
        with open(self.path, "w", encoding="utf-8", newline="") as f:
            f.write(text)
        print("patched %s (%d edits)" % (os.path.relpath(self.path, KJ), self.n))
