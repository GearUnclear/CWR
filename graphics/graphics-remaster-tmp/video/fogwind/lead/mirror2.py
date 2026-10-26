import numpy as np, mirror as m
m.N = 40
m.I, m.J = np.meshgrid(np.arange(m.N, dtype=float), np.arange(m.N, dtype=float), indexing='xy')
def truth_c(t, pan):
    return m.medium(m.I + pan * t, m.J, 0.0)
m.truth = truth_c
def run2(sc, pan):
    r = m.run(sc, pan, frames=240, warm=80, lag=2)
    tr = r['best'] - max(r['c0'], 0.0)
    return r, tr
for name, sc in [('framewide', m.scheme_framewide), ('ign', m.scheme_ign), ('white', m.scheme_white), ('fixed', m.scheme_fixed)]:
    for pan in (0.0, 0.075, 0.3):
        r, tr = run2(sc, pan)
        print(f"{name:10s} pan {pan:5.3f} pump {r['pump']:.5f} travel {tr:.3f} best {r['best']:.3f} at {r['lagbest']} c0 {r['c0']:.3f} held {r['held']:.2e}")
