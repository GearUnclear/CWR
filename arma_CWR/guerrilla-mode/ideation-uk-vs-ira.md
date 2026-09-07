# UK vs IRA mod — candidate mods & Guerrilla Mode fit

Ideation note, not a spec. Scoping out whether an **"Operation Banner"-era
Northern Ireland** scenario (British Army / SAS occupier vs. Provisional IRA
resistance) can be assembled from surviving OFP/CWA community content, and how
it would plug into Guerrilla Mode's island/faction swap system the same way
`@LoBo` does.

This is the natural "second pack" test case: the swap machinery was built and
proven on `@LoBo` (Sinai island, IDF occupier, PLO/Hizballah resistance). A
UK-vs-IRA pack would exercise the same seams — island pack + `CfgGuerrillaFactions`
block — with a completely different asset family.

> License/reminder that shapes everything below: like `@LoBo`, all of these mods
> are **APL-SA-style third-party game data**. The pbo bytes are never committed
> to this GPL repo. What gets committed is a **regeneration/install script** that
> fetches or repairs the pack, so it can be rebuilt after a reinstall. See the
> repo-root CLAUDE.md `@LoBo` precedent.

---

## Verdict (short)

- **There is exactly one mod that actually contains a playable IRA faction:**
  **Operation Blue Rose — "Case Jonathan" (2023)**. It is the anchor.
- **St. Patrick's Day** is the only other IRA-themed package, but Blue Rose has
  **already absorbed its SAS** (see below), so Blue Rose is both the most
  complete *and* the most current source.
- The **island is the real gap**, not the factions. The 2026-09-06 hunt
  (Faguss database, OFP.info mirror, ACE/KMP island-pack rosters, BI forums)
  confirmed that no Northern Ireland or Irish island was ever made for OFP, so
  the map is a temperate stand-in by necessity. Shortlist with live links in
  "Island hunt results" below; Gaia first.
- The Falklands / P:UKF packs are **donor** material for a period British Army,
  not alternatives — Blue Rose already ships a full 1986–91 British roster.

---

## Tier 1 — the anchor

### Operation Blue Rose — "Case Jonathan" (ModDB, released Oct 3 2023) ✅ verified

A large Cold-War compilation built to sit on top of **RCWC** (Real Cold War
Crisis). Single full-version archive:

- **File:** `BLUEROSE.1.rar`, **1,000.66 MB** (1,049,273,048 bytes), MD5 `0cd34269fc881091e40cd8434dc8982c`, 546 downloads, developer TamTank.
- **Page:** https://www.moddb.com/mods/operation-blue-rose
- **Download:** https://www.moddb.com/mods/operation-blue-rose/downloads/operation-blue-rose-case-jonathan

**What it carries that's relevant here** (from the verified ModDB listing):

- **British Forces (86-91)** — PLCE + Osprey vests, SA80 family, SAS.
- **Provisional IRA** — listed as `Men (Provisional IRA) (79-91)`.
- **Land Rover 90 & 110** — `British - Cars (86-91)`.
- **Challenger 1, FV101 Scorpion, FV107 Scimitar** — period British armour.
- **Blackburn Buccaneer, AW101 Merlin** — `British - Air`; also **Westland Wessex**.
- **SAS from the St. Patrick mod** (explicitly: "SAS from St Patrick mod has been added").
- Credits line: `228, Makin, OFrP, VME, FFAA, RKSL, UKF, WSE, VTE` — i.e. it folds
  in **UKF** and **RKSL** content (the classic British addon families).

**Requirements:** RCWC, RCWC Plus, RCWC Winter Pack (all on ModDB). Launch line
`-mod=@BLUEROSE;@RCWC;@RCWC_Plus;`.

