#!/usr/bin/env python3
"""Uslu dur! internal roadmap generator.

Reads roadmap/roadmap.yaml (the machine-parsable source of truth), enriches it
with git history and GitHub issue state from GearUnclear/CWR, validates the
dependency graph, and writes:

  roadmap/index.html   human-readable roadmap (untracked, regenerated per commit)
  roadmap/status.json  machine-readable summary for other tooling

Run manually:      python roadmap/generate.py
Run by the hook:   .githooks/post-commit (installed via: git config core.hooksPath .githooks)

Exit code is 0 even on validation warnings (a broken roadmap must never block a
commit); warnings are printed to stderr and rendered prominently in the HTML.
Exit code 1 only when roadmap.yaml itself is unreadable.
"""

import html
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

import yaml

ROOT = Path(__file__).resolve().parent.parent
RM_DIR = ROOT / "roadmap"
YAML_PATH = RM_DIR / "roadmap.yaml"
HTML_PATH = RM_DIR / "index.html"
STATUS_PATH = RM_DIR / "status.json"
ISSUE_CACHE = RM_DIR / "gh-issues-cache.json"

GH_REPO = "GearUnclear/CWR"  # gh defaults to the upstream Bohemia repo; always pass -R
STATUSES = ("done", "in-progress", "planned", "blocked")
STATUS_LABEL = {
    "done": "DONE",
    "in-progress": "IN PROGRESS",
    "planned": "PLANNED",
    "blocked": "BLOCKED",
}


def run(cmd, timeout=20):
    """Run a command, return stdout or None on any failure."""
    try:
        p = subprocess.run(
            cmd, capture_output=True, text=True, timeout=timeout,
            cwd=str(ROOT), encoding="utf-8", errors="replace",
        )
        return p.stdout if p.returncode == 0 else None
    except (OSError, subprocess.TimeoutExpired):
        return None


# ---------------------------------------------------------------- data intake

def load_roadmap():
    with open(YAML_PATH, encoding="utf-8") as f:
        return yaml.safe_load(f)


def git_facts():
    facts = {"head": None, "branch": None, "commits": {}, "count": 0, "dirty_files": 0}
    out = run(["git", "rev-parse", "--short", "HEAD"])
    if out:
        facts["head"] = out.strip()
    out = run(["git", "rev-parse", "--abbrev-ref", "HEAD"])
    if out:
        facts["branch"] = out.strip()
    out = run(["git", "log", "--pretty=format:%h%x09%ad%x09%s", "--date=short"])
    if out:
        lines = [l for l in out.splitlines() if l.strip()]
        facts["count"] = len(lines)
        for line in lines:
            parts = line.split("\t", 2)
            if len(parts) == 3:
                facts["commits"][parts[0]] = {"date": parts[1], "subject": parts[2]}
    out = run(["git", "status", "--porcelain"])
    if out is not None:
        facts["dirty_files"] = len([l for l in out.splitlines() if l.strip()])
    return facts


def fetch_issues():
    """Live gh query with cache fallback. Returns (issues, source)."""
    out = run(
        ["gh", "issue", "list", "-R", GH_REPO, "--state", "all", "--limit", "200",
         "--json", "number,title,state,url,closedAt"],
        timeout=25,
    )
    if out:
        try:
            issues = json.loads(out)
            payload = {"fetched_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
                       "issues": issues}
            ISSUE_CACHE.write_text(json.dumps(payload, indent=2), encoding="utf-8")
            return issues, "live"
        except json.JSONDecodeError:
            pass
    if ISSUE_CACHE.exists():
        try:
            payload = json.loads(ISSUE_CACHE.read_text(encoding="utf-8"))
            return payload.get("issues", []), "cache (%s)" % payload.get("fetched_at", "?")
        except json.JSONDecodeError:
            pass
    return [], "unavailable"


# ---------------------------------------------------------------- validation

