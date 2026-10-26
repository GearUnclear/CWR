#!/bin/sh
V=../../v2/vanilla; U=../../v2/ud
m() { side=$1; clip=$2; boxes=$3; d=$V; [ $side = ud ] && d=$U; echo "== $side $clip"; python ../repro/metric.py $d/$clip/${clip}__${side}.mkv --boxes "$boxes" --track --start 0 --count 120 --out m8/${side}_${clip} --label ${side}_${clip} 2>&1 | tail -n +2; }
m vanilla outpost "920,560,300,440"
m vanilla ambush "730,705,145,200;1190,625,120,155;550,855,170,65"
m vanilla valley_mist "220,590,260,345;1525,685,125,215;1190,560,150,295"
m vanilla firefight "650,548,72,95;400,595,80,95;740,605,80,90;1120,535,70,95;1245,510,65,85;1465,545,75,95"
for s in vanilla ud; do
m $s opening "695,728,60,112;810,726,62,108"
m $s pine_shafts "730,688,80,110"
m $s sinai_camp "1090,478,200,200"
m $s town_patrol "1135,595,155,160;1455,610,100,150"
done
