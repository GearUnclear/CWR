set -e
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
cd /mnt/d/Arma_CWA/tmp/local-dossier-portraits/arma_CWR
cmake --build /var/tmp/codex-dossier-engine-build --target PoseidonGame PoseidonTests -j 8
/var/tmp/codex-dossier-engine-build/tests/unit/engine/Poseidon/PoseidonTests '[portrait],[journal],[guerrilla]' --reporter compact
