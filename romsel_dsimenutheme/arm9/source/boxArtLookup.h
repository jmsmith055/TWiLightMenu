#pragma once

#include <stddef.h>

// Finds a game's box art in _nds/TWiLightMenu/boxart.
//
// Checking for a file with access() searches the whole folder, because FAT folders
// aren't sorted. With thousands of images that takes seconds, and each game could need
// up to four checks. Instead, the folder's file names are read once (as hashes, 4 bytes
// each), so names that aren't there cost nothing and only a match touches the card.

// Writes the box art path for a file name (without the folder) and, if tid isn't null,
// the game's code, trying <filename>.bmp, <filename>.png, <tid>.bmp, then <tid>.png.
// Returns true if one exists. Otherwise path holds the last name tried, as before.
bool findBoxArt(char *path, size_t pathSize, const char *filename, const char *tid);
