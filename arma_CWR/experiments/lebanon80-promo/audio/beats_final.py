"""Final beat grid, downbeats, hits and sections for the promo instrumental (librosa).

    python beats_final.py <instrumental.wav> <drums stem.wav> <beats.json> [start bpm, default 126.2]

beats.py is the first look (tempo, phase, candidate sections); this is the map the edit cuts to. The
section candidates (cands, labels below) are this track's, chosen by ear from beats.py's loudness and
low-band novelty; set them for a new track. The cut list in edit/build.py takes its times from the
beats.json this writes. Needs librosa and scipy.
"""
import sys, os, json, numpy as np, librosa, scipy.ndimage as ndi
inst, drums, out = sys.argv[1:4]
START_BPM = float(sys.argv[4]) if len(sys.argv) > 4 else 126.2
SR = 22050
y, _ = librosa.load(inst, sr=SR); yd, _ = librosa.load(drums, sr=SR)
dur = len(y)/SR
# 1. beats: DP tracker on the drums-stem onset envelope (fine hop), then a smooth tempo curve
H1 = 128
od = librosa.onset.onset_strength(y=yd, sr=SR, hop_length=H1)
_, bt = librosa.beat.beat_track(onset_envelope=od, sr=SR, hop_length=H1, start_bpm=START_BPM, tightness=400, units='time')
fps1 = SR/H1
# keep beats inside the music (drums present): drop leading/trailing beats with no drum energy nearby
rmsd = librosa.feature.rms(y=yd, hop_length=H1)[0]; rd = 20*np.log10(rmsd+1e-9)
alive = np.array([rd[max(0,int(t*fps1)-20):int(t*fps1)+20].max() > -45 for t in bt])
first, last = np.argmax(alive), len(alive)-1-np.argmax(alive[::-1])
bt = bt[first:last+1]
k = np.arange(len(bt)); c = np.polyfit(k, bt, 3); bs = np.polyval(c, k)
resid = bt - bs
ibi = np.diff(bs); bpm_curve = 60/ibi
# 2. downbeat phase: kick band onset of drums + full-mix onset at each beat, summed per phase of 4
H = 512; fps = SR/H
S = np.abs(librosa.stft(y, hop_length=H)); f = librosa.fft_frequencies(sr=SR)
Sd = np.abs(librosa.stft(yd, hop_length=H))
kick = librosa.onset.onset_strength(S=librosa.amplitude_to_db(Sd[f < 150]), sr=SR, hop_length=H)
oenv = librosa.onset.onset_strength(y=y, sr=SR, hop_length=H)
bi = np.minimum(np.round(bs*fps).astype(int), len(oenv)-1)
z = lambda a: (a-a.mean())/(a.std()+1e-9)
sc = z(kick[bi]) + z(oenv[bi])
phase_scores = [round(float(sc[p::4].mean()),3) for p in range(4)]
# loudness level
rms = librosa.feature.rms(y=y, hop_length=H)[0]; rdb = 20*np.log10(rms+1e-9); tt = librosa.times_like(rms, sr=SR, hop_length=H)
low = 20*np.log10(S[f < 150].sum(0)+1e-9)
# section-entry test for phase: loudness jumps should fall on downbeats
L = ndi.uniform_filter1d(rdb, int(0.25*fps))
jump = np.r_[np.zeros(int(fps)), L[int(fps):]-L[:-int(fps)]]
jp = [p for p in librosa.util.peak_pick(jump, pre_max=int(2*fps), post_max=int(2*fps), pre_avg=1, post_avg=1, delta=0, wait=int(2*fps)) if jump[p] > 4]
jt = np.array([tt[p]-0.5 for p in jp])
def phase_err(p):
    db = bs[p::4]; return float(np.mean([np.min(np.abs(db-t)) for t in jt]))
phase_entry_err = [round(phase_err(p),3) for p in range(4)]
phase = int(np.argmax(np.array(phase_scores) - 2*np.array(phase_entry_err)))
db = bs[phase::4]
# 3. hits
on = oenv/oenv.max()
def lvl(a, b): m = (tt >= a) & (tt < b); return float(10*np.log10(np.mean(rms[m]**2)+1e-12))
pk = librosa.util.peak_pick(on, pre_max=10, post_max=10, pre_avg=20, post_avg=20, delta=0.1, wait=10)
impacts = []
for p in pk:
    t = float(tt[p]); d = lvl(t, t+0.75) - lvl(t-0.75, t)
    if d > 4: impacts.append(dict(t=round(t,3), jump_db=round(d,1), onset=round(float(on[p]),3), beat_index=int(np.argmin(np.abs(bs-t))), on_downbeat=bool(np.min(np.abs(db-t)) < 0.08)))