def validate(items, issues, git):
    warnings = []
    ids = [it["id"] for it in items]
    dupes = {i for i in ids if ids.count(i) > 1}
    for d in sorted(dupes):
        warnings.append(f"duplicate item id: {d}")
    idset = set(ids)

    for it in items:
        if it.get("status") not in STATUSES:
            warnings.append(f"{it['id']}: bad status {it.get('status')!r} (want one of {STATUSES})")
        for dep in it.get("depends_on", []):
            if dep not in idset:
                warnings.append(f"{it['id']}: depends_on unknown item {dep!r}")
        for c in it.get("commits", []):
            if git["commits"] and not any(k.startswith(c) or c.startswith(k) for k in git["commits"]):
                warnings.append(f"{it['id']}: commit {c} not found in git log")

    # cycle check via DFS
    graph = {it["id"]: [d for d in it.get("depends_on", []) if d in idset] for it in items}
    WHITE, GRAY, BLACK = 0, 1, 2
    color = {k: WHITE for k in graph}

    def dfs(node, stack):
        color[node] = GRAY
        for nxt in graph[node]:
            if color[nxt] == GRAY:
                cyc = stack[stack.index(nxt):] + [nxt] if nxt in stack else [node, nxt]
                warnings.append("dependency cycle: " + " -> ".join(cyc))
            elif color[nxt] == WHITE:
                dfs(nxt, stack + [nxt])
        color[node] = BLACK

    for k in graph:
        if color[k] == WHITE:
            dfs(k, [k])

    # done items must not depend on not-done items
    by_id = {it["id"]: it for it in items}
    for it in items:
        if it.get("status") == "done":
            for dep in it.get("depends_on", []):
                if dep in by_id and by_id[dep].get("status") != "done":
                    warnings.append(f"{it['id']} is done but depends on {dep} which is {by_id[dep].get('status')}")

    # GH issue sync: every issue must be linked from at least one roadmap item
    linked = set()
    for it in items:
        linked.update(int(n) for n in it.get("gh_issues", []))
    unlinked = [iss for iss in issues if int(iss["number"]) not in linked]
    for iss in unlinked:
        warnings.append(
            f"GH issue #{iss['number']} ({iss['state']}) not linked to any roadmap item: {iss['title']}")
    # and the reverse: items referencing issue numbers gh does not know about
    known = {int(iss["number"]) for iss in issues}
    if known:
        for it in items:
            for n in it.get("gh_issues", []):
                if int(n) not in known:
                    warnings.append(f"{it['id']}: references GH issue #{n} which was not returned by gh")

    return warnings, unlinked


def compute_ready(items):
    """Planned items whose dependencies are all done: candidates for 'work on next'."""
    by_id = {it["id"]: it for it in items}
    ready = []
    for it in items:
        if it.get("status") != "planned":
            continue
        deps = it.get("depends_on", [])
        if all(by_id.get(d, {}).get("status") == "done" for d in deps):
            ready.append(it)
    ready.sort(key=lambda it: (it.get("priority", 99), it["id"]))
    return ready


# ---------------------------------------------------------------- svg dep graph