**Two caveats worth noting in the install script:**
- The author warns of a JAM-related error dialog on launch ("ignore the error …
  most likely JAM acting weird"). Cosmetic, but expect it.
- RCWC is itself a multi-GB dependency chain. The *whole* RCWC stack is only
  needed because Blue Rose is built against it; if we end up **extracting just
  the UK/IRA pbos** rather than running the full compilation (see "Approaches"
  below), the RCWC dependency may be avoidable for those specific classes.

### St. Patrick's Day (ModDB, ~252 MB) 🟡 superseded but worth mirroring

The most *directly* themed package: 1990s British soldiers, SAS, and IRA fighters,
plus an included single-player mission (play a British corporal). Survives as
`BritanIRA.zip`.

- Re-verified 2026-09-06: the package lives under ModDB's files section, not
  the mods section: https://www.moddb.com/downloads/st-patricks-day
  (`BritanIRA.zip`, 252.05 MB / 264,291,214 bytes, MD5 `2b26d851c2cac1df182d574a1cc8cb86`,
  uploaded 2025-12-04 by TAMTank, 58 downloads). Description: 90s British
  soldiers, SAS, IRA fighters and the St. Patrick's Day mission (Russian/English).
  No island inside.
- **Largely redundant** now: its SAS are already inside Blue Rose. Its residual
  value is the **standalone IRA unit classes and the bundled mission** — worth
  mirroring as a source of clean IRA class names / a reference mission if Blue
  Rose's IRA roster turns out to be thin.

---

## Tier 2 — British Army donors (not needed on their own)

These matter only if we *don't* use Blue Rose's British roster, or want extra
period-correct vehicles/weapons:

- **1982: Flashpoint in the Falklands** — full British/Argentine conversion
  (period infantry, weapons, vehicles, aircraft, terrain, missions). Good donor
  for a DPM/1980s British look and for small-arms like the L1A1 SLR, which is
  arguably *more* period-correct for early-Troubles than Blue Rose's SA80 (SA80
  only entered service ~1985).
- **Project UK Forces / P:UKF** — historically the main British addon family for
  OFP, now fragmented. Blue Rose already credits `UKF`, so the useful pieces are
  likely folded in. Verified surviving component: British Army **Land Rover 90GS**.
- **RKSL Studios** — the other classic UK vehicle/aircraft family, also credited
  inside Blue Rose.

**Note on period weapons:** for a 1970s–80s Troubles feel you want **L1A1 SLR,
Sterling SMG, Browning Hi-Power, L7 GPMG** on the British side and **AR-15/ArmaLite,
AKM, Garand, RPG-7, and IEDs** on the IRA side. Confirm what Blue Rose's IRA
roster actually fields once extracted — that determines whether we need a
Falklands/P:UKF donor pass for small arms.

---

## The island gap (the hard part)

> Update 2026-09-06: option 1 below was carried out and came back empty; see
> "Island hunt results" for the stand-in shortlist. Option 2 (Blue Rose's own
> bundled islands) is still open until the archive is extracted.

Guerrilla Mode needs a `CfgWorlds` island with towns, a road net, and enterable
buildings (see `ISLAND-PACKS.md` §1). For a Troubles campaign that ideally means
a Northern Ireland / border-county terrain — but **no surviving, readily
downloadable NI island turned up in this pass.** Options, in preference order:

1. **Hunt for an OFP-era NI/Ulster/Belfast island** on OFP.info, ofp.faguss.de,
  and the BI forums archives. This is the ideal and needs a dedicated search
  (search engines were rate-limiting; do it in a browser). Search terms:
  `OFP ireland island`, `ulster wrp`, `northern ireland addon`, `south armagh`.
2. **Use Blue Rose's bundled "new islands"** — the release notes say Case Jonathan
  adds "new islands"; inspect the pbo list after extraction to see whether any
  is a usable temperate terrain.
3. **Stand-in temperate island** — a generic green/European OFP island (e.g.
  `Eden`-style or one of the many community European terrains) re-labelled as a
  border county. Lose the authentic place names but get a playable map now.

