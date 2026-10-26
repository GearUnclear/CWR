set -e
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
cd /mnt/d/Arma_CWA/tmp/local-dossier-portraits/arma_CWR
python3 tmp/portrait-stop-linux.py
export VCPKG_ROOT=/var/tmp/codex-dossier-vcpkg
export VCPKG_MAX_CONCURRENCY=8
cmake --preset linux-x64-clang-rwdi -B /var/tmp/codex-dossier-engine-build -DCMAKE_C_COMPILER_LAUNCHER= -DCMAKE_CXX_COMPILER_LAUNCHER= -DVCPKG_INSTALLED_DIR="$PWD/build/linux-x64-clang-rwdi/vcpkg_installed" '-DVCPKG_INSTALL_OPTIONS=--x-buildtrees-root=/var/tmp/codex-dossier-buildtrees;--x-packages-root=/var/tmp/codex-dossier-packages'
cmake --build /var/tmp/codex-dossier-engine-build --target PoseidonGame PoseidonTests -j 8
/var/tmp/codex-dossier-engine-build/tests/unit/engine/Poseidon/PoseidonTests '[portrait],[journal],[guerrilla]' --reporter compact
