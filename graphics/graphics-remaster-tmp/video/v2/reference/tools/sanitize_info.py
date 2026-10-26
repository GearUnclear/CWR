"""Read a yt-dlp -J info dict on stdin and write a cookie-free metadata subset.

Usage: yt-dlp -J ... | python -I sanitize_info.py <out.json>
Formats (which carry per-request cookies and signed URLs) are dropped entirely.
Caption tracks keep only language codes, track names and extensions, never URLs.
"""
import json
import sys


def tracks(d):
    out = {}
    for lang, entries in (d or {}).items():
        out[lang] = sorted({(e.get("ext") or "?") for e in entries})
    return out


def main():
    out_path = sys.argv[1]
    raw = sys.stdin.read()
    info = json.loads(raw) if raw.strip() else None
    if not isinstance(info, dict):
        json.dump(None, open(out_path, "w", encoding="utf-8"))
        print("NO_INFO")
        return
    subs = tracks(info.get("subtitles"))
    auto = tracks(info.get("automatic_captions"))
    keep = {
        "id": info.get("id"),
        "title": info.get("title"),
        "channel": info.get("channel"),
        "channel_id": info.get("channel_id"),
        "uploader": info.get("uploader"),
        "uploader_id": info.get("uploader_id"),
        "upload_date": info.get("upload_date"),
        "duration": info.get("duration"),
        "age_limit": info.get("age_limit"),
        "webpage_url": info.get("webpage_url"),
        "subtitles": subs,
        "automatic_captions_en": {k: v for k, v in auto.items() if k.startswith("en")},
        "automatic_captions_language_count": len(auto),
        "subtitle_track_names": {
            lang: sorted({(e.get("name") or "") for e in entries})
            for lang, entries in (info.get("subtitles") or {}).items()
        },
    }
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(keep, f, indent=2, ensure_ascii=False)
    print(json.dumps({k: keep[k] for k in ("id", "channel", "channel_id", "upload_date", "duration", "title")}, ensure_ascii=False))
    print("subtitles:", sorted(subs.keys()))
    print("auto en:", sorted(keep["automatic_captions_en"].keys()))


if __name__ == "__main__":
    main()
