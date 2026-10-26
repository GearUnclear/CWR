import subprocess, pathlib
B = pathlib.Path('D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/boom'); out = B / 'compare'
FF = 'C:/ffmpeg/bin/ffmpeg.exe'; FONT = "C\:/Windows/Fonts/arialbd.ttf"
NAMES = ['before', '1 physics', '2 look table', '3 HE vs fuel', '4 rendering', '5 timing', '6 trailer']
CLIPS = [('dusk_he', 'Dusk: 120 mm HE shell', 98), ('dusk_grenade', 'Dusk: hand grenade', 158), ('dusk_heat', 'Dusk: 120 mm HEAT round', 98),
         ('dusk_fuel', 'Dusk: jeep fuel explosion', 158), ('night_he', 'Night: 120 mm HE shell', 98), ('night_fuel', 'Night: jeep fuel explosion', 158)]
segs = []
def dt(text, size=22, y=8):
    t = text.replace(':', '\:')
    return f"drawtext=fontfile='{FONT}':text='{t}':x=8:y={y}:fontsize={size}:fontcolor=yellow:box=1:boxcolor=black@0.6:boxborderw=4"
for clip, title, nf in CLIPS:
    srcs = [B / 'before/clips' / f'{clip}.mp4'] + [B / str(n) / 'final' / f'{clip}.mp4' for n in range(1, 7)]
    for slow in (False, True):
        dur = nf / 30 if not slow else 45 / 30 * 4
        inputs, chains = [], []
        for i, s in enumerate(srcs):
            inputs += ['-i', str(s)]
            pts = 'setpts=4*PTS,' if slow else ''
            trim = 'trim=end_frame=45,setpts=PTS-STARTPTS,' if slow else ''
            chains.append(f"[{i}:v]{trim}{pts}scale=480:320,{dt(NAMES[i])}[v{i}]")
        label = title + (' (slow motion x4, first 1.5 s)' if slow else ' (real time)')
        chains.append(f"color=c=0x141414:s=480x320:d={dur:.2f},{dt(label.split(' (')[0], 22, 120)},{dt('(' + label.split(' (')[1], 18, 160)}[v7]")
        lay = '0_0|480_0|960_0|1440_0|0_320|480_320|960_320|1440_320'
        fc = ';'.join(chains) + f";[v0][v1][v2][v3][v7][v4][v5][v6]xstack=inputs=8:layout={lay}[g]"
        seg = out / f"seg_{clip}_{'slow' if slow else 'rt'}.mp4"
        subprocess.run([FF, '-v', 'error', '-y', *inputs, '-filter_complex', fc, '-map', '[g]', '-t', f'{dur:.2f}', '-r', '30',
                        '-c:v', 'libx264', '-crf', '20', '-pix_fmt', 'yuv420p', str(seg)], check=True)
        segs.append(seg)
lst = out / 'list.txt'; lst.write_text(''.join(f"file '{s.as_posix()}'\n" for s in segs))
final = out / 'explosions_six_options.mp4'
subprocess.run([FF, '-v', 'error', '-y', '-f', 'concat', '-safe', '0', '-i', str(lst), '-c', 'copy', '-movflags', '+faststart', str(final)], check=True)
print(final, final.stat().st_size // 1024, 'KB')
