#pragma once

// DSi apps from the console's internal memory (NAND), shown in the game list after the
// inserted game card (TWLSettings::dsiAppsInList).
//
// The built-in apps (title/00030005: PictoChat, Download Play, Camera, Sound) and
// installed DSiWare (title/00030004) are listed by their NAND path, so the normal
// icon and title code reads their banners straight from NAND. NAND is only ever read.
// Launching boots the app through Unlaunch, as "exit to DSi Menu" does.

#include <string>
#include <string_view>
#include <vector>

// Mounts NAND as "nand:" the first time it's needed. Returns true if it's mounted.
bool nandMount(void);

// Paths of the apps to list ("nand:/title/00030005/484e4541/content/00000001.app").
// Empty when the setting is off or NAND apps can't be launched here.
const std::vector<std::string> &dsiAppPaths(void);

inline bool isDsiAppEntry(std::string_view name) {
	return name.substr(0, 12) == "nand:/title/";
}