order = np.argsort(-on[pk]); strong = []
for i in order:
    t = float(tt[pk[i]])
    if all(abs(t - s['t']) >= 2.0 for s in strong): strong.append(dict(t=round(t,3), onset=round(float(on[pk[i]]),3)))
    if len(strong) >= 25: break
strong.sort(key=lambda s: s['t'])
# 4. sections: boundaries chosen from the loudness/low-band novelty candidates of beats.py, snapped to downbeats
cands = [0.0, 13.5, 20.5, 31.75, 35.6, 46.9, 66.0, 86.9, 104.0, 107.7, 111.8, 124.4, 128.4, 143.8, 147.5, 155.0, dur]
labels = ['intro','build','drop','break','drop','break','drop','break','build','drop','break','build','drop','break','drop','outro']
def snap(t):
    if t in (0.0, dur): return t
    j = np.argmin(np.abs(db-t)); return float(db[j]) if abs(db[j]-t) < 0.7 else t
bnd = [round(snap(t),3) for t in cands]
secs = []
for i, lab in enumerate(labels):
    a, b = bnd[i], bnd[i+1]; m = (tt >= a) & (tt < b)
    half = (a+b)/2
    secs.append(dict(start=a, end=b, label=lab, loudness_rms_dbfs=round(lvl(a,b),1),
                     low_band_db=round(float(np.mean(low[m])),1),
                     first_half_dbfs=round(lvl(a,half),1), second_half_dbfs=round(lvl(half,b),1),
                     bars=round((b-a)/(4*np.median(ibi)),1)))
res = dict(file=os.path.basename(inst), duration_s=round(dur,6),
           tempo_bpm=round(float(60/np.median(ibi)),2),
           tempo_note='a cubic fit of beat time against beat index: beat times follow a slow tempo drift (see tempo_curve)',
           tempo_curve=[dict(t=round(float(bs[i]),1), bpm=round(float(bpm_curve[i]),2)) for i in range(0, len(ibi), 32)],
           beat_fit_residual_ms=dict(median=round(float(np.median(np.abs(resid))*1000),1), max=round(float(np.abs(resid).max()*1000),1)),
           beats=[round(float(t),3) for t in bs],
           downbeats=[round(float(t),3) for t in db], downbeat_phase=phase, downbeat_phase_scores=phase_scores,
           downbeat_phase_entry_error_s=phase_entry_err, bar_length_s=round(float(4*np.median(ibi)),3),
           impacts=impacts, strongest_onsets=strong, sections=secs,
           method=dict(beats='librosa.beat.beat_track on drums-stem onset strength (hop 128, start_bpm %g, tightness 400), cubic fit of beat time vs index' % START_BPM,
                       downbeats='bar = 4 beats; phase picked from kick-band + full-mix onset strength at each beat phase and from where loudness jumps land',
                       impacts='onset peaks where loudness 0.75 s after is > 4 dB above 0.75 s before',
                       strongest_onsets='top 25 full-mix onset peaks at least 2 s apart',
                       sections='loudness and low-band (<150 Hz) novelty candidates cross-checked with librosa.segment.agglomerative on beat-synced chroma+MFCC, snapped to downbeats; loudness = mean RMS dBFS'))
json.dump(res, open(out, 'w'), indent=1)
print('tempo', res['tempo_bpm'], 'beats', len(bs), 'span', bs[0].round(3), bs[-1].round(3), 'fit resid ms', res['beat_fit_residual_ms'])
print('phase scores', phase_scores, 'entry err', phase_entry_err, 'chosen', phase, 'jumps', np.round(jt,2))
print('curve', [(c['t'], c['bpm']) for c in res['tempo_curve']])
for s in secs: print(s)
print('impacts', [(i['t'], i['jump_db'], i['on_downbeat']) for i in impacts])
print('strong', [(s['t'], s['onset']) for s in strong])
