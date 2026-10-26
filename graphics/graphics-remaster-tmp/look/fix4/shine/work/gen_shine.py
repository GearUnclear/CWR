"""Writes look_shine_abel/eden tests: town_night_rain, seam_b, seam_d, seam_day, rain_day (the seam fixer's views)."""
import sys
sys.path.insert(0, r"D:/Arma_CWA_worktrees/look-fix-shine/arma_CWR/tests/rendering/captures")
import gen_look_director as g
tnr = g.t("town_night_rain")
g.VIEWS["Abel"] = [
    tnr,
    dict(tnr, name="seam_b", h=20, p=-2),
    dict(tnr, name="seam_d", x=7160, z=8905, h=10, p=-6),
    dict(tnr, name="seam_day", date=[1985, 7, 5, 11, 0], overcast=1.0),
]
g.VIEWS["Eden"] = [v for v in g.VIEWS["Eden"] if v["name"] == "rain_day"]
g.VIEWS["Sinai"] = []
sys.argv = ["gen", "--modes", "ovh=remastered", "--tag", "shine"]
g.main()
