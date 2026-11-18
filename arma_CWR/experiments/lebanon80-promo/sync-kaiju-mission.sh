#!/bin/bash
# Builds PromoKaiju.Lebanon80: PromoShoot.Lebanon80 plus the kaiju script set (kj\ from
# experiments/kaiju/integration, copied, never edited at its source) and the three kaiju addons in addOns[].
# sync_kaiju_patch.py then patches the copies: the addons in mission.sqm, and the PROMO_NOSHAKE guard in
# kj\roar\shake.sqs (no camera tremor on film). Rerun after editing PromoShoot.Lebanon80 or kj\. The output is
# generated (gitignored); only this script and the patch are tracked. The kaiju takes also need the mod
# folder @kj_dedushka inside the shoot shadow: build it with
# experiments/kaiju/integration/addon/build-kj-dedushka.ps1 (-ModDir <shoot shadow>\@kj_dedushka).
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/PromoShoot.Lebanon80"; DST="$HERE/PromoKaiju.Lebanon80"
rm -rf "$DST"; mkdir -p "$DST"
cp "$SRC"/*.sqs "$SRC"/*.sqf "$SRC"/description.ext "$DST"/
cp -r "$HERE/../kaiju/integration/kj" "$DST/kj"
python "$HERE/sync_kaiju_patch.py" "$SRC/mission.sqm" "$DST/mission.sqm" "$DST/kj/roar/shake.sqs" || exit 1
echo "PromoKaiju.Lebanon80 ready"
