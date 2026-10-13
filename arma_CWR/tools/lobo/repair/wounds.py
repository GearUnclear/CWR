"""LoBo_Pilot wounds for every pilot class, in the suit's own colour (2026-10-11.4).

A wounded pilot swaps each body-part texture for the `_d` texture its class
lists in `wounds[]` (healthy, wounded pairs; UD's WoundInfo::Load matches the
healthy name, `.pac` and `.paa` apart, against the model's face textures and
skips a pair that matches none). LoBo ships one olive wounded set: every
folder's `back_d`, `legsbk_d`, `front_d`, `armle_d`, `armri_d`, `legsfr_d`,
`legslft_d` and `legsrgt_d` are byte-identical, so a wounded pilot in a
non-olive suit turned olive. The non-olive suits get new members, written next
to the shipped files under `<part>_dr.paa` (no shipped texture is overwritten):
EAF `tex` (reddish brown) and IAF `tex2` (grey) all eight parts, IAF `tex3`
and `tex4` (the grey left arms of the 106th and 133th Sqn) `armle`.

Config, so that every face of a pilot model whose part has a `_d` texture has
a pair (each `_cap` subclass inherits its parent's model and wounds[]):

- LoBo_Egypt_Pilot, LoBo_IAF_Pilot_grey: their eight pairs name the `_dr`
  set of their suit.
- LoBo_IAF_Pilot_grey2, _grey3: they authored one left-arm pair (grey3's named
  `tex3\\armle`, which no face of its model uses), so their grey body and face
  showed no wound. Each gets a full array: the face entries, the seven IAF
  `tex2` body parts its model uses (all but `armle`) on the `tex2` `_dr` set,
  and its own `tex3` or `tex4` `armle` on that folder's `armle_dr`.
- LoBo_IAF_Pilot2 (olive): it inherited the IAF `tex` pairs, but its model's
  left arm is `tex5\\armle` (same bytes as `tex\\armle`), so the arm had no
  pair. Its own array keeps the faces and pairs, the arm pair naming `tex5`
  and the shipped `tex5\\armle_d`.
- LoBo_Syria_Pilot (olive): it inherited the EAF pairs, which match none of
  its SyAF `tex` faces. Its own array keeps the faces and pairs the eight SyAF
  `tex` faces (`back` and `legsbk` are `.pac`) with the shipped SyAF `_d` set.
- LoBo_IAF_Pilot, LoBo_Jordan_Pilot (olive, every face paired): unchanged.

A new array sits where the parent's does (after class Animations) or replaces
the authored one; its face entries are FACES, which must equal the face entries
of every authored full array (LoBo_IAF_Pilot, _grey, Egypt and Jordan), and its
pairs follow the authored order and each name a face of the class's model.
Every shipped wounded target a new pair names is a REQUIRED_MEMBER. The `lod\\`
textures of LOD 9.0 have no `_d` and stay unpaired, as in every authored array.

Method (the shipped olive `_d` set is the RJAF healthy suit plus painted blood,
DXT noise apart): per part, the RJAF healthy texture box-downscaled to the
`_d` size is the base; RJAF-only patches (name tag, arm flag, knee patch:
pixels not olive-like, dilated 2 px) are filled from IAF `tex` where that is
plain olive, else from the part's mean olive. alpha is a smoothstep of the
colour distance `_d` vs base, gated on "red" or "darker and not green",
quantised to 8 bits and blurred as Pillow's GaussianBlur(0.6) does. The blood
layer is unmixed as premul = `_d` - base * (1 - alpha), the olive excess in it
(G above 0.30 R, B above 0.25 R) is moved into R (the "red" variant) and the
layer is capped at 255 * alpha per channel, so outside the blood the result
equals the healthy suit exactly. new = healthy * (1 - alpha) + layer. Output
is DXT1 at the shipped `_d` size with the shipped `_d` level count (repair.paa).

Every source texture is read from LoBo_Pilot.pbo itself (no rule changes
them) and pinned by SHA-256, so a different LoBo install fails closed. Every
generated member is pinned too: the pipeline is deterministic (see
repair.paa), a regenerated member must equal its pin, and a present one is
verified without regenerating. Profile-neutral: plain DXT1 PAA files and
config texture names.
"""
import hashlib

