#pragma once

// "All games in one list" (TWLSettings::flatRomList).
//
// In this mode the menu stays in one root folder and every game below it is listed
// together. Each list entry's name is then a path relative to that root, such as
// "Pokemon/game.nds"; in the normal per-folder list a name never contains a '/'.
// These helpers split such a name. Both return views into the string they were given,
// and flatBaseName() always ends where that string ends, so its data() is
// NUL-terminated whenever the input is.

#include <string>
#include <string_view>

inline std::string_view flatBaseName(std::string_view path) {
	const size_t slash = path.rfind('/');
	return slash == std::string_view::npos ? path : path.substr(slash + 1);
}

// The folder part with its trailing '/' ("Pokemon/"), or "" for a game in the root.
inline std::string_view flatDirPrefix(std::string_view path) {
	const size_t slash = path.rfind('/');
	return slash == std::string_view::npos ? std::string_view() : path.substr(0, slash + 1);
}

// Path of a game's companion file in a folder next to it, without an extension:
// ("Pokemon/game.nds", "saves") gives "Pokemon/saves/game", and ("game.nds", "saves")
// gives "saves/game" as before.
inline std::string flatCompanionPathNoExt(std::string_view path, std::string_view folder) {
	const std::string_view base = flatBaseName(path);
	std::string result(flatDirPrefix(path));
	result += folder;
	result += '/';
	result += base.substr(0, base.rfind('.'));
	return result;
}
