# #76 seeded-base instruments (stable shape, published by the implementation agent)

`gmSeededBases` (nular) returns one row per zone the runtime base seed added
(CfgGuerrillaZones >> seedBases), in seeding order (AIRFIELDs first, then SEAPORTs):

    [name, type, [easting, northing, height], confidence, [evidence lines]]

- name: STRING, the zone name (also the gmZone / save key), e.g. "Malden Airfield", "Le Port Harbour"
- type: STRING, "AIRFIELD" or "SEAPORT" (never OUTPOST: outposts are offline-tool only)
- pos: [easting, northing, height] in getPos order (height = ground height at the centre)
- confidence: NUMBER 0..1 (seeding needs >= 0.5)
- evidence: ARRAY of STRING, one line per piece of evidence (the ILS entry, or each object "data3d\molo_beton.p3d at [8476, 3754]")

An empty array means nothing was seeded (seedBases resolved to 0, or no candidate passed).
Authored zones are never in this list. Seeded bases also appear in `gmZone` rows with the
`seeded` flag set (as seeded CITY zones do), owner = the occupier side, and are saved by name.

`seedBases` default: 1 when the template authors no AIRFIELD and no SEAPORT zone, else 0.
Abel authors both, so Abel seeds nothing at runtime (gmSeededBases is []): test Abel's
detection through `gmDetectedBases` below (authored Airfield [8030, 9905], Le Port Harbour
[8410, 3690]); the unit tests assert the same agreement.

## gmDetectedBases (nular, added so Abel's covered candidates are observable)

Detection runs at every Guerrilla load with a landscape, whatever seedBases says; seeding is
what seedBases gates. `gmDetectedBases` returns every AIRFIELD and SEAPORT candidate the
runtime detector found, seeded or not:

    [name, type, [easting, northing, height], confidence, [evidence lines], status, coveredBy]

- status: STRING, "seed" (passes every rule; seeded when seedBases is on), "covered" (an
  authored zone models it; coveredBy names that zone), "outside-region" (playRegion[]),
  "low-confidence"
- coveredBy: STRING, the authored zone's name, "" unless status is "covered"

So on Abel (default run) expect an AIRFIELD row with status "covered", coveredBy "Airfield",
and a SEAPORT row with coveredBy "Le Port Harbour", each within 300 m of the authored zone.