from . import paa
from .rules import p, split_newline
from .scripts import _replace

ADDONS = ("lobo_pilot",)
PILOT = "lobo_pilot/config.cpp"
BS = chr(92)
PARTS = ("back", "legsbk", "front", "armle", "armri", "legsfr", "legslft", "legsrgt")


def _key(folder: str, name: str) -> str:
    return "lobo_pilot/" + folder.replace(BS, "/").lower() + "/" + name.lower()


def _names(ext_of=None) -> dict:
    """{part: healthy file name}; `ext_of` lists the parts whose healthy texture is a .pac."""
    return {part: part + (".pac" if part in (ext_of or ()) else ".paa") for part in PARTS}


# Healthy face-texture names per folder (each equals the name the model's faces use).
OLIVE_BASE = ("RJAF" + BS + "tex", _names())
OLIVE_FILL = ("IAF" + BS + "tex", _names(("back", "legsbk")))
# The shared olive wounded set, read from RJAF (byte-identical in every folder).
OLIVE_WOUNDED = "RJAF" + BS + "tex"
# folder: {part: healthy name}; each gets <part>_dr.paa in the same folder.
SUITS = {
    "EAF" + BS + "tex": _names(),
    "IAF" + BS + "tex2": _names(("back",)),
    "IAF" + BS + "tex3": {"armle": "armle.paa"},
    "IAF" + BS + "tex4": {"armle": "armle.paa"},
}
# class: suit folder; each of its eight authored pairs moves from <part>_d.paa
# to the suit's <part>_dr.paa.
CLASSES = {
    "LoBo_Egypt_Pilot": "EAF" + BS + "tex",
    "LoBo_IAF_Pilot_grey": "IAF" + BS + "tex2",
}

