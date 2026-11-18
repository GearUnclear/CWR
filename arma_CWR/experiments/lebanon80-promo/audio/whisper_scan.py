"""Speech scan with faster-whisper: where a stem holds words, by timestamp and word count only.

    python whisper_scan.py <stem.wav> [--model large-v3] [--vad] [--lang xx] [--device cuda]

Used to prove an instrumental carries no voice: run it on the instrumental (and on the separated vocal stem
for comparison) and read the segment times, word counts, no-speech probability and log probability. It
never prints or saves the recognised text: a song's words must not reach the terminal, a notes file or the
repo. Needs faster-whisper (UD_WHISPER_PYTHON in udpaths.py names a Python that has it).
"""
import argparse

from faster_whisper import WhisperModel

ap = argparse.ArgumentParser()
ap.add_argument('wav')
ap.add_argument('--model', default='large-v3')
ap.add_argument('--vad', action='store_true')
ap.add_argument('--lang', default=None)
ap.add_argument('--device', default='cuda')
a = ap.parse_args()
m = WhisperModel(a.model, device=a.device, compute_type='float16' if a.device == 'cuda' else 'int8')
segs, info = m.transcribe(a.wav, vad_filter=a.vad, word_timestamps=True, language=a.lang, beam_size=5,
                          condition_on_previous_text=False)
segs = list(segs)
print(f"# model={a.model} vad={a.vad} lang={info.language} p={info.language_probability:.2f} segments={len(segs)}")
nw = 0
for s in segs:
    words = s.words or []
    nw += len(words)
    print(f"{s.start:7.2f} {s.end:7.2f} words={len(words)} nsp={s.no_speech_prob:.2f} lp={s.avg_logprob:.2f}")
print('# total words', nw)
