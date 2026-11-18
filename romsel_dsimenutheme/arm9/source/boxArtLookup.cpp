#include "boxArtLookup.h"

#include <nds.h>
#include <dirent.h>
#include <stdio.h>
#include <algorithm>
#include <vector>

#include "common/logging.h"
#include "common/systemdetails.h"

static const char *boxArtFolder(void) {
	return sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/boxart" : "fat:/_nds/TWiLightMenu/boxart";
}

// FNV-1a of the name, ignoring ASCII case like FAT does. part1 and part2 are hashed as
// one string, so "game" + ".png" matches a folder entry "Game.PNG".
static u32 nameHash(const char *part1, const char *part2 = "") {
	u32 hash = 2166136261u;
	for (const char *part : {part1, part2}) {
		for (const char *c = part; *c; c++) {
			char ch = *c;
			if (ch >= 'A' && ch <= 'Z')
				ch += 'a' - 'A';
			hash = (hash ^ (u8)ch) * 16777619u;
		}
	}
	return hash;
}

// Sorted hashes of the box art folder's file names, read on first use. Box art is only
// added from a PC, so the folder can't gain files while the menu is running.
static const std::vector<u32> &boxArtNames(void) {
	static std::vector<u32> names;
	static bool read = false;
	if (!read) {
		read = true;
		DIR *dir = opendir(boxArtFolder());
		if (dir) {
			while (dirent *entry = readdir(dir)) {
				if (entry->d_type != DT_DIR)
					names.push_back(nameHash(entry->d_name));
			}
			closedir(dir);
		}
		std::sort(names.begin(), names.end());
		logPrint("Box art images found: %d\n", (int)names.size());
	}
	return names;
}

static bool tryName(char *path, size_t pathSize, const char *name, const char *ext) {
	snprintf(path, pathSize, "%s/%s%s", boxArtFolder(), name, ext);
	const std::vector<u32> &names = boxArtNames();
	// A hash collision only means drawing fails as if the file were missing.
	return std::binary_search(names.begin(), names.end(), nameHash(name, ext));
}

bool findBoxArt(char *path, size_t pathSize, const char *filename, const char *tid) {
	if (tryName(path, pathSize, filename, ".bmp") || tryName(path, pathSize, filename, ".png"))
		return true;
	if (tid && (tryName(path, pathSize, tid, ".bmp") || tryName(path, pathSize, tid, ".png")))
		return true;
	return false;
}
