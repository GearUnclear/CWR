#!/bin/sh
V=../../v2/vanilla; U=../../v2/ud
m() { side=$1; clip=$2; shift 2; d=$V; [ $side = ud ] && d=$U; echo "== $side $clip"; H7TAG=${side}_${clip} python heads7.py measure $d/$clip/${clip}__${side}.mkv 0 180 "$@"; }
for s in vanilla; do
m $s outpost outpost_man1:1040,598,22:1140,710,30
m $s ambush ambA:815,720,12:770,770,18 ambB:1262,640,10:1230,695,14 ambProne:650,868,8:620,885,10
m $s valley_mist vmL:370,615,14:310,690,22 vmR:1605,698,12:1570,750,18 vmM:1250,605,12:1250,665,18
m $s firefight ff1:675,560,6:675,590,9 ff2:435,605,7:435,640,10 ff3:765,615,7:770,650,10 ff4:1170,555,6:1170,585,9 ff5:1275,525,6:1275,555,9 ff6:1500,560,6:1500,590,9
done
for s in vanilla ud; do
m $s opening op1:712,739,8:722,767,10 op2:837,736,8:852,765,10
m $s pine_shafts ps1:777,700,8:755,725,10
m $s sinai_camp sc1:1177,492,14:1235,540,22
m $s town_patrol tp1:1220,610,10:1190,655,14 tp2:1485,625,10:1505,665,14
done
m vanilla checkpoint ck1:835,375,6:835,395,8
