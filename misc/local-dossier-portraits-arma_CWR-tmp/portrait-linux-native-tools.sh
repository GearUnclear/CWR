set -e
old=/mnt/d/Arma_CWA/tmp/local-dossier-portraits/arma_CWR/tmp/vcpkg-linux
native=/var/tmp/codex-dossier-vcpkg
if [ ! -d "$native/.git" ]; then git clone --quiet --shared "$old" "$native"; fi
if [ ! -d "$native/downloads/tools" ]; then mkdir -p "$native/downloads"; cp -a "$old/downloads/tools" "$native/downloads/"; fi
cp -n "$old"/downloads/*.gz "$old"/downloads/*.xz "$old"/downloads/*.zip "$native/downloads/" 2>/dev/null || true
cp "$old/vcpkg" "$native/vcpkg"
echo 'Native vcpkg checkout and tool cache ready.'
