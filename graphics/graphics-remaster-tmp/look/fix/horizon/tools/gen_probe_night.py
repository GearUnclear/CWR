"""Probe (not part of the patch): town_night and ffh through gen_look_director's shot rhythm."""
import sys
import gen_look_director as g
g.VIEWS = {"Abel": [g.t("town_night"), g.t("ffh")]}
sys.argv = [sys.argv[0], "--modes", "overhaul=remastered", "--tag", sys.argv[1] if len(sys.argv) > 1 else "probe_night"]
g.main()