SOURCE_SHA256 = {  # every member the generator reads
    "lobo_pilot/rjaf/tex/back.paa": "8938aadf4a8a8464be63bf002b4084969939b8ebd279b0b938f7ccb4fc5fac51",
    "lobo_pilot/iaf/tex/back.pac": "99f132bb1384b0bd21d15796bd5572d08340b385cf668feda01adcd6a5f8ca32",
    "lobo_pilot/rjaf/tex/back_d.paa": "066982ad5dd7c8b9c087dff4cd52a276f4f5d00bb61a34309f67ce664fc54ea9",
    "lobo_pilot/rjaf/tex/legsbk.paa": "c191085694ae349560bb33bf058843adde9b83147619d2c11171dad5df1c41e0",
    "lobo_pilot/iaf/tex/legsbk.pac": "629be02d9eafda84eaef9023294e02f77a1131d49903aefa449b6a4762582b82",
    "lobo_pilot/rjaf/tex/legsbk_d.paa": "6404799741a334ca91ca6be19d869006c2a5a7b94e03dfaf0a7bc96798468a1d",
    "lobo_pilot/rjaf/tex/front.paa": "0fab6a0c55bc3c09a4213097431ea1ab9d955e1d765e92723c27ed971a84148f",
    "lobo_pilot/iaf/tex/front.paa": "bd62c320f5ee36fdef26e068b9bcdbb8e7fbbda844f99a90c4f998f8f76f71f2",
    "lobo_pilot/rjaf/tex/front_d.paa": "f31f313c4b3b164a1576dd38d67e0683c6c1c49ceccccbd847d1b3830f69819d",
    "lobo_pilot/rjaf/tex/armle.paa": "49f82e83185f4a7325ce6c58cbe1d15330ace5c81ce3c8c29a3acf01bb8e8e85",
    "lobo_pilot/iaf/tex/armle.paa": "3f9721061aab40a6c4b756863c3362d6f1084f16f07af101fae4b239b86b1008",
    "lobo_pilot/rjaf/tex/armle_d.paa": "c1418ce307018b443e9327d0dc19733dc0d287621cec6ec98bbd678b46e44659",
    "lobo_pilot/rjaf/tex/armri.paa": "c2891e36a3310a639714d56d3f2866edba5dfd0ca3c30c5f72ac53334d7361b4",
    "lobo_pilot/iaf/tex/armri.paa": "f1a3468ba801e5f786811f9f11f3f9a52f048025dc5b76c3d2aeeb85972c5b7e",
    "lobo_pilot/rjaf/tex/armri_d.paa": "c1e7ccf7533bb42b84c9f0f88c8b7f994bcb46ac121af70c866384fcecb69acc",
    "lobo_pilot/rjaf/tex/legsfr.paa": "7f692f2444906fbaee23aac158112e0058d9492e3c4583eb6475228ecd814448",
    "lobo_pilot/iaf/tex/legsfr.paa": "4765409c33e979bf92be2a42a97a3e5cfe03793652097c380241d1f5e7a96878",
    "lobo_pilot/rjaf/tex/legsfr_d.paa": "8031819cbafadc39c50c6fc0844728f252f137a8c455e85deddeb0037645d050",
    "lobo_pilot/rjaf/tex/legslft.paa": "e5adb993d5cc86d795eff3fb56deb839df993890e8e9385bb7f1850c631efdbb",
    "lobo_pilot/iaf/tex/legslft.paa": "e5adb993d5cc86d795eff3fb56deb839df993890e8e9385bb7f1850c631efdbb",
    "lobo_pilot/rjaf/tex/legslft_d.paa": "aa6b2e1d3c9d353c1c5790506c55ece0da35d7db80c08cf7ce8b9769ac4c2332",
    "lobo_pilot/rjaf/tex/legsrgt.paa": "0876d8d428b9915e92c102c557b13d45920349ded298152d31813748f55b4237",
    "lobo_pilot/iaf/tex/legsrgt.paa": "0876d8d428b9915e92c102c557b13d45920349ded298152d31813748f55b4237",
    "lobo_pilot/rjaf/tex/legsrgt_d.paa": "6b5e7743381e5293f9c0810e75a8099747dd228008111115bd5f75d0e5b49df7",
    "lobo_pilot/eaf/tex/back.paa": "fcaee335bbc049e03e1810efb14fad918f630afeb9834d6d0f3feb05c39fb4e6",
    "lobo_pilot/eaf/tex/legsbk.paa": "796bcccc07e1f43a0ce87adba77a1f476d79a421eb529660651aa57ab4c00f02",
    "lobo_pilot/eaf/tex/front.paa": "1afb654ba56bf5506714001066ce6517c8973eb57c078db010ba2d7dbef4f728",
    "lobo_pilot/eaf/tex/armle.paa": "9d74a405c7d48daabf28761d4a460d3d999dd2d65e716406291d5a93344aa688",
    "lobo_pilot/eaf/tex/armri.paa": "846a3079dfb62aab80ce2125ad4beca311340722e4330baf131e4ba82a1d6a6f",
    "lobo_pilot/eaf/tex/legsfr.paa": "c535fd1bec9f5fb6fb45286681f40c342f5cd990c2d40e48f061ca35970d7c4a",
    "lobo_pilot/eaf/tex/legslft.paa": "528c6720b852842af08770bb7298f52141fe4b087f6305f6000f504a21325def",
    "lobo_pilot/eaf/tex/legsrgt.paa": "0ee6bae4da9092de449ff4f3c8802ca81e7d204cd8cd7a7957c68e54e272e439",
    "lobo_pilot/iaf/tex2/back.pac": "7863a65d070685adc7f5c417a45c006f214934b1eef6ccfba5668f17ce2e9be7",
    "lobo_pilot/iaf/tex2/legsbk.paa": "8b5c0196dcae17dc37d4095d3b62df7b68d4e7a40ffac98a4ac9ec91d7165689",
    "lobo_pilot/iaf/tex2/front.paa": "ebbb67bd1ec96a723deeebf3e4547909ff1b14e75b1cc7386256a68a26feff1a",
    "lobo_pilot/iaf/tex2/armle.paa": "835c9da539bf95c0cefaa20df290d69b66cc524cc30691ebe3ba5f4ce87b4e45",
    "lobo_pilot/iaf/tex2/armri.paa": "fea01264b53f846dcc5c5db94672296bf59bc4c65735dc84da96d5b4cfb058a0",
    "lobo_pilot/iaf/tex2/legsfr.paa": "d5c6415589f964369ec51f23d523b96f7c724818af2a40b7d2671ace1dc817b4",
    "lobo_pilot/iaf/tex2/legslft.paa": "fd5fcc9df6228f7333192ad08b7a8fcd1603f56f7ba0338b189d4d2c8b2e71a2",
    "lobo_pilot/iaf/tex2/legsrgt.paa": "06cbaf3b6f13396acd4899f352682ec43677476adb0b9ed82e79105ca1d274bf",
    "lobo_pilot/iaf/tex3/armle.paa": "6cfcf4103ed9296c848248498af6f2e7c235d353880773e340b29ae5744529cd",
    "lobo_pilot/iaf/tex4/armle.paa": "ac73feb4f21c2d42a007a745fd338cc8a1df1b603b422ccaf11fa83a1fe60f85",
}
# added member key: (stored name, sha256)
OUTPUTS = {
    "lobo_pilot/eaf/tex/back_dr.paa": ("EAF\\tex\\back_dr.paa",
                                       "b0b8755f263e53b6b6acb4018bcbd30e45b348645252a8486caabb7817f612d2"),
    "lobo_pilot/eaf/tex/legsbk_dr.paa": ("EAF\\tex\\legsbk_dr.paa",
                                         "db4f12d3f2567df08a15fb6210c59a7217aaa311b8aafbb56e80f61673270738"),
    "lobo_pilot/eaf/tex/front_dr.paa": ("EAF\\tex\\front_dr.paa",
                                        "0d4bd28e3cc77b8d2c39ad29d332ce4901485df6ddc51f0300ef8fa5f0bb01b3"),
    "lobo_pilot/eaf/tex/armle_dr.paa": ("EAF\\tex\\armle_dr.paa",
                                        "7ca82bdc68a27ff473688b48c9600b68b8a333571d87c75a1c9f17919689ebe5"),
    "lobo_pilot/eaf/tex/armri_dr.paa": ("EAF\\tex\\armri_dr.paa",
                                        "e6a79a59f5decb9adbcbc3672f72f05f84598764ba286279bf1afbb409bab5d3"),
    "lobo_pilot/eaf/tex/legsfr_dr.paa": ("EAF\\tex\\legsfr_dr.paa",
                                         "062aa163eef49f1b9aee7ff62dec2b804fac00faf8feed80c900defd151aa7c3"),
    "lobo_pilot/eaf/tex/legslft_dr.paa": ("EAF\\tex\\legslft_dr.paa",
                                          "b04fc759afa0fac8f432cd0bd0e73ef4904cd20f7ebae4307e4186ae3249e1af"),
    "lobo_pilot/eaf/tex/legsrgt_dr.paa": ("EAF\\tex\\legsrgt_dr.paa",
                                          "866acec34cc4cea48034a0c895a66611861781d69bfbff1b39ad507bd3892ec3"),
    "lobo_pilot/iaf/tex2/back_dr.paa": ("IAF\\tex2\\back_dr.paa",
                                        "91d78d6668418ab26dde4d5c5ed6c85993228ddfe2f2d0e65820844abfd65927"),
    "lobo_pilot/iaf/tex2/legsbk_dr.paa": ("IAF\\tex2\\legsbk_dr.paa",
                                          "8ea5b3c0eb22a98a5c28d53cab2e5492c1aa1638bcfe0961ba8987c85b6c9bcb"),
    "lobo_pilot/iaf/tex2/front_dr.paa": ("IAF\\tex2\\front_dr.paa",
                                         "9e91a5f62253ec66232ebfbbc5424f5c90c2333e7e087a21d818a4f73fdd3256"),
    "lobo_pilot/iaf/tex2/armle_dr.paa": ("IAF\\tex2\\armle_dr.paa",
                                         "89a3ab4403f359b4ceef9849e6e213629538e0c34c5410d6d0628c7bbe968825"),
    "lobo_pilot/iaf/tex2/armri_dr.paa": ("IAF\\tex2\\armri_dr.paa",
                                         "534af7848058990449fec06a84a4ab204d2d54523cc443bdda19df1221605dbd"),
    "lobo_pilot/iaf/tex2/legsfr_dr.paa": ("IAF\\tex2\\legsfr_dr.paa",
                                          "1ebcf9467043768ef5a429e2b0fd88c6ab3f39cf16a3843e1ec67299baeb2f2b"),
    "lobo_pilot/iaf/tex2/legslft_dr.paa": ("IAF\\tex2\\legslft_dr.paa",
                                           "2e04967572fe419976f098bec425403ac93b24014a203957c934a177a40251ce"),
    "lobo_pilot/iaf/tex2/legsrgt_dr.paa": ("IAF\\tex2\\legsrgt_dr.paa",
                                           "050fdafb4cea2c5cab163c25282f1683e9bd2a6f82a08a650d8c28179c168398"),
    "lobo_pilot/iaf/tex3/armle_dr.paa": ("IAF\\tex3\\armle_dr.paa",
                                         "be66a3841e8fef5196a0e0ae976b7fdd5adcd460586bfe685fb827cf100c004e"),
    "lobo_pilot/iaf/tex4/armle_dr.paa": ("IAF\\tex4\\armle_dr.paa",
                                         "13ca9f1c1551d00155ac906a8db450424710b8866213462b9816c13ed5539589"),
}