def dep_graph_svg(items):
    """Small layered DAG. Layer = longest path from a root; edges drawn as beziers."""
    idset = {it["id"] for it in items}
    deps = {it["id"]: [d for d in it.get("depends_on", []) if d in idset] for it in items}
    memo = {}

    def layer(node, seen=()):
        if node in memo:
            return memo[node]
        if node in seen:  # cycle guard; validation already warned
            return 0
        val = 0 if not deps[node] else 1 + max(layer(d, seen + (node,)) for d in deps[node])
        memo[node] = val
        return val

    layers = {}
    for it in items:
        layers.setdefault(layer(it["id"]), []).append(it)
    ncols = len(layers)
    if ncols == 0:
        return ""

    colw, rowh, bw, bh, pad = 240, 64, 210, 40, 16
    height = max(len(v) for v in layers.values()) * rowh + pad * 2
    width = ncols * colw + pad
    pos = {}
    for col in sorted(layers):
        col_items = sorted(layers[col], key=lambda it: it.get("phase", ""))
        for row, it in enumerate(col_items):
            pos[it["id"]] = (pad + col * colw, pad + row * rowh)

    def esc(s):
        return html.escape(s, quote=True)

    parts = [f'<svg viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg" '
             f'role="img" aria-label="dependency graph" style="min-width:{width}px">']
    # edges first
    for it in items:
        x2, y2 = pos[it["id"]]
        for d in deps[it["id"]]:
            x1, y1 = pos[d]
            sx, sy = x1 + bw, y1 + bh / 2
            ex, ey = x2, y2 + bh / 2
            mx = (sx + ex) / 2
            parts.append(f'<path d="M{sx:.0f},{sy:.0f} C{mx:.0f},{sy:.0f} {mx:.0f},{ey:.0f} '
                         f'{ex:.0f},{ey:.0f}" class="edge"/>')
    for it in items:
        x, y = pos[it["id"]]
        st = it.get("status", "planned")
        title = it.get("title", it["id"])
        label = title if len(title) <= 30 else title[:29] + "…"
        parts.append(
            f'<g class="node st-{esc(st)}"><a href="#item-{esc(it["id"])}">'
            f'<rect x="{x}" y="{y}" width="{bw}" height="{bh}" rx="4"/>'
            f'<text x="{x + 10}" y="{y + 17}" class="nlabel">{esc(label)}</text>'
            f'<text x="{x + 10}" y="{y + 31}" class="nstatus">{esc(STATUS_LABEL[st] if st in STATUS_LABEL else st)}</text>'
            f'</a></g>')
    parts.append("</svg>")
    return "".join(parts)


# ---------------------------------------------------------------- html output

CSS = """
:root { color-scheme: dark; }
* { box-sizing: border-box; margin: 0; }
body { background:#111512; color:#cfd6cd; font:15px/1.55 "Segoe UI",system-ui,sans-serif;
       max-width:1080px; margin:0 auto; padding:28px 20px 80px; }
code, .mono { font-family:Consolas,ui-monospace,monospace; font-size:.92em; }
a { color:#8fb996; }
h1 { font-size:1.5rem; color:#e6ead9; letter-spacing:.02em; }
h1 .brand { color:#c8a24b; }
h2 { font-size:1.05rem; color:#e6ead9; margin:34px 0 10px; text-transform:uppercase;
     letter-spacing:.08em; border-bottom:1px solid #2c352c; padding-bottom:6px; }
.sub { color:#7d877c; font-size:.88rem; margin-top:4px; }
.warnbox { border:1px solid #8a5a2b; background:#241a10; padding:12px 16px; margin:22px 0;
           border-radius:4px; }
.warnbox h2 { margin:0 0 8px; border:0; color:#e0a458; padding:0; }
.warnbox li { margin-left:18px; }
.okbox { border:1px solid #33502f; background:#141b13; padding:10px 16px; margin:22px 0;
         border-radius:4px; color:#9fc29b; }
.bar { display:flex; height:22px; border-radius:3px; overflow:hidden; margin:10px 0 4px;
       border:1px solid #2c352c; }
.bar div { min-width:0; }
.bar .b-done { background:#3f6b3a; } .bar .b-inprog { background:#b08a34; }
.bar .b-planned { background:#37403a; } .bar .b-blocked { background:#7c3a34; }
.legend { font-size:.82rem; color:#98a296; }
.legend span { margin-right:16px; }
.dot { display:inline-block; width:9px; height:9px; border-radius:2px; margin-right:5px; }
.phase { margin-top:8px; }
.item { border:1px solid #2c352c; border-left-width:4px; border-radius:4px; padding:12px 16px;
        margin:10px 0; background:#161b16; }
.item.st-done { border-left-color:#4d8a46; }
.item.st-in-progress { border-left-color:#d3a844; }
.item.st-planned { border-left-color:#5a665e; }
.item.st-blocked { border-left-color:#a5453c; }
.item h3 { font-size:1rem; color:#e6ead9; display:flex; gap:10px; align-items:baseline;
           flex-wrap:wrap; }
.badge { font-size:.7rem; letter-spacing:.06em; padding:2px 7px; border-radius:3px;
         font-weight:600; white-space:nowrap; }
.badge.st-done { background:#2c4429; color:#a8d6a0; }
.badge.st-in-progress { background:#4a3c17; color:#ecd08a; }
.badge.st-planned { background:#2a302c; color:#9aa79d; }
.badge.st-blocked { background:#4a221e; color:#e0968e; }
.meta { font-size:.83rem; color:#8d978b; margin-top:6px; }
.meta b { color:#aeb8ac; font-weight:600; }
.item p.desc { margin-top:8px; color:#b9c1b6; }
.item ul.log { margin:8px 0 0 18px; font-size:.88rem; color:#a3ac9f; }
.next { border:1px solid #3d5238; background:#151d14; border-radius:4px; padding:12px 16px;
        margin:10px 0; }
.next .why { color:#8d978b; font-size:.85rem; }
.graphwrap { overflow-x:auto; border:1px solid #2c352c; border-radius:4px; background:#141814;
             padding:8px; }
svg .node rect { fill:#1d231d; stroke:#3a453a; }
svg .node.st-done rect { stroke:#4d8a46; }
svg .node.st-in-progress rect { stroke:#d3a844; }
svg .node.st-blocked rect { stroke:#a5453c; }
svg .nlabel { fill:#dbe2d6; font:12px Consolas,monospace; }
svg .nstatus { fill:#87928a; font:9px Consolas,monospace; letter-spacing:.08em; }
svg .edge { fill:none; stroke:#46523f; stroke-width:1.4; }
table { border-collapse:collapse; width:100%; font-size:.9rem; }
td, th { border:1px solid #2c352c; padding:6px 10px; text-align:left; }
th { color:#aeb8ac; background:#181d18; }
tr.closed td { color:#78827a; }
footer { margin-top:44px; color:#6b756a; font-size:.8rem; border-top:1px solid #2c352c;
         padding-top:12px; }
@media (prefers-color-scheme: light) { body { background:#111512; } }
"""


