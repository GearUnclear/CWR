"""One table for the gate: bench views, golden tiers, rain and golden-hour classes, tripwire
crop boxes and motion regions (PERF-PLAN 3.3, S6 Appendix A1, S6-5).

Views come from ../views.py (TIMING_*), ../gen_look_director.py (look_final views) and the
round-1 combat staging. Nothing here launches anything.

Boxes are fractions of the frame (y0, y1, x0, x1). The per-view boxes were placed by eye on
the 1080p overhaul shots in tmp\\look\\director\\final (2026-10-31); they are ESTIMATES and the
calibration agent (P0-11) re-checks them on the afdb4d60 goldens before any tripwire result
counts. A view without a box of a kind reports that tripwire as "no box", never as quiet.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib  # noqa: E402,F401
import views  # noqa: E402

WORLDS = views.WORLDS

COMBAT = dict(name="combat_fires", x=7600, z=10850, eye=5, h=0, p=-12, date=[1985, 7, 12, 23, 0], vd=1000,
              props=views.FFH_PROPS, refresh="[] call capBurn", feature="night fire + smoke")

# PERF-PLAN 3.3 "Standard views (8)".
BENCH_VIEWS = ["valley_0515", "broken_high", "sunrise_ridge", "noon_fields", "town_night_rain", "combat_fires",
               "bay_offshore", "sinai_desert_noon"]
QUICK_VIEWS = ["valley_0515", "town_night_rain", "sunrise_ridge"]
ROUND1_VIEWS = ["valley_0515", "sunrise_ridge", "town_night_rain", "combat_fires", "noon_fields", "broken_high"]
PIN0_VIEWS = {"valley_0515", "broken_high"}

# S6 A1 golden tiers. T2 = T1 + the rest of TIMING_* + four look_final views + a heavy-fog day;
# the cockpit, optics and combat P-mode stills come from their own existing tests (EXTRA_TESTS).
T1 = ["valley_0515", "broken_high", "lowsun_fp", "town_night_rain", "forest_edge_golden", "noon_fields",
      "sinai_desert_noon", "coastline_sunset"]
T2_MORE = ["sunrise_ridge", "town_night", "woods_pine2_evening", "road_flat_drive_moonless", "ffh", "bay_offshore",
           "pinewood_morn", "hill_1730", "coast_oc_up", "sea_noon", "overcast_broken", "sinai_lowsun_fp",
           "rain_day", "overcast_heavy", "noon_vista", "town_fog_night", "noon_fields_fog06"]
T2 = T1 + T2_MORE
EXTRA_TESTS = {  # T2 items shot by an existing test as-is (relative to arma_CWR); labels as those tests write them
    "tank_gunner_interior/tank_gunner_optics": "tests/rendering/captures/abel_lighting.test.sqf",
    "optics_mist_sight": "tests/rendering/captures/look_director_optics.test.sqf",
    "cockpit_fog_abel": "tests/rendering/captures/cockpit_fog_abel.test.sqf",
    "combat P-mode (ffh flash k06, flare)": "tests/rendering/captures/combat_ab.test.sqf",
}

RAIN_VIEWS = {"town_night_rain", "rain_day", "seam_b", "seam_d", "seam_day", "materials_town_night_rain", "rain_town"}


def _tables():
    t = {"Abel": list(views.TIMING_ABEL), "Eden": list(views.TIMING_EDEN), "Sinai": list(views.TIMING_SINAI)}
    t["Abel"].append(dict(COMBAT))
    try:
        import gen_look_director as gld  # noqa: E402
        for w, vs in gld.VIEWS.items():
            have = {v["name"] for v in t[w]}
            t[w] += [dict(v) for v in vs if v["name"] not in have]
    except Exception:  # the look director generator is optional here
        pass
    noon = next(v for v in t["Abel"] if v["name"] == "noon_fields")
    fog = dict(noon)
    fog.update(name="noon_fields_fog06", fog=0.6, feature="heavy-fog day: the drawn fog the AI must match")
    t["Abel"].append(fog)
    return t


TABLES = _tables()


# Short names other squad files use (RESUME.md's job example says "valley", "eden_250").
VIEW_ALIASES = {"valley": "valley_0515", "eden": "broken_high", "eden_250": "broken_high", "sunrise": "sunrise_ridge",
                "noon": "noon_fields", "town": "town_night_rain", "town_rain": "town_night_rain", "wet_town": "town_night_rain",
                "combat": "combat_fires", "bay": "bay_offshore", "sea": "bay_offshore", "sinai": "sinai_desert_noon"}


def resolve(name):
    return VIEW_ALIASES.get(name, name)


def view(name):
    name = resolve(name)
    for vs in TABLES.values():
        for v in vs:
            if v["name"] == name:
                d = dict(v)
                for k in ("note", "f10"):
                    d.pop(k, None)
                return d
    raise KeyError(f"unknown view {name}")


def world_of(name):
    name = resolve(name)
    for w, vs in TABLES.items():
        if any(v["name"] == name for v in vs):
            return w
    raise KeyError(name)


def is_rain(name, feat=None):
    """Rain views are gated perceptually until P0-6 seeds the rain path (3.3)."""
    if name in RAIN_VIEWS or "rain" in name:
        return True
    try:
        if float(view(name).get("rain", 0) or 0) > 0:
            return True
    except KeyError:
        pass
    if feat:
        f = gatelib.feat_values(feat)
        return f.get("rain", 0) > 0 or f.get("wet", 0) > 0
    return False


def is_golden_hour(name):
    try:
        d = view(name)["date"]
    except KeyError:
        return any(k in name for k in ("sunset", "sunrise", "golden", "evening", "dawn", "lowsun", "blue_hour"))
    hour = d[3] + d[4] / 60.0
    return 4.5 <= hour <= 8.5 or 16.5 <= hour <= 20.5


# Tripwire boxes, (y0, y1, x0, x1) fractions. Defaults apply to every view; per-view entries add
# or replace. Kinds: sky, ground (lit ground, also the mustard and blue-wash ground band), texture
# (Laplacian sharpness), crown (stddev of L*), hull (crushed shade: Otsu split into lit and shade
# inside the box), wet (rain rule: non-puddle wet ground; no puddle mask exists yet).
DEFAULT_BOXES = {
    "sky": [(0.02, 0.20, 0.05, 0.95)],
    "ground": [(0.80, 0.98, 0.05, 0.95)],
    "texture": [(0.82, 0.98, 0.25, 0.75)],
}
VIEW_BOXES = {
    "lowsun_fp": {"hull": [(0.38, 0.56, 0.60, 0.80)], "sky": [(0.02, 0.30, 0.30, 0.95)],
                  "ground": [(0.72, 0.98, 0.05, 0.95)], "texture": [(0.80, 0.98, 0.30, 0.70)]},
    "forest_edge_golden": {"crown": [(0.02, 0.42, 0.72, 0.97), (0.24, 0.48, 0.27, 0.55)],
                           "sky": [(0.02, 0.25, 0.02, 0.60)], "texture": [(0.80, 0.98, 0.40, 0.90)]},
    "woods_pine2_evening": {"crown": [(0.05, 0.45, 0.05, 0.95)]},
    "noon_fields": {"hull": [(0.55, 0.70, 0.555, 0.69)], "sky": [(0.02, 0.30, 0.02, 0.98)],
                    "ground": [(0.75, 0.98, 0.05, 0.95)], "texture": [(0.82, 0.98, 0.30, 0.70)]},
    "coastline_sunset": {"sky": [(0.02, 0.38, 0.02, 0.55)], "ground": [(0.75, 0.98, 0.05, 0.95)]},
    "town_night_rain": {"wet": [(0.65, 0.98, 0.02, 0.98)], "sky": [], "ground": [(0.65, 0.98, 0.02, 0.98)],
                        "texture": [(0.40, 0.55, 0.20, 0.40)]},
    "rain_day": {"wet": [(0.75, 0.98, 0.05, 0.95)]},
}


def boxes(name):
    b = {k: list(v) for k, v in DEFAULT_BOXES.items()}
    b.update({k: list(v) for k, v in VIEW_BOXES.get(name, {}).items()})
    if is_rain(name) and "wet" not in b:
        b["wet"] = list(b.get("ground", []))
    return b


# Motion regions per clip (fractions). "valley_mist" is fogcheck/fogmotion.py's dawn shot.
MOTION_REGIONS = {
    "valley_mist": {"bay": (0.56, 0.70, 0.72, 0.97), "valley": (0.54, 0.64, 0.58, 0.68),
                    "farhill": (0.45, 0.53, 0.25, 0.55), "haze": (0.47, 0.55, 0.62, 0.97),
                    "slope": (0.56, 0.68, 0.30, 0.48)},
    "default": {"upper": (0.15, 0.45, 0.10, 0.90), "centre": (0.35, 0.65, 0.30, 0.70),
                "lower": (0.60, 0.90, 0.10, 0.90)},
}


def motion_regions(clip_name):
    for k, v in MOTION_REGIONS.items():
        if k != "default" and k in (clip_name or ""):
            return dict(v)
    return dict(MOTION_REGIONS["default"])


if __name__ == "__main__":
    import json
    print(json.dumps({"bench": BENCH_VIEWS, "T1": T1, "T2": T2, "extra_tests": EXTRA_TESTS,
                      "worlds": {n: world_of(n) for n in T2}, "rain": sorted(n for n in T2 if is_rain(n)),
                      "golden_hour": sorted(n for n in T2 if is_golden_hour(n))}, indent=1))