def sources() -> tuple[str, ...]:
    """Every archive member the generator reads."""
    keys = []
    for part in PARTS:
        keys += [_key(OLIVE_BASE[0], OLIVE_BASE[1][part]), _key(OLIVE_FILL[0], OLIVE_FILL[1][part]),
                 _key(OLIVE_WOUNDED, part + "_d.paa")]
    for folder, names in SUITS.items():
        keys += [_key(folder, name) for name in names.values()]
    return tuple(dict.fromkeys(keys))


def outputs() -> dict:
    """{added key: stored name} for every generated member."""
    return {_key(folder, part + "_dr.paa"): folder + BS + part + "_dr.paa"
            for folder, names in SUITS.items() for part in names}


SOURCE_MEMBERS = sources()
ADDED_MEMBERS = tuple(outputs())


# --- pixel pipeline ----------------------------------------------------------

def _smooth(x: float, lo: float, hi: float) -> float:
    t = (x - lo) / (hi - lo)
    if t <= 0.0:
        return 0.0
    if t >= 1.0:
        return 1.0
    return t * t * (3.0 - 2.0 * t)


def _olive_like(r: int, g: int, b: int) -> bool:
    """Plain olive suit (not a patch, tag or highlight); luminance in thousandths, exact."""
    lum = 299 * r + 587 * g + 114 * b
    return -18 < r - g < 18 and 28000 < lum < 105000 and b < g + 8


