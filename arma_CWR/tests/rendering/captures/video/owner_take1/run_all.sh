# The music runs of 2026-10-23 (owner take 1, and the two ElevenLabs cuts), for the record.
# Set UD_MUSIC_DIR to the folder of the CC BY 4.0 album whose tracks the trailer uses (credits ship
# with the finished files); the work folder is UD_VIDEO_WORK (default <repo>/tmp/video/v2).
HERE=$(cd "$(dirname "$0")" && pwd)
W=${UD_VIDEO_WORK:-$HERE/../../../../../../tmp/video/v2}
: "${UD_MUSIC_DIR:?set UD_MUSIC_DIR to the album folder}"
M="$UD_MUSIC_DIR/ANGEL DUST DEALERS - ROMANTIC COLLECTION"
F=$W/final
mkdir -p "$W/vo_owner/music" && cd "$W/vo_owner/music" || exit 1
python "$HERE/add_music.py" --voice "$W/vo_owner/take1/take1_norm.wav" \
  --music "$M - 03 Crisping Lover's Theme.ogg" --name uslu_dur_part1_graphics_owner_take1_music --subs owner --sbs > owner.log 2>&1
echo "owner exit=$?"
python "$HERE/add_music.py" --voice "$F/uslu_dur_part1_graphics_sarah_youtube_1080p60.mp4" \
  --music "$M - 01 Preamble- Lost Angeles (1988).ogg" --name uslu_dur_part1_graphics_sarah_music > sarah.log 2>&1
echo "sarah exit=$?"
python "$HERE/add_music.py" --voice "$F/uslu_dur_part1_graphics_lily_youtube_1080p60.mp4" \
  --music "$M - 01 Preamble- Lost Angeles (1988).ogg" --name uslu_dur_part1_graphics_lily_music > lily.log 2>&1
echo "lily exit=$?"
