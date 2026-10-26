# Round 3 D (the road quote, OCCUPIER-FORCES.md 6.5): the director side.
# Applies anchored replacements to OccupierForces.{hpp,cpp},
# OccupierForcesCommands.cpp and test_occupier_forces.cpp.  Fails loudly on a
# missing anchor and is idempotent (a second run changes nothing).
#
#   python tmp/of-round3d-director-hunk.py [arma_CWR root] [--check]
#
# Default root: D:\Arma_CWA\arma_CWR.  --check reports what would change and
# writes nothing.  Written by the round 3 D agent; the files it touches were
# owned by round 3 C at the time, so the orchestrator applies it.
import os
import sys

root = r'D:\Arma_CWA\arma_CWR'
check = False
for a in sys.argv[1:]:
    if a == '--check':
        check = True
    else:
        root = a
G = os.path.join(root, 'engine', 'Poseidon', 'Game', 'Guerrilla')
T = os.path.join(root, 'tests', 'unit', 'engine', 'Poseidon', 'Game', 'Guerrilla')

EDITS = {
    os.path.join(G, 'OccupierForces.hpp'): [
        ("""    float airEta = 1500.0f;        // forcesAirEta, 0..86400 s
""",
         """    float airEta = 1500.0f;        // forcesAirEta, 0..86400 s
    // the road quote (6.5, 2026-10-16): a ground leg whose road route is not
    // known (none, a detour the service does not march, or not planned this
    // tick) is quoted as the straight distance x roadFactor
    float roadFactor = 1.5f;       // forcesRoadFactor, 1..5
""",
         'float roadFactor = 1.5f;'),
    ],
    os.path.join(G, 'OccupierForces.cpp'): [
        ("""    t.airEta = ReadRanged(c, "forcesAirEta", t.airEta, 0.0f, 86400.0f);
""",
         """    t.airEta = ReadRanged(c, "forcesAirEta", t.airEta, 0.0f, 86400.0f);
    // 2026-10-16: the road quote (6.5)
    t.roadFactor = ReadRanged(c, "forcesRoadFactor", t.roadFactor, 1.0f, 5.0f);
""",
         '"forcesRoadFactor"'),
        # PlanFor and PlanAlarm both set req.offered = OfferedPackages: the
        # road factor goes right after each (two occurrences)
        ("""    req.offered = OfferedPackages;
""",
         """    req.offered = OfferedPackages;
    req.roadFactor = _tuning.roadFactor; // the road quote (6.5): a ground leg with no road route known
""",
         'req.roadFactor = _tuning.roadFactor;', 2),
        ("""        src.distance = sqrtf(dx * dx + dz * dz);
""",
         """        src.distance = sqrtf(dx * dx + dz * dz);
        // the road quote (6.5, 2026-10-16): the road route the service will
        // march, cached per pair (RoadQuoteCache, planned at most once a
        // tick); -1 = the straight line x forcesRoadFactor.  Air flies the
        // straight line.
        src.road = src.airOnly ? -1.0f : RoadQuoteCache::Instance().Length(s->pos, dest);
""",
         'src.road = src.airOnly'),
        ("""#include <Poseidon/Game/Guerrilla/RoadRoute.hpp>     // SameLandmass: ground sources share the target's land
""",
         """#include <Poseidon/Game/Guerrilla/RoadRoute.hpp>     // SameLandmass, RoadQuoteCache (the road quote)
""",
         'SameLandmass, RoadQuoteCache'),
        # the arrival-rule line: both quotes in words
        ("""                         "OccupierForces: [op {}] arrival rule ({}): the cheapest, {}, needs about {:.0f} s; {} goes, "
                         "about {:.0f} s",
                         opId, a.choice.etaRule == PackageChoice::EtaAir ? "forcesAirEta" : "forcesSlowEta",
                         ForcePackages::PackageName(a.choice.cheapestPackage), (double)a.choice.cheapestSeconds,
                         ForcePackages::PackageName(q.package), (double)q.seconds);
""",
         """                         "OccupierForces: [op {}] arrival rule ({}): the cheapest, {}, needs {}; {} goes, {}",
                         opId, a.choice.etaRule == PackageChoice::EtaAir ? "forcesAirEta" : "forcesSlowEta",
                         ForcePackages::PackageName(a.choice.cheapestPackage),
                         a.choice.cheapestIndex >= 0
                             ? (const char*)ForcePackages::LogEtaNote(a.choice.quotes[a.choice.cheapestIndex])
                             : "?",
                         ForcePackages::PackageName(q.package), (const char*)ForcePackages::EtaNote(q));
""",
         'needs {}; {} goes, {}'),
        # the four operation lines: "about N s" becomes the quote's words
        # ("road 1794 s", "line x1.5 2733 s"), kept for gmForcesEtaNotes
        ("""                     "of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
""",
         """                     "of {:.2f}, {} fighter(s) reported{}{}, {})",
""",
         '"of {:.2f}, {} fighter(s) reported{}{}, {})"'),
        ("""                     "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
""",
         """                     "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})",
""",
         '"{:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})",\n                     opId, push'),
        ("""                 "lethality {:.2f} of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
""",
         """                 "lethality {:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})",
""",
         '"lethality {:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})"'),
        ("""             "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, about {:.0f} s)",
""",
         """             "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})",
""",
         '             "{:.2f} of {:.2f}, {} fighter(s) reported{}{}, {})"'),
        ("""                     a.urgent ? ", urgent" : "", shortNote, (double)q.seconds);
""",
         """                     a.urgent ? ", urgent" : "", shortNote, (const char*)ForcePackages::LogEtaNote(q));
""",
         'shortNote, (const char*)ForcePackages::LogEtaNote(q));\n        }\n        else', 2),
        ("""                 shortNote, (double)q.seconds);
""",
         """                 shortNote, (const char*)ForcePackages::LogEtaNote(q));
""",
         '                 shortNote, (const char*)ForcePackages::LogEtaNote(q));'),
        ("""             RoundToInt(a.fighters), a.urgent ? ", urgent" : "", shortNote, (double)q.seconds);
""",
         """             RoundToInt(a.fighters), a.urgent ? ", urgent" : "", shortNote,
             (const char*)ForcePackages::LogEtaNote(q));
""",
         'shortNote,\n             (const char*)ForcePackages::LogEtaNote(q));'),
    ],
    os.path.join(G, 'OccupierForcesCommands.cpp'): [
        ("""//                                  "airEta" (0..86400 s); phase 4: "hqAssaultMin"
""",
         """//                                  "airEta" (0..86400 s); 2026-10-16:
//                                  "roadFactor" (1..5); phase 4: "hqAssaultMin"
""",
         '"roadFactor" (1..5)'),
        ("""    else if (stricmp(key, "airEta") == 0)
    {
        t.airEta = ClampF(v, 0, 86400);
    }
""",
         """    else if (stricmp(key, "airEta") == 0)
    {
        t.airEta = ClampF(v, 0, 86400);
    }
    else if (stricmp(key, "roadFactor") == 0)
    {
        t.roadFactor = ClampF(v, 1, 5);
    }
""",
         'stricmp(key, "roadFactor")'),
    ],
    os.path.join(T, 'test_occupier_forces.cpp'): [
        # the arrival-rule case pins the straight line: the unit fixture has
        # no road net, so every ground leg would be the line x 1.5
        ("""    CHECK(f.of.Tuning().airEta == Approx(1500.0f));
    FakeLauncher launcher;
""",
         """    CHECK(f.of.Tuning().airEta == Approx(1500.0f));
    // the road quote (2026-10-16): no road net in the fixture, so a ground
    // leg is the straight line x forcesRoadFactor (1.5); this case pins the
    // straight line (factor 1) to keep its distances
    CHECK(f.of.Tuning().roadFactor == Approx(1.5f));
    f.of.TuningMutable().roadFactor = 1.0f;
    FakeLauncher launcher;
""",
         'f.of.TuningMutable().roadFactor = 1.0f;'),
    ],
}


def main():
    changed = []
    for path, edits in EDITS.items():
        s = open(path, encoding='utf-8', newline='').read()
        crlf = '\r\n' in s
        s = s.replace('\r\n', '\n')
        orig = s
        for e in edits:
            old, new, marker = e[0], e[1], e[2]
            count = e[3] if len(e) > 3 else 1
            n = s.count(old)
            if old in new:
                # an insertion after the anchor: the anchor stays, the marker shows it ran
                if s.count(marker) >= count:
                    continue  # applied already
            elif n == 0 and marker in s:
                continue  # applied already
            if n != count:
                raise SystemExit('anchor count %d (want %d) in %s:\n%s' % (n, count, path, old))
            s = s.replace(old, new)
        if s != orig:
            changed.append(path)
            if not check:
                if crlf:
                    s = s.replace('\n', '\r\n')
                open(path, 'w', encoding='utf-8', newline='').write(s)
    for p in changed:
        print(('would change ' if check else 'changed ') + p)
    if not changed:
        print('nothing to change (applied already)')


main()