def _dilate(mask: list, w: int, h: int, radius: int = 2) -> list:
    """Square max filter of a 0/1 mask, clamped at the edges."""
    rows = []
    for y in range(h):
        row = mask[y * w:(y + 1) * w]
        rows.append([max(row[max(0, x - radius):x + radius + 1]) for x in range(w)])
    out = [0] * (w * h)
    for x in range(w):
        col = [rows[y][x] for y in range(h)]
        for y in range(h):
            out[y * w + x] = max(col[max(0, y - radius):y + radius + 1])
    return out


# Pillow 12 GaussianBlur(0.6): three extended box passes of radius 0.0681818
# per axis in 8.24 fixed point (BoxBlur.c), edges clamped. Verified equal to
# Pillow on random and real masks.
_WW = 14763950
_FW = ((1 << 24) - _WW) // 2


def _blur_line(line: list) -> list:
    n = len(line)
    if n == 1:
        return line[:]
    out = [(line[0] * _WW + (line[0] + line[1]) * _FW + (1 << 23)) >> 24]
    out += [(line[x] * _WW + (line[x - 1] + line[x + 1]) * _FW + (1 << 23)) >> 24 for x in range(1, n - 1)]
    out.append((line[n - 1] * _WW + (line[n - 2] + line[n - 1]) * _FW + (1 << 23)) >> 24)
    return out


def _blur(values: list, w: int, h: int) -> list:
    rows = [values[y * w:(y + 1) * w] for y in range(h)]
    for _ in range(3):
        rows = [_blur_line(row) for row in rows]
    cols = [[rows[y][x] for y in range(h)] for x in range(w)]
    for _ in range(3):
        cols = [_blur_line(col) for col in cols]
    return [cols[x][y] for y in range(h) for x in range(w)]


def _fit(image: tuple, w: int, h: int) -> tuple:
    """Box-downscale `image` by halves to w x h."""
    while image[0] > w:
        image = paa.downscale(image)
    if image[0] != w or image[1] != h:
        raise ValueError(f"Cannot fit a {image[0]}x{image[1]} texture to {w}x{h}")
    return image


