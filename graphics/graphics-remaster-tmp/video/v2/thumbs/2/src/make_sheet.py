import os, sys
from PIL import Image, ImageDraw
from lib import BASE, font
stems = sys.argv[1:] or ['2a_poster_beside_outpost', '2b_poster_frame_outpost', '2c_poster_column_split']
pad = 16
cols_w = 360 + pad + 168
sheet = Image.new('RGB', (pad + len(stems) * (cols_w + pad), pad * 2 + 202 + 22), (24, 24, 24))
d = ImageDraw.Draw(sheet)
for i, s in enumerate(stems):
    im = Image.open(os.path.join(BASE, s + '.png')).convert('RGB')
    x = pad + i * (cols_w + pad)
    sheet.paste(im.resize((360, 202), Image.LANCZOS), (x, pad + 22))
    sheet.paste(im.resize((168, 94), Image.LANCZOS), (x + 360 + pad, pad + 22))
    d.text((x, pad), s, font=font(14, 'cwr_body.ttf'), fill=(220, 220, 220))
sheet.save(os.path.join(BASE, 'sheet_small.png'))
# a light-theme copy for my own check (YouTube's white feed)
print(sheet.size)
