#pragma once

// The inserted game card, shown as the first entry of the game list
// (TWLSettings::slot1CartInList).
//
// When a card is inserted, its header and banner are copied into a small .nds file in
// the cache folder. That file is listed like any other game, so the normal icon, title
// and box art code shows it; launching it boots the card in Slot-1 instead.

#include <string_view>

// Reads the card's header and banner the first time it's seen. Returns true when
// the card should be listed (slot1CartPath() then holds its header and banner).
bool slot1CartPrepare(void);

// True when a card has been inserted or removed since the last slot1CartPrepare().
bool slot1CartChanged(void);

// Absolute path of the file that stands in for the card in the list.
const char *slot1CartPath(void);

inline bool isSlot1CartEntry(std::string_view name) {
	return name == slot1CartPath();
}