Whichever island is chosen, it plugs in exactly like Sinai did: a
`mission/Guerrilla.<World>` template plus a `Names` block for towns.

---

## Island hunt results (2026-09-06)

Swept: the [Faguss OFP islands database](https://ofp-faguss.com/islands.php)
(all 600 records; 392 Woodland, 98 Desert, 64 Tropical, 37 Winter), the
[OFP.info 2.5 mirror](http://ofpr.info.paradoxstudio.uk/addons/islands.html)
(195 entries, downloads rehosted on files.ofpisnotdead.com), the ACE and KMP
OFP-island-pack rosters (the "most popular community islands" lists), the BI
forums and ModDB. Search terms covered Ireland/Irish/Eire/Ulster/Celt, the six
counties and their towns, Britain/England/Scotland/Wales and their regions.

**Result: no Northern Ireland or Irish island exists for OFP.** Zero name or
comment hits. The one Irish-themed OFP project (Irish Interim Troops, 2005,
BI forums) explicitly declined to build an island. The old ftp.armedassault.info
links in the Faguss database are dead; every archive below was HEAD-checked on
files.ofpisnotdead.com on 2026-09-06 (sizes from the server). Screenshots were
recovered from the Wayback Machine's copies of the OFP.info news images (the
news posts themselves were never archived).

### Tier A: looks right, standalone or light dependencies, links live

| Island | Author / year | Grid | Archive (under `files/ofpd/`) | Deps | Why it fits |
|---|---|---|---|---|---|
| **Gaia** | Phaeden, 2003 | 256 | `islands2/gaia.rar` 24.5 MB (WGL variant `islands2/wg_gaia_1.00.exe` 37 MB) | none listed | One-off houses strung along a dense lane network over green low hills: the ribbon-development pattern of rural Ulster. In the ACEIP/KMP popular rosters. Faguss rating 2. |
| **Trinity 2.0** | Buggs (Brent), 2003 | 256 | `islands2/bug_trinityhigh.zip` 1.8 MB (or `bug_trinitylow.zip`) | AGS Industry 2.1, AGS Harbour 1.3, Baracken 1.1 (all in `unofaddons2/`) | Misty green upland, lake, compact villages, conifer blocks. Faguss rating 3, the best-rated of the shortlist; in ACEIP/KMP. |
| **Skye** | Waterman, 2002-2004 | 256 | `islands2/SkyeV2.zip` 6.8 MB, `islands2/SkyeV3.rar` 0.5 MB, `islands/Skye3.zip` 0.2 MB | none listed | Scottish-island theme, steep green glens. Rating 2 (Skye 3); in ACEIP/KMP. Version history is messy (Beta 1 to Last Beta, then 2 and 3); check which one is complete. |
| **Brigadoon** | Ziggy, 2003 | 256 | `islands2/Brigadoon.rar` 2.9 MB | none listed | Misty moorland with scattered pines, Highland look. Rating 2. Settlement density unknown from the one surviving shot. |

### Tier B: right structure, wrong era or flavour

- **Normandy v2.1** (Jean Christophe, 2004, 256, `islands2/jc_normandy2.1.rar`
  12.8 MB, standalone, rating 2.5): bocage hedgerow fields and stone farmhouses,
  structurally the closest thing to South Armagh, but a WW2 object set. Same
  family: **WWII Normandy** (Linker Split, 2006, rating 3, grass and new
  buildings, `islands2/LinerSplitWWIImap.rar` 100 MB) and **I44 Bocage** (2007,
  needs the Invasion 1944 mod).
- **Farmland Islands 1 / 2** (Mig, 2005 / 2007, 128 grid = 6.4 km,
  `mods/FMLpack_v1.0.rar` 12.4 MB / `unofaddons2/Farmland_v2.0.rar` 13.3 MB):
  rural farmland with a stone Gothic church and a civilian pack, but a quarter
  of Everon's area; too small for a campaign map.
- **PMC Rugen** (Snake Man, 2005, 512 grid = 25.6 km; needs AGS Industrial 3.0,
  AGS Harbor 1.3, MapFact Baracken 1.5): "Everon and Nogova combined", cities,
  villages, airbase, two army bases; German flavour. The OFP release is not on
  the mirror (only the `PMC_Euro_04-20-04.rar` alpha); the original is on
  pmctactical.org and the ArmA port on ModDB (`pmc_rugen-1.4.7z`, 44 MB).
- **Fulda Gap** (kubi, 2008, 256, rating 2): German farmland villages with a
  baroque church; "found in 80+" pack only, no direct link located.
- **Sontonagh District** (Smiley Nick, 2005, 256; inside WGL 5.1,
  `mods/WGL5.1_Setup.exe` 168 MB): UK-authored and in the ACEIP/KMP popular
  rosters, but Faguss rates it 1 and no screenshot survived. Unverified.
- **Nevis** (OFPman, 2006, 512, 10 villages; needs Berghoff Nature Pack 3 and
  UWAR grass): mountainous "Nordic" look per the author; both 2006 download
  links are dead.

Checked and rejected on looks: Freya (Alpine town), Drago 2 and Saria (rocky
coast), Avignon (Provence), Morton (Scandinavian forest), Civil (flat plain),
Gala (Middle East), Havelte and Leusderheide (Dutch heath), Goslar and Fulda
(German).

### Stock fallbacks

- **Everon**: `Guerrilla.Eden` already exists (18 CITY zones). Baltic/Croatian
  rather than Irish, but zero install work.
- **Nogova**: `Noe.pbo` ships in the Classic install, but
  `PoseidonTools guerrilla scaffold --world Noe` today classifies all 177,224
  placed objects as roads and none as buildings, so all 30 Names towns are
  skipped and the template gets no CITY zones. `terrain objects` reads the same
  wrp's model names correctly (trees, bushes, `sil25` road pieces), so the miss
  is on the scaffold side; it needs a fix before Nogova is a usable template.

### Recommendation

Fitness-check the Tier A four with `PoseidonTools guerrilla scaffold` (unpack
each into a scratch mod folder, pass `--mod`, compare the CLASSIFIED and
SUMMARY lines: road points, building points, CITY count) before choosing. Gaia
first: it has no dependencies and its settlement pattern is the one that reads
as Ulster. Whatever wins, the anonymised "border county" framing from open
question 5 fits a stand-in island naturally.

---

## How it plugs into Guerrilla Mode

The swap machinery is faction/island-agnostic; `@LoBo` is just the first data
set. A UK/IRA pack would be **the same shape**:

**Faction half** (`FACTION-PACKS.md`): ship a `class CfgGuerrillaFactions` block.
Two classes at minimum, mirroring the LoBo IDF/PLO pair:

```cpp
class BritishArmy            // side WEST — the OCCUPIER
{
    side = "WEST";
    tiers[]      = { /* British private -> corporal-tier -> NCO-tier classes */ };
    tierThresholds[] = {3, 6};
    tiersMG[]    = { /* L7 GPMG gunner ladder */ };
    tiersAT[]    = { /* LAW 80 / Carl Gustav ladder */ };
    tiersMedic[] = { /* medic ladder, or "" role-absent marker */ };
    tiersSniper[]= { /* L42A1 / L96 ladder */ };
    // QRF vehicles: Land Rover 90/110, then armoured (Scimitar/Saracen/Saxon).
    flag = "\flags\uk.jpg";  // Classic Flags.pbo stand-in, same trick as LoBo's lebanon.jpg
};

class ProvisionalIRA         // side GUER — the RESISTANCE
{
    side = "GUER";
    tiers[]      = { /* volunteer -> ASU rifleman -> experienced volunteer */ };
    tierThresholds[] = {3, 6};
    // ... same role ladders, IED/AT via RPG-7-class ...
    flag = "\flags\ireland.jpg"; // if present in Classic Flags.pbo, else stand-in
};
```

Concrete class names get filled in **after extracting the Blue Rose (and maybe
St. Patrick) pbos** and listing the actual classes — same process that produced
`config/lobo-factions.hpp`.

**Side layout note:** LoBo precedent — occupier `IDF` is WEST, resistance `PLO`
is GUER, and the PLO/East twin pair handles the occupier/resistance side
collision. For UK/IRA the natural layout is identical (WEST occupier, GUER
resistance). Watch whether Blue Rose ships its IRA on a single side only; if it
ships an EAST-only IRA we may need a `sideTwin` analogue or to re-side, and the
engine's descriptor-resolution pass (plan 15) keeps any missing class non-fatal.

**Island half** (`ISLAND-PACKS.md`): scaffold a `Guerrilla.<World>` template once
the island is picked (`PoseidonTools.exe guerrilla scaffold --world <W> ...`).

**Install/regen script (the committed artifact):** a
`tools/ukira/install-ukira.ps1` (or `tools/bluerose/...`) that mirrors
`tools/lobo/install-lobo-factions.ps1` — downloads/locates the pack, optionally
runs `mod doctor`-style repairs if the Blue Rose pbos have scope/origin defects
(LoBo needed exactly this), and writes the `CfgGuerrillaFactions` block into the
mod's `bin\config.cpp`.

---

## Open questions / next steps

1. **Extract Blue Rose and inventory classes.** Confirm the actual British and
  IRA unit/vehicle/weapon class names, what side the IRA are on, and whether the
  IRA roster has MG/AT/medic/sniper depth (or just riflemen). This drives the
  `CfgGuerrillaFactions` block.
2. **Mirror St. Patrick's Day** (`BritanIRA.zip`, link re-verified 2026-09-06,
  see Tier 1) as a backup IRA-class source.
