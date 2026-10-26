import sys, os, glob
from PIL import Image, ImageDraw
old, new, out = sys.argv[1], sys.argv[2], sys.argv[3]
os.makedirs(out, exist_ok=True)
for pn in sorted(glob.glob(os.path.join(new, '*__pair.png'))):
    name = os.path.basename(pn)
    po = os.path.join(old, name)
    b = Image.open(pn).convert('RGB')
    a = Image.open(po).convert('RGB') if os.path.exists(po) else Image.new('RGB', b.size, (60, 0, 0))
    w = max(a.width, b.width)
    img = Image.new('RGB', (w, a.height + b.height + 8), (255, 255, 0))
    img.paste(a, (0, 0)); img.paste(b, (0, a.height + 8))
    d = ImageDraw.Draw(img)
    d.rectangle([w - 330, 0, w, 36], fill=(150, 0, 0)); d.text((w - 320, 8), 'BEFORE 20261017-final', fill=(255, 255, 255))
    d.rectangle([w - 330, a.height + 8, w, a.height + 44], fill=(0, 110, 0)); d.text((w - 320, a.height + 16), 'AFTER 20261018-bugfix', fill=(255, 255, 255))
    img = img.resize((int(img.width * 0.8), int(img.height * 0.8)), Image.LANCZOS)
    img.save(os.path.join(out, name.replace('__pair', '__stack')))
    print(name)