def blood_layer(wounded: tuple, base: tuple, fill: tuple) -> tuple[list, list]:
    """(alpha 0..1 per pixel, straight red blood layer premultiplied as (r, g, b) per pixel).

    `wounded` is the shipped olive _d, `base` the RJAF healthy and `fill` the
    IAF tex healthy, all at the _d size."""
    w, h, dr, dg, db = wounded
    _, _, br, bg, bb = base
    _, _, fr, fg, fb = fill
    n = w * h
    not_ok = [0 if _olive_like(br[i], bg[i], bb[i]) else 1 for i in range(n)]
    patch = _dilate(not_ok, w, h)
    fill_bad = _dilate([0 if _olive_like(fr[i], fg[i], fb[i]) else 1 for i in range(n)], w, h)
    count = n - sum(not_ok)
    if not count:
        raise ValueError("Olive base has no plain suit pixels")
    mean = tuple(sum(c[i] for i in range(n) if not not_ok[i]) / count for c in (br, bg, bb))
    base_r, base_g, base_b = [float(v) for v in br], [float(v) for v in bg], [float(v) for v in bb]
    for i in range(n):
        if patch[i]:
            if not fill_bad[i]:
                base_r[i], base_g[i], base_b[i] = float(fr[i]), float(fg[i]), float(fb[i])
            else:
                base_r[i], base_g[i], base_b[i] = mean
    alpha8 = []
    for i in range(n):
        r, g, b = dr[i], dg[i], db[i]
        xr, xg, xb = base_r[i], base_g[i], base_b[i]
        dist = abs(r - xr) + abs(g - xg) + abs(b - xb)
        lum = 0.299 * r + 0.587 * g + 0.114 * b
        w_red = _smooth(r - max(g, b), 0.0, 15.0) * (1.0 - _smooth(lum, 115.0, 150.0))
        dark = (0.299 * xr + 0.587 * xg + 0.114 * xb) - lum
        w_dark = _smooth(dark, 12.0, 40.0) * _smooth(r - g, -4.0, 3.0)
        alpha8.append(int(_smooth(dist, 10.0, 60.0) * max(w_red, w_dark) * 255.0))
    alpha = [v / 255.0 for v in _blur(alpha8, w, h)]
    layer = []
    for i in range(n):
        a = alpha[i]
        keep = 1.0 - a
        r = min(255.0, max(0.0, dr[i] - base_r[i] * keep))
        g = min(255.0, max(0.0, dg[i] - base_g[i] * keep))
        b = min(255.0, max(0.0, db[i] - base_b[i] * keep))
        # Olive excess over what blood carries goes into red.
        xg = max(0.0, g - 0.30 * r)
        xb = max(0.0, b - 0.25 * r)
        cap = 255.0 * a
        layer.append((min(cap, 255.0, r + xg + 0.5 * xb), min(cap, g - xg), min(cap, b - xb)))
    return alpha, layer


def composite(healthy: tuple, alpha: list, layer: list) -> tuple:
    """healthy * (1 - alpha) + layer, clipped and truncated to 8 bits."""
    w, h, hr, hg, hb = healthy
    out = ([], [], [])
    for i in range(w * h):
        keep = 1.0 - alpha[i]
        lr, lg, lb = layer[i]
        for c, v in zip(out, (hr[i] * keep + lr, hg[i] * keep + lg, hb[i] * keep + lb)):
            c.append(0 if v <= 0.0 else 255 if v >= 255.0 else int(v))
    return (w, h, *out)


def generate(source: dict[str, bytes], wanted=None) -> dict[str, bytes]:
    """{added key: PAA bytes} for `wanted` (default all) from the source member bytes; no pin checks."""
    names = outputs()
    wanted = set(names if wanted is None else wanted)
    result = {}
    for part in PARTS:
        suits = [(folder, healthy[part]) for folder, healthy in SUITS.items()
                 if part in healthy and _key(folder, part + "_dr.paa") in wanted]
        if not suits:
            continue
        shipped = source[_key(OLIVE_WOUNDED, part + "_d.paa")]
        wounded = paa.decode(shipped)
        levels = len(paa.read(shipped)["levels"])
        w, h = wounded[0], wounded[1]
        base = _fit(paa.decode(source[_key(OLIVE_BASE[0], OLIVE_BASE[1][part])]), w, h)
        fill = _fit(paa.decode(source[_key(OLIVE_FILL[0], OLIVE_FILL[1][part])]), w, h)
        alpha, layer = blood_layer(wounded, base, fill)
        for folder, healthy in suits:
            image = composite(_fit(paa.decode(source[_key(folder, healthy)]), w, h), alpha, layer)
            result[_key(folder, part + "_dr.paa")] = paa.write(image, levels)
    return {key: result[key] for key in names if key in result}


