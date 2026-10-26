#!/bin/sh
# mkvar.sh <name> [key=value ...]: sources in work/src_<name>, override GLSL in work/glsl_<name>
W=D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/puddle/3/work
N=$1; shift
rm -rf $W/src_$N $W/dump_$N $W/glsl_$N
mkdir -p $W/src_$N $W/glsl_$N
cp $W/src_base/*.cpp $W/src_$N/
python $W/apply.py $W/src_$N "$@" || exit 1
cd D:/Arma_CWA_worktrees/look-puddle-3/arma_CWR/tests/rendering/lighting_lab
python glsl_extract.py --source $W/src_$N/EngineGL33_PostSsrShaders.cpp --dump $W/dump_$N >/dev/null
python glsl_extract.py --source $W/src_$N/EngineGL33_LightingShadersSurface.cpp --dump $W/dump_$N >/dev/null
cp $W/dump_$N/psRemasterCommon.glsl $W/dump_$N/psPostSsrTrace.glsl $W/glsl_$N/
rm -rf $W/dump_$N
ls $W/glsl_$N
