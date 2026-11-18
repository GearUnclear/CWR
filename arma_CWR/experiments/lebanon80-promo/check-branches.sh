#!/bin/bash
# Before any shoot: is every engine branch tip already in the branch being shot? Lists, per branch, the
# commits on origin/<branch> that the current HEAD does not have. An empty list for every branch means the
# build has them all; anything listed (a fix the shoot would miss) is merged first, never rebased.
#
#   bash check-branches.sh [branch ...]      default: main graphics-remaster occupier-graphics
#
# The first shoot of the Lebanon80 promo ran on a video-production that lacked 11 graphics-remaster commits,
# the pop-in fix among them, and every clip had to be shot again.
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE" || exit 2
git fetch --quiet origin || { echo "git fetch failed"; exit 2; }
[ $# -gt 0 ] || set -- main graphics-remaster occupier-graphics
echo "HEAD $(git rev-parse --short HEAD) on $(git branch --show-current)"
missing=0
for b in "$@"; do
  if ! git rev-parse --verify --quiet "origin/$b" >/dev/null; then
    echo "  $b: no origin/$b"; missing=1; continue
  fi
  n=$(git rev-list --count "HEAD..origin/$b")
  if [ "$n" -eq 0 ]; then
    echo "  $b: in HEAD"
  else
    echo "  $b: $n commit(s) not in HEAD"; git log --oneline "HEAD..origin/$b" | head -20 | sed 's/^/      /'
    missing=1
  fi
done
exit $missing
