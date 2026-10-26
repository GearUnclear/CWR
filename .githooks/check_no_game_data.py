#!/usr/bin/env python3
"""Commit guard: keep copyrighted game data and secrets out of the public fork.

Uslu dur! is GPL. Bohemia's game data and the @LoBo mod data are APL-SA and
must never be committed: pbo, paa, p3d, wrp, sounds, de-binarized configs and
stringtables. Neither may API keys or other secrets.

Modes:
  --staged      check the blobs staged in the index (pre-commit hook)
  --push        check every blob added or modified by the commits being pushed;
                reads pre-push stdin lines "<local ref> <local sha> <remote ref> <remote sha>"
  --tree REV    check every blob in the tree of REV (CI, audits)

Exit 0 when clean, 1 when something is blocked, 2 on a usage or git error.

The allowlist (game-data-allowlist.txt next to this file) is found relative to
this script, never relative to the current worktree, so the hook keeps working
in worktrees whose branch does not carry the guard files. Python 3 stdlib only.
"""

import argparse
import os
import re
import subprocess
import sys
import threading
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT_ALLOWLIST = HERE / "game-data-allowlist.txt"

MAX_SIZE = 5 * 1024 * 1024  # 5,242,880 bytes
CONFIG_DIR_LIMIT = 64 * 1024  # config/stringtable files above this need an exact entry
ZERO_SHA_RE = re.compile(r"^0+$")

BLOCKED_EXTENSIONS = {
    "pbo", "paa", "pac", "p3d", "wrp", "wss", "ogg", "wav", "rtm", "fxy", "fps",
    "bin", "ebo", "7z", "zip", "rar", "exe", "dll", "mp4", "mkv", "jpg", "jpeg",
    "png", "ttf", "otf",
}
BLOCKED_NAMES = {".env", ".trident.env"}
# Directory segments that are blocked at any depth (compared case-insensitively).
BLOCKED_SEGMENTS = {
    "packages",
    "@lobo",
    "arma cold war assault [classic]",
    "arma cold war assault demo [remaster]",
}
# Path prefixes blocked from the repo root.
BLOCKED_PREFIXES = ("tmp/", "arma_CWR/tmp/")

# Magic bytes at offset 0 (model, world and binarized config formats).
MAGIC = (
    (b"MLOD", "model file (MLOD magic)"),
    (b"ODOL", "binarized model (ODOL magic)"),
    (b"SP3X", "model file (SP3X magic)"),
    (b"4WVR", "world file (4WVR magic)"),
    (b"8WVR", "world file (8WVR magic)"),
    (b"\x00raP", "binarized config (raP magic)"),
)
LFS_POINTER = b"version https://git-lfs"

# Secret patterns. Built from pieces so this file never matches itself.
SECRET_PATTERNS = (
    ("OpenRouter key", "sk-" + r"or-v1-[0-9a-f]{20,}"),
    ("Anthropic key", "sk-" + r"ant-[A-Za-z0-9_-]{20,}"),
    ("GitHub token", "gh" + r"p_[A-Za-z0-9]{30,}"),
    ("GitHub fine-grained token", "github" + r"_pat_[A-Za-z0-9_]{30,}"),
    ("AWS access key id", "AK" + r"IA[0-9A-Z]{16}"),
    ("private key block", "-----" + r"BEGIN [A-Z ]*PRIVATE KEY-----"),
)
API_KEY_RE = re.compile(rb"[A-Z0-9_]*API_KEY\s*=\s*['\"]?[A-Za-z0-9_\-]{16,}")
PLACEHOLDER_MARKERS = (b"<", b"$", b"{", b"...", b"your")
SECRET_RES = [(name, re.compile(pat.encode())) for name, pat in SECRET_PATTERNS]

CONFIG_STARTS = (b"class cfg", b"language,")

HINT_FIX = {
    "staged": "unstage the file (git rm --cached <path>)",
    "push": "drop it from the unpushed commits (amend or rebase them) before pushing",
    "tree": "remove it from the branch (git rm --cached <path>)",
}
HINT = (
    "hint: game data, LoBo data and secrets never belong in this GPL repo; {fix}. A genuine "
    "false positive can be added to .githooks/game-data-allowlist.txt with a # comment "
    "justifying it, but Bohemia game data or LoBo data never qualifies, and secrets are "
    "never allowlisted."
)


# ---------------------------------------------------------------- allowlist

class Allowlist:
    def __init__(self, exact=(), prefixes=()):
        self.exact = set(exact)
        self.prefixes = tuple(prefixes)

    @classmethod
    def load(cls, path):
        exact, prefixes = set(), []
        try:
            text = Path(path).read_text(encoding="utf-8")
        except FileNotFoundError:
            print(f"check_no_game_data: allowlist {path} not found; checking without it",
                  file=sys.stderr)
            return cls()
        for raw in text.splitlines():
            line = raw.split("#", 1)[0].strip()
            if not line:
                continue
            if line.endswith("/"):
                prefixes.append(line)
            else:
                exact.add(line)
        return cls(exact, prefixes)

    def level(self, path):
        """'exact', 'prefix' or None."""
        if path in self.exact:
            return "exact"
        if any(path.startswith(p) for p in self.prefixes):
            return "prefix"
        return None


