cd D:/Arma_CWA_worktrees/graphics-remaster/tmp/video
for c in cloud_shadows pine_shafts convoy sinai_camp; do
  for v in "new:v2/ud/$c/${c}__ud.mkv" "orig:mist/backup_ud/$c/${c}__ud.mkv" "percell:mist/percell_ud/$c/${c}__ud.mkv" "van:v2/vanilla/$c/${c}__vanilla.mkv"; do
    tag=${v%%:*}; f=${v#*:}; [ -f "$f" ] || continue
    echo "== $c $tag $(python fogwind/../mist/tools/bandmetric.py $f --band 0,17,0,31)"
    python fogwind/orch/fogmotion_any.py --from 100 --to 260 --regions mid,upper "$f" 2>&1 | tail -4
  done
done
echo ALLDONE