def esc(s):
    return html.escape(str(s))


def issue_link(n, issues_by_no):
    iss = issues_by_no.get(int(n))
    if iss:
        cls = "closed" if iss["state"] == "CLOSED" else "open"
        return (f'<a class="{cls}" href="{esc(iss["url"])}" title="{esc(iss["title"])}">'
                f'#{n}</a>' + (" (closed)" if iss["state"] == "CLOSED" else ""))
    return f"#{n}"


def render_html(data, items, git, issues, issue_source, warnings, unlinked, ready):
    issues_by_no = {int(i["number"]): i for i in issues}
    counts = {s: 0 for s in STATUSES}
    for it in items:
        counts[it.get("status", "planned")] = counts.get(it.get("status", "planned"), 0) + 1
    total = max(len(items), 1)
    now = datetime.now().strftime("%Y-%m-%d %H:%M")

    out = [f"<!doctype html><html lang='en'><head><meta charset='utf-8'>"
           f"<meta name='viewport' content='width=device-width,initial-scale=1'>"
           f"<title>Uslu dur! roadmap</title><style>{CSS}</style></head><body>"]
    out.append(f"<h1><span class='brand'>Uslu dur!</span> internal roadmap</h1>")
    out.append(f"<p class='sub'>Generated {now} at <code>{esc(git.get('head') or '?')}</code> "
               f"on <code>{esc(git.get('branch') or '?')}</code>. "
               f"{git.get('count', 0)} commits total, {git.get('dirty_files', 0)} files "
               f"modified in the working tree. GitHub issues source: {esc(issue_source)}. "
               f"Source of truth: <code>roadmap/roadmap.yaml</code>.</p>")

    # progress
    out.append("<h2>Progress</h2>")
    out.append("<div class='bar'>")
    for s, cls in (("done", "b-done"), ("in-progress", "b-inprog"),
                   ("planned", "b-planned"), ("blocked", "b-blocked")):
        pct = 100.0 * counts.get(s, 0) / total
        if pct:
            out.append(f"<div class='{cls}' style='width:{pct:.1f}%' "
                       f"title='{STATUS_LABEL[s]}: {counts.get(s,0)}'></div>")
    out.append("</div>")
    out.append("<p class='legend'>"
               + "".join(f"<span><i class='dot' style='background:{c}'></i>"
                         f"{STATUS_LABEL[s]} {counts.get(s,0)}</span>"
                         for s, c in (("done", "#3f6b3a"), ("in-progress", "#b08a34"),
                                      ("planned", "#37403a"), ("blocked", "#7c3a34")))
               + f"<span>total {len(items)}</span></p>")

    # warnings
    if warnings:
        out.append("<div class='warnbox'><h2>Sync warnings</h2><ul>")
        for w in warnings:
            out.append(f"<li>{esc(w)}</li>")
        out.append("</ul></div>")
    else:
        out.append("<div class='okbox'>Roadmap is consistent: dependency graph is acyclic and "
                   "every GitHub issue is linked to a roadmap item.</div>")

    # what to work on next
    out.append("<h2>Work on next</h2>")
    inprog = [it for it in items if it.get("status") == "in-progress"]
    if inprog:
        out.append("<p class='sub'>Currently in flight:</p>")
        for it in inprog:
            out.append(f"<div class='next'><b><a href='#item-{esc(it['id'])}'>"
                       f"{esc(it.get('title', it['id']))}</a></b>"
                       f"<div class='why'>{esc(it.get('summary', ''))}</div></div>")
    if ready:
        out.append("<p class='sub'>Unblocked and ready to start (all dependencies done):</p>")
        for it in ready:
            deps = ", ".join(it.get("depends_on", [])) or "none"
            out.append(f"<div class='next'><b><a href='#item-{esc(it['id'])}'>"
                       f"{esc(it.get('title', it['id']))}</a></b>"
                       f"<div class='why'>{esc(it.get('summary', ''))} "
                       f"<span class='mono'>[deps: {esc(deps)}]</span></div></div>")
    if not inprog and not ready:
        out.append("<p class='sub'>Nothing unblocked. Check blocked items below.</p>")

    # dependency graph
    out.append("<h2>Dependency graph</h2><div class='graphwrap'>")
    out.append(dep_graph_svg(items))
    out.append("</div><p class='legend'>Left to right: foundations to frontier. "
               "An edge means the right item builds on the left one. Click a node to jump.</p>")

    # items by phase
    phases = data.get("phases", [])
    phase_names = [p["name"] for p in phases]
    by_phase = {}
    for it in items:
        by_phase.setdefault(it.get("phase", "unphased"), []).append(it)
    ordered = [p for p in phase_names if p in by_phase] + \
              [p for p in sorted(by_phase) if p not in phase_names]
    phase_meta = {p["name"]: p for p in phases}

    for ph in ordered:
        pits = by_phase[ph]
        dcount = sum(1 for it in pits if it.get("status") == "done")
        pm = phase_meta.get(ph, {})
        out.append(f"<h2>{esc(pm.get('title', ph))} <span style='color:#7d877c;font-weight:400'>"
                   f"({dcount}/{len(pits)} done)</span></h2>")
        if pm.get("goal"):
            out.append(f"<p class='sub'>{esc(pm['goal'])}</p>")
        out.append("<div class='phase'>")
        for it in pits:
            st = it.get("status", "planned")
            out.append(f"<div class='item st-{esc(st)}' id='item-{esc(it['id'])}'>")
            out.append(f"<h3>{esc(it.get('title', it['id']))} "
                       f"<span class='badge st-{esc(st)}'>{STATUS_LABEL.get(st, st)}</span></h3>")
            meta = []
            if it.get("gh_issues"):
                meta.append("<b>GH:</b> " + ", ".join(issue_link(n, issues_by_no)
                                                      for n in it["gh_issues"]))
            if it.get("depends_on"):
                meta.append("<b>Depends on:</b> " + ", ".join(
                    f"<a href='#item-{esc(d)}'><code>{esc(d)}</code></a>"
                    for d in it["depends_on"]))
            if it.get("commits"):
                cs = []
                for c in it["commits"]:
                    info = next((v for k, v in git["commits"].items()
                                 if k.startswith(c) or c.startswith(k)), None)
                    cs.append(f"<code title='{esc(info['subject']) if info else ''}'>{esc(c)}</code>"
                              + (f" ({info['date']})" if info else ""))
                meta.append("<b>Commits:</b> " + ", ".join(cs))
            if meta:
                out.append(f"<div class='meta'>{' &nbsp;|&nbsp; '.join(meta)}</div>")
            if it.get("summary"):
                out.append(f"<p class='desc'>{esc(it['summary'])}</p>")
            if it.get("log"):
                out.append("<ul class='log'>")
                for entry in it["log"]:
                    out.append(f"<li>{esc(entry)}</li>")
                out.append("</ul>")
            out.append("</div>")
        out.append("</div>")

    # gh issue table
    out.append("<h2>GitHub issues (GearUnclear/CWR)</h2>")
    if issues:
        linked_map = {}
        for it in items:
            for n in it.get("gh_issues", []):
                linked_map.setdefault(int(n), []).append(it["id"])
        out.append("<table><tr><th>#</th><th>State</th><th>Title</th>"
                   "<th>Roadmap items</th></tr>")
        for iss in sorted(issues, key=lambda i: int(i["number"])):
            n = int(iss["number"])
            links = linked_map.get(n)
            cell = ", ".join(f"<a href='#item-{esc(i)}'><code>{esc(i)}</code></a>"
                             for i in links) if links else "<b style='color:#e0a458'>UNLINKED</b>"
            out.append(f"<tr class='{'closed' if iss['state']=='CLOSED' else ''}'>"
                       f"<td><a href='{esc(iss['url'])}'>#{n}</a></td>"
                       f"<td>{esc(iss['state'])}</td><td>{esc(iss['title'])}</td>"
                       f"<td>{cell}</td></tr>")
        out.append("</table>")
    else:
        out.append("<p class='sub'>gh unavailable and no cache; issue sync skipped this run.</p>")

    out.append("<footer>Machine-readable versions: <code>roadmap/roadmap.yaml</code> "
               "(source of truth, edit this) and <code>roadmap/status.json</code> (generated). "
               "Regenerated automatically by <code>.githooks/post-commit</code>; run "
               "<code>python roadmap/generate.py</code> to refresh by hand.</footer>")
    out.append("</body></html>")
    return "".join(out)


