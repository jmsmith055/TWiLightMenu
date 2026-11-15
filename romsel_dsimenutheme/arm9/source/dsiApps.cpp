#include "dsiApps.h"

#include <nds.h>
#include <fat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "common/flashcard.h"
#include "common/logging.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"
#include "nand/nandio.h"

bool nandMount(void) {
	static bool nandInited = false;
	static bool nandMounted = false;
	if (!nandInited) {
		nandMounted = fatMountSimple("nand", &io_dsi_nand) && (access("nand:/", F_OK) == 0);
		nandInited = true;
	}
	return nandMounted;
}

// Apps are booted with Unlaunch, which needs a DSi (not a 3DS or a dev unit) running in
// DSi mode with SCFG access, from the SD card, with Unlaunch installed.
static bool supported(void) {
	return ms().dsiAppsInList && isDSiMode() && !sys().arm7SCFGLocked()
		&& ms().consoleModel == TWLSettings::EDSiRetail && ms().launcherApp != -1
		&& sdFound() && !ms().secondaryDevice;
}

// Finds a title's app file in its content folder. The title's TMD names the boot
// content; if it can't be read, the first .app file is used.
static bool findTitleApp(const std::string &titleFolder, std::string &appPath) {
	const std::string contentFolder = titleFolder + "/content";

	FILE *tmd = fopen((contentFolder + "/title.tmd").c_str(), "rb");
	if (tmd) {
		u8 id[4];
		const bool read = fseek(tmd, 0x1E4, SEEK_SET) == 0 && fread(id, 1, sizeof(id), tmd) == sizeof(id);
		fclose(tmd);
		if (read) {
			char name[16];
			snprintf(name, sizeof(name), "/%02x%02x%02x%02x.app", id[0], id[1], id[2], id[3]);
			appPath = contentFolder + name;
			if (access(appPath.c_str(), F_OK) == 0)
				return true;
		}
	}

	DIR *dir = opendir(contentFolder.c_str());
	if (!dir)
		return false;
	bool found = false;
	while (dirent *entry = readdir(dir)) {
		const size_t len = strlen(entry->d_name);
		if (entry->d_type != DT_DIR && len > 4 && strcasecmp(entry->d_name + len - 4, ".app") == 0) {
			appPath = contentFolder + "/" + entry->d_name;
			found = true;
			break;
		}
	}
	closedir(dir);
	return found;
}

static void addTitles(std::vector<std::string> &paths, const char *titleIdHigh) {
	const std::string folder = std::string("nand:/title/") + titleIdHigh;
	DIR *dir = opendir(folder.c_str());
	if (!dir)
		return;

	std::vector<std::string> titleFolders;
	while (dirent *entry = readdir(dir)) {
		if (entry->d_type != DT_DIR || strlen(entry->d_name) != 8)
			continue;
		if (strcasecmp(entry->d_name, "53524c41") == 0) // "SRLA": TWiLight Menu++ itself
			continue;
		titleFolders.push_back(folder + "/" + entry->d_name);
	}
	closedir(dir);

	for (const std::string &titleFolder : titleFolders) {
		std::string appPath;
		if (findTitleApp(titleFolder, appPath)) {
			logPrint("DSi app listed: %s\n", appPath.c_str());
			paths.push_back(std::move(appPath));
		}
	}
}

const std::vector<std::string> &dsiAppPaths(void) {
	static std::vector<std::string> paths;
	static bool scanned = false;

	if (!supported()) {
		static const std::vector<std::string> none;
		return none;
	}

	// NAND's titles can't change while the menu is running, so look once per start.
	if (!scanned) {
		scanned = true;
		if (nandMount()) {
			addTitles(paths, "00030005"); // Built-in apps
			addTitles(paths, "00030004"); // Installed DSiWare
		}
	}
	return paths;
}