3. **Island: fitness-check the stand-in shortlist** (Gaia, Trinity 2.0, Skye,
  Brigadoon; see "Island hunt results"): download, unpack into a scratch mod
  folder, run `PoseidonTools guerrilla scaffold`, compare roads/buildings/CITY
  counts, pick one, then scaffold `Guerrilla.<class>`.
4. **Decide full-compile vs. extract.** Running all of Blue Rose + the RCWC
  chain is heavy; pulling just the UK/IRA-relevant pbos (plus whatever they
  hard-depend on) is likely leaner and may dodge the RCWC requirement. Determine
  dependencies during extraction.
5. **Decide the tone/setting** — abstract "border county" vs. named real places.
  An anonymized stand-in island sidesteps the sensitivity of a real Troubles map
  while keeping the mechanics.

---

## Sources

- Operation Blue Rose mod page — https://www.moddb.com/mods/operation-blue-rose
- St. Patrick's Day file page (re-verified 2026-09-06): https://www.moddb.com/downloads/st-patricks-day
- Faguss OFP islands database: https://ofp-faguss.com/islands.php
- OFP.info 2.5 mirror, islands section: http://ofpr.info.paradoxstudio.uk/addons/islands.html
- Island archives mirror: https://files.ofpisnotdead.com/files/ofpd/ (islands/, islands2/, unofaddons2/, mods/)
- PMC Rugen (PMC Tactical): https://www.pmctactical.org/ofp/island.php
- Case Jonathan file page (size/hash/credits verified) — https://www.moddb.com/mods/operation-blue-rose/downloads/operation-blue-rose-case-jonathan
- 1982: Flashpoint in the Falklands — https://www.moddb.com/mods/1982-flashpoint-in-the-falklands
- Project UK Forces (historical wiki) — OFP.info community wiki
- Guerrilla contracts — `arma_CWR/guerrilla-mode/FACTION-PACKS.md`, `ISLAND-PACKS.md`, `config/lobo-factions.hpp`
