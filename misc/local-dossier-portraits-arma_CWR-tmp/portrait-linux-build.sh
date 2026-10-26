set -e
cd /mnt/d/Arma_CWA/tmp/local-dossier-portraits/arma_CWR
export VCPKG_ROOT="$PWD/tmp/vcpkg-linux"
if [ ! -d "$VCPKG_ROOT/.git" ]; then git clone --quiet --no-checkout https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"; fi
git -C "$VCPKG_ROOT" checkout --quiet 170bd3bfb152a1795b67b5c2190ab7d899fc9971
"$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
export VCPKG_MAX_CONCURRENCY=8
cmake --preset linux-x64-clang-rwdi -DCMAKE_C_COMPILER_LAUNCHER= -DCMAKE_CXX_COMPILER_LAUNCHER=
cmake --build build/linux-x64-clang-rwdi --target PoseidonGame PoseidonTests -j 8
build/linux-x64-clang-rwdi/tests/unit/engine/Poseidon/PoseidonTests '[portrait],[journal],[guerrilla]' --reporter compact
