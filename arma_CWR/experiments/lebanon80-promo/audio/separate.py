"""Stem separation with Demucs through its Python API, at the model's rate, stems written at the input's rate.

    python separate.py <source.wav> <out dir> [-n htdemucs_ft] [--shifts 2] [--overlap 0.25] [--prefix p_]
                       [--device cuda]

Writes <out dir>/<prefix><stem>.wav (32-bit float) for each of the model's sources (drums, bass, other,
vocals), trimmed or padded to the source's length. mkinst.py then sums drums + bass + other into the
instrumental; whisper_scan.py and envstats.py check that no voice is left in it. The source is any WAV the
production has the right to use; keep it and the stems in the production folder, never in the repo, and
name them generically (the source's own title stays out of file names, notes and commits). Needs torch,
torchaudio, demucs and soundfile.
"""
import argparse

import numpy as np
import soundfile as sf
import torch
import torchaudio
from demucs.apply import apply_model
from demucs.pretrained import get_model

ap = argparse.ArgumentParser()
ap.add_argument('inp')
ap.add_argument('outdir')
ap.add_argument('-n', '--model', default='htdemucs_ft')
ap.add_argument('--shifts', type=int, default=2)
ap.add_argument('--overlap', type=float, default=0.25)
ap.add_argument('--prefix', default='')
ap.add_argument('--device', default='cuda' if torch.cuda.is_available() else 'cpu')
a = ap.parse_args()
RS = dict(lowpass_filter_width=64, rolloff=0.9475937167399596, resampling_method='sinc_interp_kaiser', beta=14.769656459379492)
x, sr = sf.read(a.inp, dtype='float32', always_2d=True)
n = x.shape[0]
model = get_model(a.model)
model.eval()
msr = model.samplerate
wav = torch.from_numpy(x.T.copy())
if sr != msr:
    wav = torchaudio.functional.resample(wav, sr, msr, **RS)
ref = wav.mean(0)
mean, std = ref.mean(), ref.std()
inp = ((wav - mean) / std)[None].to(a.device)
with torch.no_grad():
    out = apply_model(model, inp, device=a.device, shifts=a.shifts, split=True, overlap=a.overlap, progress=False)[0]
out = out.cpu() * std + mean
for i, name in enumerate(model.sources):
    s = out[i]
    if sr != msr:
        s = torchaudio.functional.resample(s, msr, sr, **RS)
    s = s.numpy().T
    s = s[:n] if s.shape[0] >= n else np.pad(s, ((0, n - s.shape[0]), (0, 0)))
    sf.write(f"{a.outdir}/{a.prefix}{name}.wav", s, sr, subtype='FLOAT')
    print(name, s.shape, float(np.abs(s).max()))
print('model', a.model, 'sources', model.sources, 'model_sr', msr, 'in_sr', sr, 'samples', n)
