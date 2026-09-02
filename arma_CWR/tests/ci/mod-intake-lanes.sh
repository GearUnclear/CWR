#!/usr/bin/env bash
#
# Mod-intake CI lane (issue #54): the three offline checks a third-party mod has
# to pass before it is worth loading, run against the repo's own GPL-clean
# fixtures so the lane needs no game data.
#
#   1. `mod doctor`     - no known archive defect in the fixture mods
#   2. `guerrilla lint`  - an intact faction pack resolves every descriptor key
#   3. `guerrilla lint`  - a broken one FAILS, which is the point of the lane
#   4. `guerrilla probe` - the intact pack's roster passes the static spawn gate
#
# Usage: tests/ci/mod-intake-lanes.sh <path to PoseidonTools[.exe]>

set -u

TOOLS="${1:-}"
if [ -z "$TOOLS" ]; then
    echo "usage: $0 <path to PoseidonTools[.exe]>" >&2
    exit 2
fi
if [ ! -x "$TOOLS" ] && [ ! -f "$TOOLS" ]; then
    echo "not found: $TOOLS" >&2
    exit 2
fi

# Repo root, from tests/ci/.
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
MINI="$ROOT/tests/fixtures/packages/mini"
FACTION="$ROOT/tests/fixtures/mods-factionpack/@udfaction"
BROKEN="$ROOT/tests/fixtures/mods-factionpack/@udbroken"

failures=0

# Run a command, compare its exit code with what the lane expects, and print the
# output only when the expectation is not met - a green lane stays quiet.
expect() {
    local want="$1"
    local name="$2"
    shift 2
    local out
    out="$("$@" 2>&1)"
    local got=$?
    if [ "$got" = "$want" ]; then
        echo "PASS $name (exit $got)"
    else
        echo "FAIL $name (exit $got, expected $want)"
        echo "$out" | sed 's/^/    /'
        failures=$((failures + 1))
    fi
}

expect 0 "mod doctor @udfaction" "$TOOLS" mod doctor "$FACTION"
expect 0 "mod doctor @udbroken" "$TOOLS" mod doctor "$BROKEN"
expect 0 "guerrilla lint @udfaction" "$TOOLS" guerrilla lint --data-dir "$MINI" --mod "$FACTION"
expect 1 "guerrilla lint @udbroken" "$TOOLS" guerrilla lint --data-dir "$MINI" --mod "$BROKEN"
expect 0 "guerrilla probe @udfaction" "$TOOLS" guerrilla probe --data-dir "$MINI" --mod "$FACTION"

if [ "$failures" -gt 0 ]; then
    echo "mod-intake lanes: $failures lane(s) failed"
    exit 1
fi
echo "mod-intake lanes: all lanes green"
exit 0