# ---------------------------------------------------------------- main

def main():
    try:
        data = load_roadmap()
    except Exception as e:  # unreadable source of truth is the one fatal case
        print(f"roadmap: cannot read {YAML_PATH}: {e}", file=sys.stderr)
        return 1

    items = data.get("items", [])
    git = git_facts()
    issues, issue_source = fetch_issues()
    warnings, unlinked = validate(items, issues, git)
    ready = compute_ready(items)

    html_text = render_html(data, items, git, issues, issue_source, warnings, unlinked, ready)
    HTML_PATH.write_text(html_text, encoding="utf-8")

    counts = {s: sum(1 for it in items if it.get("status") == s) for s in STATUSES}
    STATUS_PATH.write_text(json.dumps({
        "generated_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "head": git.get("head"),
        "branch": git.get("branch"),
        "counts": counts,
        "total_items": len(items),
        "in_progress": [it["id"] for it in items if it.get("status") == "in-progress"],
        "ready_next": [it["id"] for it in ready],
        "unlinked_gh_issues": [int(i["number"]) for i in unlinked],
        "warnings": warnings,
        "issue_source": issue_source,
    }, indent=2), encoding="utf-8")

    for w in warnings:
        print(f"roadmap warning: {w}", file=sys.stderr)
    print(f"roadmap: {len(items)} items ({counts['done']} done, "
          f"{counts['in-progress']} in progress), {len(warnings)} warnings "
          f"-> {HTML_PATH.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