def _sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def add_members(existing: dict[str, bytes], asset_source=None, sources=None) -> tuple[dict, list[str]]:
    """existing: the ADDED_MEMBERS already in LoBo_Pilot.pbo; sources: {SOURCE_MEMBERS key: bytes}.

    Every source must carry its pinned bytes and every present member its
    pinned output; the missing members are generated, must equal their pins
    and are returned in ADDED_MEMBERS order. `asset_source` is not used.
    """
    unknown = sorted(set(existing) - set(OUTPUTS))
    if unknown:
        raise ValueError("Unknown added members: " + ", ".join(unknown))
    sources = sources or {}
    for key, pin in SOURCE_SHA256.items():
        if key not in sources:
            raise ValueError(f"lobo_pilot wounds: source {key} missing")
        if _sha(sources[key]) != pin:
            raise ValueError(f"lobo_pilot wounds: {key} is not the pinned LoBo texture; a different LoBo install")
    for key, data in existing.items():
        if _sha(data) != OUTPUTS[key][1]:
            raise ValueError(f"{key}: present but not the pinned generated texture")
    missing = [key for key in OUTPUTS if key not in existing]
    if not missing:
        return {}, []
    made = generate(sources, missing)
    added = {}
    for key in missing:
        stored, pin = OUTPUTS[key]
        if key not in made or _sha(made[key]) != pin:
            raise ValueError(f"{key}: generated bytes do not match the pinned sha256")
        added[key] = (stored, made[key])
    return added, []


# --- config pairs -------------------------------------------------------------

def _pair(folder: str, healthy: str, wounded: str) -> str:
    parts = folder.split(BS)
    return '"' + p("LoBo_Pilot", *parts, healthy) + '", "' + p("LoBo_Pilot", *parts, wounded) + '"'


def pair_rules() -> dict:
    """{class: ((authored pair, repaired pair), ...)}: the suit's healthy name with
    the shipped olive _d, then with the suit's generated _dr, in PARTS order."""
    return {cls: tuple((_pair(folder, SUITS[folder][part], part + "_d.paa"),
                        _pair(folder, SUITS[folder][part], part + "_dr.paa")) for part in PARTS)
            for cls, folder in CLASSES.items()}


# The face (xicht) entries of every authored full wounds[] (FULL), byte for
# byte: heads a to z and 2a to 2z, each with the wounded head it uses
# (xicht_a_zranen unless listed); one separator lacks its space, as authored.
_FACE_WOUNDED = {"e": "e", "o": "o", "v": "v", "w": "w", "2b": "w", "2d": "v", "2g": "w", "2p": "2p",
                 "2r": "e", "2w": "w"}


def _faces() -> str:
    heads = list("abcdefghijklmnopqrstuvwxyz") + ["2" + c for c in "abcdefghijklmnopqrstuvwxyz"]
    text = ", ".join('"xicht_' + h + '.paa", "xicht_' + _FACE_WOUNDED.get(h, "a") + '_zranen"' for h in heads)
    return text.replace('"xicht_2y.paa", "xicht_a_zranen"', '"xicht_2y.paa","xicht_a_zranen"')


FACES = _faces()
FULL = ("LoBo_IAF_Pilot", "LoBo_IAF_Pilot_grey", "LoBo_Egypt_Pilot", "LoBo_Jordan_Pilot")
# An authored full array up to its first body pair; every new array starts so.
FACE_HEAD = "\t\twounds[] =\n\t\t{\n\t\t\t" + FACES + ", "
# The pair order of every authored full array.
ORDER = ("armle", "armri", "back", "front", "legsbk", "legsfr", "legslft", "legsrgt")
# Where a class without wounds[] gets one: after class Animations, as in its parent.
ANIMATIONS_END = "\t\t\t};\n\t\t};\n"
IAF, IAF2, SYAF = "IAF" + BS + "tex", "IAF" + BS + "tex2", "SyAF" + BS + "tex"


