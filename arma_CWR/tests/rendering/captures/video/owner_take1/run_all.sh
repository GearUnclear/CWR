cd D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/vo_owner/music
M="C:/Users/Dane/Downloads/ANGEL DUST DEALERS - ROMANTIC COLLECTION/ANGEL DUST DEALERS - ROMANTIC COLLECTION"
F=D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/final
python add_music.py --voice D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/vo_owner/take1/take1_norm.wav \
  --music "$M - 03 Crisping Lover's Theme.ogg" --name uslu_dur_part1_graphics_owner_take1_music --subs owner --sbs > owner.log 2>&1
echo "owner exit=$?"
python add_music.py --voice $F/uslu_dur_part1_graphics_sarah_youtube_1080p60.mp4 \
  --music "$M - 01 Preamble- Lost Angeles (1988).ogg" --name uslu_dur_part1_graphics_sarah_music > sarah.log 2>&1
echo "sarah exit=$?"
python add_music.py --voice $F/uslu_dur_part1_graphics_lily_youtube_1080p60.mp4 \
  --music "$M - 01 Preamble- Lost Angeles (1988).ogg" --name uslu_dur_part1_graphics_lily_music > lily.log 2>&1
echo "lily exit=$?"
