"""Agent 3: agent 1's fogmotion with regions for the pine_shafts static pose (camera held at k=300).
usage: python pinemotion.py <clip.mkv ...> --static --from 15"""
import sys
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit')
import fogmotion as fm
# y0, y1, x0, x1 fractions of 1920x1080, picked on look/pine_f300.png (the pose held)
fm.REGIONS = {
    "skyglow": (0.10, 0.36, 0.40, 0.56),   # the bright air behind the trees toward the sun
    "shafts": (0.38, 0.62, 0.30, 0.44),    # the sunbeams' area left of the centre tree
    "treehaze": (0.25, 0.50, 0.58, 0.85),  # the haze among the right-hand crowns
    "farair": (0.40, 0.55, 0.03, 0.12),    # the far air at the left edge
    "ground": (0.72, 0.95, 0.30, 0.90),    # the meadow in front (land, little air)
}
sys.argv = [sys.argv[0]] + sys.argv[1:] + ["--regions", ",".join(fm.REGIONS)]
fm.main()
