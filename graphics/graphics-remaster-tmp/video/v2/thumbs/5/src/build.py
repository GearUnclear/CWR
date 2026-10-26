import sys, subprocess
from lib import *
S = r"C:/Users/Dane/AppData/Local/Temp/claude/D--Arma-CWA/5d8095f8-8e74-4ade-8eb5-abff8cad78dc/scratchpad/t5"
POSTER = r"D:/Arma_CWA/ChatGPT Image Oct 5, 2026, 09_49_52 PM.png"
which = sys.argv[1:] or ["tri","stack","hero","night","poster"]

def capof(f): b = f.getbbox("U"); return b[3]-b[1]

# ---------- landscape: three stacked pairs ----------
def tri():
    TILES = [["pine_shafts",57,520,100,1120],["outpost",300,360,130,1200],["supply_run",300,520,220,1040]]
    W,H,BAND,G = 1280,720,150,4
    tw = (W-2*G)//3; rh = (H-BAND-G)//2
    cv = Image.new("RGB",(W,H),(11,11,11))
    for i,(clip,f,x0,y0,rw) in enumerate(TILES):
        rhh = round(rw*rh/tw); x = i*(tw+G)
        if i == 2: tw_i = W - x
        else: tw_i = tw
        rw_i = round(rw*tw_i/tw)
        cv.paste(crop(frame(clip,"vanilla",f),x0,y0,rw_i,rhh,tw_i,rh),(x,BAND))
        cv.paste(crop(frame(clip,"ud",f),x0,y0,rw_i,rhh,tw_i,rh),(x,BAND+rh+G))
    d = ImageDraw.Draw(cv); seam = BAND+rh
    d.rectangle((0,seam,W,seam+G-1),fill=WHITE)
    fb,fs,fl = font(118),font(44),font(34)
    base = (BAND+capof(fb))//2+4
    draw_text(cv,(34,base),"USLU DUR!",fb,track=4,shadow=0)
    draw_text(cv,(W-36,base),"PART 1: GRAPHICS",fs,fill=GREEN,track=4,shadow=0,anchor="rs")
    draw_text(cv,(W-18,seam-14),"VANILLA",fl,track=3,shadow=0.75,anchor="rs")
    draw_text(cv,(W-18,seam+G+14+capof(fl)),"USLU DUR!",fl,track=3,shadow=0.75,anchor="rs")
    return cv

# ---------- Shorts cover: the two sides stacked ----------
def stack(clip="pine_shafts", f=57, x0=520, y0=100):
    W,H,SEAM = 1080,1920,960
    cv = Image.new("RGB",(W,H),(10,10,10))
    cv.paste(crop(frame(clip,"vanilla",f),x0,y0,1080,958,1080,SEAM-2),(0,0))
    cv.paste(crop(frame(clip,"ud",f),x0,y0,1080,958,1080,H-SEAM-2),(0,SEAM+2))
    ImageDraw.Draw(cv).rectangle((0,SEAM-2,W,SEAM+1),fill=WHITE)
    fb,fl = font(200),font(80)
    draw_text(cv,(60,SEAM-32),"VANILLA",fl,track=5,shadow=0.65)
    draw_text(cv,(54,SEAM+36+capof(fb)),"USLU DUR!",fb,track=6,shadow=0.4,blur=10)
    return cv

# ---------- Shorts cover: one tall hero crop ----------
def hero():
    W,H = 1080,1920
    cx, cw = 1000, 608
    cv = crop(frame("opening","ud",99), cx-cw//2, 0, cw, 1080, W, H)
    fb,fs = font(204),font(66)
    y = 150+capof(fb)
    draw_text(cv,(W//2,y),"USLU DUR!",fb,track=6,shadow=0.4,blur=10,anchor="ms")
    draw_text(cv,(W//2,y+104),"PART 1: GRAPHICS",fs,track=8,shadow=0.55,blur=5,anchor="ms")
    return cv

# ---------- Shorts cover: the poster's masthead over the stacked sides ----------
def poster():
    W,H = 1080,1920
    RED = (175,78,46); CREAM = None
    p = Image.open(POSTER).convert("RGB")
    title = p.crop((25,38,486,294))           # inside the red border, title only
    CREAM = p.getpixel((40,45))
    sc = 2
    tw, th = title.width*sc, title.height*sc
    m = (W-tw)//2
    cv = Image.new("RGB",(W,H),RED)
    pad_top, pad_bot = 10, 26
    cream = Image.new("RGB",(tw, th+pad_top+pad_bot), CREAM)
    cream.paste(title.resize((tw,th),Image.NEAREST),(0,pad_top))
    cv.paste(cream,(m,m))
    top = m+cream.height
    bot = H-m
    seam = top + (bot-top)//2
    hh_top = seam-2-top; hh_bot = bot-(seam+2)
    clip,f,x0,y0 = "pine_shafts",57,260,200
    rw = 1180; rh = round(rw*hh_top/tw)
    cv.paste(crop(frame(clip,"vanilla",f),x0,y0,rw,rh,tw,hh_top),(m,top))
    cv.paste(crop(frame(clip,"ud",f),x0,y0,rw,rh,tw,hh_bot),(m,seam+2))
    ImageDraw.Draw(cv).rectangle((m,seam-2,m+tw-1,seam+1),fill=CREAM)
    fl = font(72)
    draw_text(cv,(m+36,seam-28),"VANILLA",fl,fill=CREAM,track=5,shadow=0.7)
    draw_text(cv,(m+36,seam+30+capof(fl)),"USLU DUR!",fl,fill=CREAM,track=5,shadow=0.7)
    return cv

out = {}
if "tri" in which: out["thumb_stack_triptych"] = tri()
if "stack" in which: out["shorts_cover_stack"] = stack()
if "hero" in which: out["shorts_cover_hero"] = hero()
if "night" in which: out["shorts_cover_stack_night"] = stack("supply_run",300,420,100)
if "poster" in which: out["shorts_cover_poster"] = poster()
for n,img in out.items():
    p,j = save_both(img,n)
    small_views(img,n)
    print(n, img.size, p)
