# Internal roadmap

Ground truth for what is done, what is in flight, and what to work on next.

## Files

| File | Role | Tracked |
|---|---|---|
| `roadmap.yaml` | **Source of truth.** Machine-parsable item list: status, phase, dependencies, GH issue links, commits. Edit this. | yes |
| `generate.py` | Validator + renderer. Reads the YAML, git history, and GitHub issues; writes the HTML and JSON outputs. | yes |
| `index.html` | Human-readable roadmap: progress, "work on next", dependency graph, GH sync table. | no (generated) |
| `status.json` | Machine-readable summary for other tooling (counts, ready items, warnings). | no (generated) |
| `gh-issues-cache.json` | Last successful `gh` fetch, used offline. | no (generated) |

## Regeneration

Every commit regenerates the roadmap via `.githooks/post-commit`. One-time setup per clone:

```
git config core.hooksPath .githooks
```

Manual refresh any time:

```
python roadmap/generate.py
```

Requires Python 3 with PyYAML, and the `gh` CLI for live issue sync (falls back
to the cache, then to skipping the sync, when offline).

## GitHub issue sync

Issues live on the fork **GearUnclear/CWR**. `gh` resolves this repo to the
upstream Bohemia repo by default, so all tooling passes `-R GearUnclear/CWR`
explicitly. The generator warns (stderr, HTML, and `status.json`) when:

- a GitHub issue is not linked from any roadmap item (`gh_issues:` list), or
- a roadmap item references an issue number GitHub does not know about.

## Editing rules

- One item per meaningful chunk of work. Keep ids stable (kebab-case).
- `status`: `done` | `in-progress` | `planned` | `blocked`.
- `depends_on` lists item ids that must be **done** before this item can start.
  The generator rejects cycles and flags done-items resting on not-done deps.
- `gh_issues` links the item to fork issue numbers; every issue must be linked
  somewhere (create a `planned` item for a new issue, or link it to an existing
  one).
- `commits` lists landing commits (short hashes) as evidence for `done` work.
- `log` is a free-form bullet history: decisions, problems hit, dead ends.
  This is where "issues we faced" live so they are not relearned.