# ---------------------------------------------------------------- git helpers

def git(*args, input_bytes=None, check=True):
    proc = subprocess.run(["git", *args], input=input_bytes, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
    if check and proc.returncode != 0:
        msg = proc.stderr.decode("utf-8", "replace").strip()
        raise GitError(f"git {' '.join(args)} failed: {msg}")
    return proc


class GitError(Exception):
    pass


def decode_path(b):
    return b.decode("utf-8", "surrogateescape")


def staged_entries():
    """(path, sha, mode) for staged additions and modifications."""
    out = git("diff", "--cached", "--raw", "-z", "--diff-filter=ACMR").stdout
    return parse_raw_diff(out)


def parse_raw_diff(out):
    """Parse `git diff --raw -z` / `git diff-tree -r -z` output into (path, sha, mode)."""
    entries = []
    fields = out.split(b"\0")
    i = 0
    n = len(fields)
    while i < n:
        head = fields[i]
        if not head.startswith(b":"):
            i += 1  # commit-id header lines or the trailing empty field
            continue
        parts = head[1:].split(b" ")
        new_mode, new_sha, status = parts[1].decode(), parts[3].decode(), parts[4].decode()
        if status[:1] in ("R", "C"):
            path = fields[i + 2]
            i += 3
        else:
            path = fields[i + 1]
            i += 2
        if new_mode == "160000" or ZERO_SHA_RE.match(new_sha):
            continue  # submodule link, or a working tree side we cannot read
        entries.append((decode_path(path), new_sha, new_mode))
    return entries


def tree_entries(rev):
    out = git("ls-tree", "-r", "-z", "--full-tree", rev).stdout
    entries = []
    for rec in out.split(b"\0"):
        if not rec:
            continue
        meta, path = rec.split(b"\t", 1)
        mode, otype, sha = meta.decode().split(" ")
        if otype != "blob":
            continue
        entries.append((decode_path(path), sha, mode))
    return entries


def commit_exists(sha):
    return git("cat-file", "-e", f"{sha}^{{commit}}", check=False).returncode == 0


def push_entries(stdin_text):
    commits = []
    seen_commits = set()
    for line in stdin_text.splitlines():
        parts = line.split()
        if len(parts) != 4:
            continue
        _local_ref, local_sha, _remote_ref, remote_sha = parts
        if ZERO_SHA_RE.match(local_sha):
            continue  # branch deletion
        if not ZERO_SHA_RE.match(remote_sha) and commit_exists(remote_sha):
            revs = git("rev-list", f"{remote_sha}..{local_sha}").stdout
        else:
            revs = git("rev-list", local_sha, "--not", "--remotes").stdout
        for c in revs.decode().split():
            if c not in seen_commits:
                seen_commits.add(c)
                commits.append(c)
    entries = []
    seen = set()
    for c in commits:
        out = git("diff-tree", "-r", "-m", "-z", "--root", "--no-renames",
                  "--no-commit-id", "--diff-filter=AM", c).stdout
        for e in parse_raw_diff(out):
            if (e[0], e[1]) not in seen:
                seen.add((e[0], e[1]))
                entries.append(e)
    return entries, len(commits)


def read_blobs(shas):
    """Yield (sha, data) for each sha via one streaming `git cat-file --batch`."""
    if not shas:
        return
    proc = subprocess.Popen(["git", "cat-file", "--batch"], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)

    def feed():
        try:
            proc.stdin.write("".join(s + "\n" for s in shas).encode())
        finally:
            proc.stdin.close()

    writer = threading.Thread(target=feed, daemon=True)
    writer.start()
    out = proc.stdout
    for sha in shas:
        header = out.readline()
        bits = header.split()
        if len(bits) < 3:  # "<sha> missing"
            yield sha, None
            continue
        size = int(bits[2])
        data = out.read(size)
        out.read(1)  # trailing LF
        yield sha, data
    writer.join()
    proc.stdout.close()
    proc.wait()


# ---------------------------------------------------------------- rules

def path_findings(path, level):
    """Rules that need only the path."""
    found = []
    name = path.rsplit("/", 1)[-1]
    if name.lower() in BLOCKED_NAMES:
        found.append(f"secrets file name '{name}'")
    if level == "exact":
        return found
    if level is None:
        ext = name.rsplit(".", 1)[-1].lower() if "." in name else ""
        if ext in BLOCKED_EXTENSIONS:
            found.append(f"blocked extension .{ext} (game data, media or binary)")
        segs = [s.lower() for s in path.split("/")[:-1]]
        hit = next((s for s in segs if s in BLOCKED_SEGMENTS), None)
        if hit:
            found.append(f"forbidden folder '{hit}/' (game data, mod data or local install)")
        elif path.startswith(BLOCKED_PREFIXES):
            found.append("forbidden folder (local scratch tmp/)")
    return found


def first_meaningful_line(data):
    head = data[:4096]
    if head.startswith(b"\xef\xbb\xbf"):
        head = head[3:]
    in_block = False
    for line in head.splitlines():
        s = line.strip()
        if in_block:
            if b"*/" in s:
                in_block = False
                s = s.split(b"*/", 1)[1].strip()
            else:
                continue
        if not s or s.startswith(b"//") or s.startswith(b"#"):
            continue
        if s.startswith(b"/*"):
            rest = s[2:]
            if b"*/" in rest:
                s = rest.split(b"*/", 1)[1].strip()
                if not s:
                    continue
            else:
                in_block = True
                continue
        return s
    return b""


def secret_findings(data):
    found = []
    for name, rx in SECRET_RES:
        if rx.search(data):
            found.append(f"possible secret ({name})")
    for m in API_KEY_RE.finditer(data):
        # Look at the rest of the line after '=' for placeholder markers.
        start = m.start()
        eol = data.find(b"\n", start)
        line = data[start:eol if eol != -1 else len(data)]
        tail = line.split(b"=", 1)[1].lower() if b"=" in line else b""
        if any(mk in tail for mk in PLACEHOLDER_MARKERS):
            continue
        found.append("possible secret (API_KEY assignment)")
        break
    return found


def content_findings(data, size, level):
    """Rules that need the blob. Returns a list of reasons."""
    found = []
    if level != "exact":
        if size > MAX_SIZE:
            found.append(f"file is {size:,} bytes, over the 5 MB limit")
        if level is None:
            head = data[:32]
            if head.startswith(b"\x00sreV") or b"sreV" in head:
                found.append("PBO archive (sreV header)")
            for magic, why in MAGIC:
                if head.startswith(magic):
                    found.append(why)
            if head.startswith(LFS_POINTER):
                found.append("Git LFS pointer (the real content would bypass this check)")
        if b"\0" not in data[:8000]:
            first = first_meaningful_line(data).lower()
            if first.startswith(CONFIG_STARTS):
                kind = "stringtable" if first.startswith(b"language,") else "config dump"
                if level is None:
                    found.append(f"looks like a {kind} (first line starts "
                                 f"'{first[:20].decode('ascii', 'replace')}')")
                elif size > CONFIG_DIR_LIMIT:
                    found.append(f"{kind} of {size:,} bytes, over 64 KB; a directory "
                                 "allowlist entry does not cover it, list the exact path")
    found.extend(secret_findings(data))
    return found


def check(entries, allow):
    """entries: (path, sha, mode). Returns sorted list of (path, reason)."""
    problems = []
    need = {}
    for path, sha, _mode in entries:
        level = allow.level(path)
        for r in path_findings(path, level):
            problems.append((path, r))
        need.setdefault(sha, []).append((path, level))
    for sha, data in read_blobs(list(need)):
        if data is None:
            for path, _ in need[sha]:
                problems.append((path, f"blob {sha} could not be read"))
            continue
        cache = {}
        for path, level in need[sha]:
            if level not in cache:
                cache[level] = content_findings(data, len(data), level)
            for r in cache[level]:
                problems.append((path, r))
    problems.sort()
    return problems


# ---------------------------------------------------------------- main

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--staged", action="store_true", help="check staged blobs")
    mode.add_argument("--push", action="store_true", help="check commits from pre-push stdin")
    mode.add_argument("--tree", metavar="REV", help="check every blob in REV's tree")
    ap.add_argument("--allowlist", default=str(DEFAULT_ALLOWLIST),
                    help="allowlist file (default: next to this script)")
    ap.add_argument("--no-allowlist", action="store_true",
                    help="ignore the allowlist (used to build it)")
    ap.add_argument("--quiet", action="store_true", help="print nothing when clean")
    args = ap.parse_args(argv)
    for stream in (sys.stdout, sys.stderr):
        try:  # odd path bytes must never crash the report on a cp1252 console
            stream.reconfigure(errors="backslashreplace")
        except (AttributeError, ValueError):
            pass

    allow = Allowlist() if args.no_allowlist else Allowlist.load(args.allowlist)
    try:
        if args.staged:
            entries = staged_entries()
            what = f"{len(entries)} staged file(s)"
        elif args.push:
            entries, ncommits = push_entries(sys.stdin.read())
            what = f"{len(entries)} blob(s) in {ncommits} new commit(s)"
        else:
            entries = tree_entries(args.tree)
            what = f"{len(entries)} file(s) in {args.tree}"
        problems = check(entries, allow)
    except GitError as exc:
        print(f"check_no_game_data: {exc}", file=sys.stderr)
        return 2

    if not problems:
        if not args.quiet and not args.staged:
            print(f"check_no_game_data: OK, {what} checked")
        return 0
    print(f"check_no_game_data: BLOCKED, {len(problems)} problem(s) in {what}:", file=sys.stderr)
    for path, reason in problems:
        print(f"  {path}: {reason}", file=sys.stderr)
    mode_name = "staged" if args.staged else ("push" if args.push else "tree")
    print(HINT.format(fix=HINT_FIX[mode_name]), file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
