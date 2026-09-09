#!/usr/bin/env python3
"""Generate (and re-verify) the issue #57 name bank tables.

The bank is 872 hand-listed strings. A transposition that duplicates one entry
while dropping another keeps every per-pool count correct, so counts alone
cannot police the transcription. This script owns the transcription instead:

  --write   (default) rewrite the generated blocks in
            engine/Poseidon/Game/Guerrilla/LegendNames.cpp and
            tests/unit/engine/Poseidon/Game/Guerrilla/test_legend_names.cpp
  --check   parse both generated blocks back out and compare them to the
            committed fixture, exit 1 on any drift

The source of truth is tests/fixtures/legend-names/issue57-names.json, the
issue #57 attachment committed verbatim (pure ASCII, no byte >= 0x80).

--check parses the C++ rather than diffing text, so clang-format reflowing the
generated block does not make it red.
"""

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = ROOT / "tests" / "fixtures" / "legend-names" / "issue57-names.json"
SOURCE = ROOT / "engine" / "Poseidon" / "Game" / "Guerrilla" / "LegendNames.cpp"
TEST = ROOT / "tests" / "unit" / "engine" / "Poseidon" / "Game" / "Guerrilla" / "test_legend_names.cpp"

TABLE_BEGIN = "// BEGIN GENERATED NAME TABLES"
TABLE_END = "// END GENERATED NAME TABLES"
SUM_BEGIN = "// BEGIN GENERATED BANK CHECKSUMS"
SUM_END = "// END GENERATED BANK CHECKSUMS"

FNV_OFFSET = 0xCBF29CE484222325
FNV_PRIME = 0x100000001B3
MASK = 0xFFFFFFFFFFFFFFFF


def fnv1a_list(strings):
    """FNV-1a 64 over every string in order, each followed by a newline.

    Order-sensitive by construction, so a swap of two entries changes it.
    The unit test recomputes this over the COMPILED table with the identical
    loop, which is what turns a transposition into a red test.
    """
    h = FNV_OFFSET
    for s in strings:
        for b in s.encode("ascii") + b"\n":
            h = ((h ^ b) * FNV_PRIME) & MASK
    return h


def load_bank():
    raw = FIXTURE.read_bytes()
    if max(raw) >= 0x80:
        sys.exit("fixture is not pure ASCII")
    data = json.loads(raw.decode("ascii"))
    good = data["international_good"]
    evil = data["western_evil"]
    pools = []
    for region, entry in good["names"].items():
        pools.append((region, entry["first"], entry["last"]))
    for region, entry in evil["names"].items():
        pools.append((region, entry["first"], entry["last"]))
    banks = [
        ("Friendly", good["prefix"], good["describer"], good["title"]),
        ("Hostile", evil["prefix"], evil["describer"], evil["title"]),
    ]
    for region, first, last in pools:
        if not re.fullmatch(r"[a-z][a-z_]*", region):
            sys.exit("bad region token %r" % region)
        for group in (first, last):
            check_strings(group, region)
    for name, prefix, describer, title in banks:
        for group in (prefix, describer, title):
            check_strings(group, name)
    return pools, banks


def check_strings(strings, where):
    seen = set()
    for s in strings:
        if not s or s != s.strip():
            sys.exit("empty or padded string in %s: %r" % (where, s))
        if any(ord(c) >= 0x80 for c in s):
            sys.exit("non-ASCII in %s: %r" % (where, s))
        if '"' in s or "\\" in s or "--" in s:
            sys.exit("unquotable string in %s: %r" % (where, s))
        if s[0] in "@$":
            sys.exit("leading sigil in %s: %r" % (where, s))
        if s in seen:
            sys.exit("duplicate inside %s: %r" % (where, s))
        seen.add(s)


def c_array(name, strings):
    body = ", ".join('"%s"' % s for s in strings)
    return "static const char* const %s[] = {%s};\n" % (name, body)


def render_tables(pools, banks):
    out = []
    for region, first, last in pools:
        out.append(c_array("kFirst_" + region, first))
        out.append(c_array("kLast_" + region, last))
        out.append("\n")
    for name, prefix, describer, title in banks:
        out.append(c_array("k%sPrefix" % name, prefix))
        out.append(c_array("k%sDescriber" % name, describer))
        out.append(c_array("k%sTitle" % name, title))
        out.append("\n")
    out.append("// Counts come from the arrays themselves: a hand-written count\n")
    out.append("// could disagree with its table, sizeof cannot.\n")
    out.append("#define UD_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))\n")
    out.append("#define UD_POOL_ROW(r) {#r, kFirst_##r, UD_COUNT(kFirst_##r), kLast_##r, UD_COUNT(kLast_##r)}\n")
    out.append("const NamePool kNamePools[] = {\n")
    for region, _first, _last in pools:
        out.append("    UD_POOL_ROW(%s),\n" % region)
    out.append("};\n")
    out.append("const NicknameBank kBanks[2] = {\n")
    for name, _p, _d, _t in banks:
        out.append(
            "    {k%(n)sPrefix, UD_COUNT(k%(n)sPrefix), k%(n)sDescriber, UD_COUNT(k%(n)sDescriber),"
            " k%(n)sTitle, UD_COUNT(k%(n)sTitle)},\n" % {"n": name}
        )
    out.append("};\n")
    out.append("#undef UD_POOL_ROW\n")
    out.append("#undef UD_COUNT\n")
    return "".join(out)


