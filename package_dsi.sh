#!/bin/bash
# Builds a DSi-only SD card package from 7zfile/, mirroring the "DSi 7z" step in
# .github/workflows/release.yml. Run "make package" first.
#
# Output:
#   dsi-package/TWiLightMenu-DSi/      files to copy to the root of the DSi's SD card
#   dsi-package/TWiLightMenu-DSi.7z    the same, archived
#
# Set NDS_BOOTSTRAP=1 to also download the latest nds-bootstrap release (needed to
# launch DS games), as official releases do.

set -euo pipefail
cd "$(dirname "$0")"

SRC=7zfile
OUT=dsi-package
STAGE="$OUT/TWiLightMenu-DSi"

if [ ! -f "$SRC/_nds/TWiLightMenu/dsimenu.srldr" ] || [ ! -f "$SRC/DSi&3DS - SD card users/BOOT.NDS" ]; then
	echo "error: $SRC/ is missing build output. Run \"make package\" first." >&2
	exit 1
fi

rm -rf "$OUT"
mkdir -p "$STAGE"

# Start from the full package, without the debug symbols or the empty roms placeholder.
cp -r "$SRC"/. "$STAGE"/
rm -rf "$STAGE/debug" "$STAGE/roms"

cd "$STAGE"

# Merge the DSi-specific folders into the SD card root.
cp -r "DSi - CFW users/SDNAND root"/. .
cp -r "DSi - SD card users"/. .
cp -r "DSi&3DS - SD card users"/. .

# Drop everything that isn't for the DSi (add-ons can be installed separately).
rm -rf "DSi - CFW users" "DSi - SD card users" "DSi&3DS - SD card users"
rm -rf "3DS - CFW users" "Flashcard users"
rm -rf "Better DSi Menu Music" "Extra UI Music" "Multimedia"
rm -rf "Virtual Console (DSi or Flashcard)" "Virtual Console (3DS, Minimal)" "Virtual Console (3DS, Full)"
rm -f _nds/GBARunner2_arm7dldi_3ds.nds _nds/GBARunner2_arm7dldi_nodsp_3ds.nds
rm -f _nds/TWiLightMenu/bootplg.srldr _nds/TWiLightMenu/gbaswitch.srldr
find . -name .gitkeep -delete

# Folders that official releases create so users know where things go.
mkdir -p _nds/nds-bootstrap \
	_nds/TWiLightMenu/3dsmenu/themes _nds/TWiLightMenu/akmenu/themes \
	_nds/TWiLightMenu/dsimenu/themes _nds/TWiLightMenu/r4menu/themes \
	_nds/TWiLightMenu/boxart _nds/TWiLightMenu/icons \
	_nds/TWiLightMenu/extras/fonts _nds/TWiLightMenu/gamesettings

twlVersion=$(git describe --tags 2>/dev/null || echo unknown)
bootstrapVersion="not included (install nds-bootstrap separately)"

if [ "${NDS_BOOTSTRAP:-0}" = "1" ]; then
	tmp=$(mktemp -d)
	trap 'rm -rf "$tmp"' EXIT
	echo "Downloading the latest nds-bootstrap release..."
	curl -fsSL -o "$tmp/nds-bootstrap.7z" https://github.com/DS-Homebrew/nds-bootstrap/releases/latest/download/nds-bootstrap.7z
	7z x -o"$tmp" "$tmp/nds-bootstrap.7z" >/dev/null
	cp "$tmp/release-bootstrap.ver" _nds/
	cp "$tmp/nds-bootstrap-release.nds" _nds/
	cp "$tmp/nds-bootstrap-hb-release.nds" _nds/
	bootstrapVersion=$(cat _nds/release-bootstrap.ver)
fi

printf "TWiLight Menu++: %s\nnds-bootstrap: %s\n\nRocketRobz, ahezard\n" "$twlVersion" "$bootstrapVersion" > version.txt

7z a "../TWiLightMenu-DSi.7z" . >/dev/null

echo "DSi package ready:"
echo "  $OUT/TWiLightMenu-DSi/    (copy its contents to the root of the SD card)"
echo "  $OUT/TWiLightMenu-DSi.7z"