def _rows(folder: str, ext_of: tuple, wounded: str, arm: str | None = None) -> tuple:
    """(folder, healthy, wounded name) per part in ORDER; `arm` is the left arm's own folder."""
    names = _names(ext_of)
    return tuple((arm if part == "armle" and arm else folder, names[part], part + wounded) for part in ORDER)


# class: (authored wounds[] or None when the class has none and inherits one,
# pairs of its new array). Every healthy name is a face of the class's model.
GREY_AUTHORED = "\t\twounds[] =\n\t\t{" + _pair("IAF" + BS + "tex3", "armle.paa", "armle_d.paa") + "};"
BLOCKS = {
    "LoBo_IAF_Pilot2": (None, _rows(IAF, ("back", "legsbk"), "_d.paa", "IAF" + BS + "tex5")),
    "LoBo_IAF_Pilot_grey2": (GREY_AUTHORED, _rows(IAF2, ("back",), "_dr.paa", "IAF" + BS + "tex3")),
    "LoBo_IAF_Pilot_grey3": (GREY_AUTHORED, _rows(IAF2, ("back",), "_dr.paa", "IAF" + BS + "tex4")),
    "LoBo_Syria_Pilot": (None, _rows(SYAF, ("back", "legsbk"), "_d.paa")),
}


def block_rules() -> dict:
    """{class: (authored, repaired)}; a class without wounds[] gets its array after class Animations."""
    rules = {}
    for cls, (authored, rows) in BLOCKS.items():
        block = FACE_HEAD + ", ".join(_pair(*row) for row in rows) + "\n\t\t};"
        rules[cls] = (ANIMATIONS_END, ANIMATIONS_END + block + "\n") if authored is None else (authored, block)
    return rules


# Shipped wounded textures the new arrays name (the generated ones are OUTPUTS):
# plan() fails closed when LoBo_Pilot.pbo lacks one.
REQUIRED_MEMBERS = tuple(dict.fromkeys(
    _key(folder, wounded) for _, rows in BLOCKS.values() for folder, _, wounded in rows
    if _key(folder, wounded) not in OUTPUTS))


def _class_span(lines: list[str], name: str) -> tuple[int, int]:
    """[start, end) line span of a CfgVehicles class body (one tab indent)."""
    head = [i for i, line in enumerate(lines)
            if line.startswith("\tclass " + name) and line[len("\tclass " + name):].lstrip()[:1] in (":", "")]
    if len(head) != 1:
        raise ValueError(f"Expected one class {name}, found {len(head)}")
    end = head[0] + 1
    while end < len(lines) and not lines[end].startswith("\tclass ") and lines[end] != "};":
        end += 1
    return head[0], end


def repair(documents: dict[str, str]) -> dict[str, str]:
    if PILOT not in documents:
        raise ValueError("Missing members: " + PILOT)
    source = documents[PILOT]
    newline, text = split_newline(source)
    lines = text.split("\n")
    try:
        # The new arrays copy the face entries every authored full array carries.
        for cls in FULL:
            start, end = _class_span(lines, cls)
            if "\n".join(lines[start:end]).count(FACE_HEAD) != 1:
                raise ValueError(f"class {cls}: wounds[] does not start with the expected face entries")
        for cls, rows in pair_rules().items():
            start, end = _class_span(lines, cls)
            body = "\n".join(lines[start:end])
            for before, after in rows:
                body = _replace(body, before, after, 1)
            lines[start:end] = body.split("\n")
        for cls, (before, after) in block_rules().items():
            start, end = _class_span(lines, cls)
            body = _replace("\n".join(lines[start:end]), before, after, 1)
            if body.count("wounds[]") != 1:
                raise ValueError(f"class {cls}: expected one wounds[], found {body.count('wounds[]')}")
            lines[start:end] = body.split("\n")
        updated = "\n".join(lines)
    except ValueError as exc:
        raise ValueError(f"{PILOT}: {exc}") from exc
    updated = updated.replace("\n", newline)
    return {PILOT: updated} if updated != source else {}
