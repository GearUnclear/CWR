import sys
from PIL import Image
E = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B10"
OUT = r"C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\rev"
view, box, name = sys.argv[1], tuple(int(v) for v in sys.argv[2].split(',')), sys.argv[3]
folder = sys.argv[4] if len(sys.argv) > 4 else 'final2'
imgs = []
for s in ('off', 'on'):
    im = Image.open(rf"{E}\{folder}\clouds_{view}__{s}.png").convert('RGB')
    print(s, im.size)
    imgs.append(im.crop(box))
w, h = imgs[0].size
out = Image.new('RGB', (w * 2 + 8, h), (0, 0, 0))
out.paste(imgs[0], (0, 0)); out.paste(imgs[1], (w + 8, 0))
out.save(rf"{OUT}\{name}.png")
