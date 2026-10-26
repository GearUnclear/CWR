set -e
cd /mnt/d/Arma_CWA/tmp/local-dossier-portraits/arma_CWR
export VCPKG_ROOT="$PWD/tmp/vcpkg-linux"
export VCPKG_MAX_CONCURRENCY=8
cmake --preset linux-x64-clang-rwdi -DCMAKE_C_COMPILER_LAUNCHER= -DCMAKE_CXX_COMPILER_LAUNCHER= '-DVCPKG_INSTALL_OPTIONS=--x-buildtrees-root=/tmp/dossier-vcpkg-buildtrees;--x-packages-root=/tmp/dossier-vcpkg-packages'
cmake --build build/linux-x64-clang-rwdi --target PoseidonGame PoseidonTests -j 8
build/linux-x64-clang-rwdi/tests/unit/engine/Poseidon/PoseidonTests '[portrait],[journal],[guerrilla]' --reporter compact
