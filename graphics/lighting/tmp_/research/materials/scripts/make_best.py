"""Final comparison images for the materials track (resumed session).
Run from the research folder: python scripts/make_best.py"""
import subprocess, sys
C = [sys.executable, 'scripts/compare.py']
def run(args):
    out = subprocess.run(C + args, capture_output=True, text=True)
    if out.returncode:
        raise SystemExit(out.stderr)
    print(out.stdout.strip()[:160])
CUR = 'remastered:Remastered (current)'
# Day views: r8 (shader v8, the final day-time state; v9 only changes local-light reflections).
run(['runs/r8', 'lowsun_fp', 'BEST_lowsun_fp.png', 'classic:Classic', CUR, 'materials:Remastered + vbfix + materials (v8)',
     '--title', 'lowsun_fp, dry: Classic | Remastered (current) | prototype'])
run(['runs/r8', 'lowsun_fp', 'BEST_lowsun_fp_wet.png', 'classic:Classic', CUR, 'wet:prototype, wetness 1 (after rain)',
     '--title', 'lowsun_fp after rain (wetness=1): wet gravel, puddles with screen-space reflections'])
run(['runs/r8', 'noon_fields', 'BEST_noon_fields.png', 'classic:Classic', CUR, 'materials:Remastered + vbfix + materials (v8)',
     '--title', 'noon_fields, dry'])
run(['runs/r8', 'abel_rain', 'BEST_abel_rain.png', 'classic:Classic (rain)', 'remastered:Remastered (current, rain)',
     'materials:prototype (v8): wet from the rain, SSR puddles', '--title', 'abel_rain: overcast 0.9, rain falling'])
run(['runs/r8', 'sinai_lowsun_fp', 'BEST_sinai_lowsun_fp.png', 'classic:Classic', CUR,
     'materials:Remastered + vbfix + materials (v8)', '--title', 'sinai_lowsun_fp (@LoBo)'])
run(['runs/r7', 'overcast_heavy_rain', 'BEST_overcast_heavy_rain.png', 'classic:Classic (rain)',
     'remastered:Remastered (current, rain)', 'materials:prototype (v7): wet from the rain',
     '--title', 'overcast_heavy with rain (Eden meadow): v7, grass holds no puddles'])
run(['runs/r7', 'tank_gunner_optics', 'BEST_tank_gunner_optics.png', 'classic:Classic', CUR,
     'materials:prototype (v7)', '--title', 'tank_gunner_optics (no visible change)'])
# Crops
run(['runs/r8', 'sinai_lowsun_fp', 'BEST_crop_sinai_hmmwv.png', 'classic:Classic', CUR, 'vbfix:+ vbfix only',
     'norim:+ materials, rim off', 'materials:+ materials (v8)', '--crop', '0,190,330,420', '--scale', '1.2',
     '--title', 'HMMWV at low sun (crop): glass reflects the sky, satin grazing sheen defines the hull'])
run(['runs/r8', 'lowsun_fp', 'BEST_crop_lowsun_m113.png', 'classic:Classic', CUR, 'vbfix:+ vbfix only',
     'materials:+ materials (v8)', 'wet:+ materials, wet', '--crop', '560,210,860,420', '--scale', '1.3',
     '--title', 'lowsun_fp M113 shaded hull (crop)'])
run(['runs/r8', 'noon_fields', 'BEST_crop_noon_jeep.png', 'classic:Classic', CUR, 'vbfix:+ vbfix only',
     'materials:+ materials (v8)', 'wet:+ materials, wet', '--crop', '540,360,720,450', '--scale', '1.6',
     '--title', 'noon jeep (crop): v8 keeps the olive paint (v6 had turned it grey-blue)'])
run(['runs/r8', 'abel_rain', 'BEST_crop_abel_rain_puddles.png', 'remastered:Remastered (current, rain)',
     'wetnossr:prototype, puddles reflect the sky only', 'materials:prototype + SSR puddles',
     '--crop', '330,330,960,640', '--title', 'abel_rain puddles (crop 1:1)'])
# Night views: r9 (shader v9: lamp reflections through a water-film lobe).
if '--night' in sys.argv or True:
    run(['runs/r9', 'town_night_rain', 'BEST_town_night_rain.png', 'classic:Classic (rain)',
         'remastered:Remastered (current, rain)', 'materials:prototype (v9): wet, lamp streaks, SSR puddles',
         '--title', 'town_night with rain: street lamp and fire reflections on the wet square'])
    run(['runs/r9', 'town_night', 'BEST_town_night_wet.png', 'classic:Classic', CUR,
         'wet:prototype (v9), wetness 1 (after rain)', '--title', 'town_night after rain (wetness=1)'])
    run(['runs/r9', 'town_night', 'BEST_crop_town_night_wet.png', CUR, 'materials:prototype v9, dry',
         'wet:prototype v9, wet', 'wetnossr:prototype v9, wet, SSR off', '--crop', '300,300,960,640', '--cols', '2',
         '--scale', '0.8', '--title', 'town_night (crop): lamp streaks come from the light-specular film lobe, '
         'house reflections from SSR'])