def render_sums(pools, banks):
    out = []
    out.append("// Per-pool FNV-1a 64 checksums of the issue #57 fixture, in order, plus the\n")
    out.append("// head and tail of every list. Regenerate with tools/legend-names/gen_legend_names.py.\n")
    out.append("struct ExpectedPool\n")
    out.append("{\n")
    out.append("    const char* region;\n")
    out.append("    int nFirst, nLast;\n")
    out.append("    unsigned long long firstSum, lastSum;\n")
    out.append("    const char* firstHead;\n")
    out.append("    const char* firstTail;\n")
    out.append("    const char* lastHead;\n")
    out.append("    const char* lastTail;\n")
    out.append("};\n")
    out.append("const ExpectedPool kExpectedPools[] = {\n")
    for region, first, last in pools:
        out.append(
            '    {"%s", %d, %d, 0x%016XULL, 0x%016XULL, "%s", "%s", "%s", "%s"},\n'
            % (
                region,
                len(first),
                len(last),
                fnv1a_list(first),
                fnv1a_list(last),
                first[0],
                first[-1],
                last[0],
                last[-1],
            )
        )
    out.append("};\n")
    out.append("struct ExpectedBank\n")
    out.append("{\n")
    out.append("    int nPrefix, nDescriber, nTitle;\n")
    out.append("    unsigned long long prefixSum, describerSum, titleSum;\n")
    out.append("};\n")
    out.append("const ExpectedBank kExpectedBanks[2] = {\n")
    for _name, prefix, describer, title in banks:
        out.append(
            "    {%d, %d, %d, 0x%016XULL, 0x%016XULL, 0x%016XULL},\n"
            % (
                len(prefix),
                len(describer),
                len(title),
                fnv1a_list(prefix),
                fnv1a_list(describer),
                fnv1a_list(title),
            )
        )
    out.append("};\n")
    return "".join(out)


def splice(path, begin, end, body):
    text = path.read_text(encoding="ascii")
    i = text.index(begin) + len(begin)
    j = text.index(end)
    path.write_text(text[:i] + "\n" + body + text[j:], encoding="ascii")


def normalize(text):
    return re.sub(r"\s+", " ", text).strip()


def section(path, begin, end):
    text = path.read_text(encoding="ascii")
    return text[text.index(begin) + len(begin) : text.index(end)]


def parse_source_tables(text):
    arrays = {}
    for match in re.finditer(r"static const char\* const (\w+)\[\]\s*=\s*\{(.*?)\};", text, re.S):
        arrays[match.group(1)] = re.findall(r'"([^"]*)"', match.group(2))
    # the trailing comma keeps the #define itself out of the row list
    rows = re.findall(r"UD_POOL_ROW\((\w+)\),", text)
    return arrays, rows


def check(pools, banks):
    problems = []
    arrays, rows = parse_source_tables(section(SOURCE, TABLE_BEGIN, TABLE_END))
    if rows != [region for region, _f, _l in pools]:
        problems.append("kNamePools rows disagree with the fixture: %s" % rows)
    for region, first, last in pools:
        if arrays.get("kFirst_" + region) != first:
            problems.append("kFirst_%s differs from the fixture" % region)
        if arrays.get("kLast_" + region) != last:
            problems.append("kLast_%s differs from the fixture" % region)
    for name, prefix, describer, title in banks:
        for suffix, want in (("Prefix", prefix), ("Describer", describer), ("Title", title)):
            if arrays.get("k%s%s" % (name, suffix)) != want:
                problems.append("k%s%s differs from the fixture" % (name, suffix))
    # whitespace-normalised, because clang-format reflows the generated block
    if normalize(section(TEST, SUM_BEGIN, SUM_END)) != normalize(render_sums(pools, banks)):
        problems.append("the generated checksum block in test_legend_names.cpp is stale")
    return problems


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="verify instead of rewriting")
    args = parser.parse_args()
    pools, banks = load_bank()
    if len(pools) != 33:
        sys.exit("expected 33 pools, fixture has %d" % len(pools))
    if sum(len(f) for _r, f, _l in pools) != 396 or sum(len(l) for _r, _f, l in pools) != 396:
        sys.exit("expected 396 first and 396 last names")
    if args.check:
        problems = check(pools, banks)
        for line in problems:
            print(line)
        return 1 if problems else 0
    splice(SOURCE, TABLE_BEGIN, TABLE_END, render_tables(pools, banks))
    splice(TEST, SUM_BEGIN, SUM_END, render_sums(pools, banks))
    print("wrote %d pools and %d banks" % (len(pools), len(banks)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
