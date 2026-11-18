"""Beat, downbeat, hit and section analysis of the promo instrumental with librosa (the first look).

    python beats.py <instrumental.wav> <drums stem.wav> <out.json>

Prints the tempo, the downbeat phase, the big hits and candidate section boundaries (loudness and
low-band novelty, and an agglomerative split of beat-synced chroma + MFCC). Pick the sections by ear from
these, then run beats_final.py for the map the edit cuts to. Needs librosa and scipy.
"""
import sys, json, numpy as np, librosa, scipy.ndimage as ndi
src, drums, out = sys.argv[1], sys.argv[2], sys.argv[3]
SR, HOP = 22050, 512
y, _ = librosa.load(src, sr=SR, mono=True)
yd, _ = librosa.load(drums, sr=SR, mono=True)
dur = len(y)/SR
oenv = librosa.onset.onset_strength(y=y, sr=SR, hop_length=HOP)
tempo, bframes = librosa.beat.beat_track(onset_envelope=oenv, sr=SR, hop_length=HOP, units='frames')
tempo = float(np.atleast_1d(tempo)[0])
btimes = librosa.frames_to_time(bframes, sr=SR, hop_length=HOP)
# downbeat phase: kick-band onset strength of the drums stem at each beat, summed per phase of 4
Sd = np.abs(librosa.stft(yd, hop_length=HOP)); f = librosa.fft_frequencies(sr=SR)
kick = librosa.onset.onset_strength(S=librosa.amplitude_to_db(Sd[f < 150]), sr=SR, hop_length=HOP)
full_at = oenv[bframes]; kick_at = kick[np.minimum(bframes, len(kick)-1)]
def z(a): return (a - a.mean())/(a.std()+1e-9)
score = z(full_at) + z(kick_at)
phase_scores = [float(score[p::4].mean()) for p in range(4)]
phase = int(np.argmax(phase_scores))
dtimes = btimes[phase::4]
# big hits: onset peaks well above the local norm
on_n = oenv / oenv.max()
pk = librosa.util.peak_pick(on_n, pre_max=20, post_max=20, pre_avg=40, post_avg=40, delta=0.12, wait=40)
thr = np.percentile(on_n, 99)
hits = [dict(t=round(float(librosa.frames_to_time(p, sr=SR, hop_length=HOP)),3), strength=round(float(on_n[p]),3)) for p in pk if on_n[p] >= thr]
# section boundaries: novelty on smoothed loudness and low-band level, plus recurrence (chroma+mfcc) agglomerative
rms = librosa.feature.rms(y=y, hop_length=HOP)[0]; rdb = 20*np.log10(rms+1e-9)
S = np.abs(librosa.stft(y, hop_length=HOP)); low = 20*np.log10(S[f < 150].sum(0)+1e-9)
fps = SR/HOP; sm = lambda a, s: ndi.uniform_filter1d(a, int(s*fps))
L = sm(rdb, 1.0); B = sm(low, 1.0)
lag = int(1.5*fps)
nov = np.abs(np.r_[np.zeros(lag), L[lag:]-L[:-lag]]) + 0.5*np.abs(np.r_[np.zeros(lag), B[lag:]-B[:-lag]])
npk = librosa.util.peak_pick(nov, pre_max=int(3*fps), post_max=int(3*fps), pre_avg=int(3*fps), post_avg=int(3*fps), delta=2.0, wait=int(4*fps))
lvl_bounds = sorted(set(float(librosa.frames_to_time(p - lag//2, sr=SR, hop_length=HOP)) for p in npk))
chroma = librosa.feature.chroma_cqt(y=y, sr=SR, hop_length=HOP); mfcc = librosa.feature.mfcc(y=y, sr=SR, hop_length=HOP, n_mfcc=13)
feat = librosa.util.sync(np.vstack([z(chroma), z(mfcc)]), bframes)
agg = librosa.segment.agglomerative(feat, 12)
agg_bounds = [float(btimes[min(i, len(btimes)-1)]) for i in agg if i > 0]
snap = lambda t: float(dtimes[np.argmin(np.abs(dtimes - t))]) if abs(dtimes[np.argmin(np.abs(dtimes - t))] - t) < 1.2 else t
json.dump(dict(tempo=tempo, beats=btimes.tolist(), phase=phase, phase_scores=phase_scores, downbeats=dtimes.tolist(),
               hits=hits, lvl_bounds=lvl_bounds, lvl_bounds_snapped=[snap(t) for t in lvl_bounds], agg_bounds=agg_bounds,
               duration=dur), open(out, 'w'))
print('tempo', tempo, 'beats', len(btimes), 'phase', phase, [round(p,2) for p in phase_scores])
print('beat interval median', np.median(np.diff(btimes)), 'first beats', np.round(btimes[:8],3))
print('lvl bounds', [round(t,2) for t in lvl_bounds]); print('snapped', [round(snap(t),2) for t in lvl_bounds])
print('agg bounds', [round(t,2) for t in agg_bounds])
print('hits', [(h['t'],h['strength']) for h in hits])
