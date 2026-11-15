#include "slot1Cart.h"

#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "common/flashcard.h"
#include "common/logging.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"
#include "ndsheaderbanner.h"
#include "read_card.h"

// Where the banner goes in the stand-in file (the header's bannerOffset points here).
static constexpr u32 STANDIN_BANNER_OFFSET = 0x1000;

static bool lastInserted = false; // card state seen by the last slot1CartPrepare()
static bool cardChecked = false;  // the inserted card has already been read
static bool cardListed = false;   // ...and it can be listed

const char *slot1CartPath(void) {
	return "sd:/_nds/TWiLightMenu/cache/slot1card.nds";
}

// The card can only be read here when running in DSi mode with SCFG access, from the
// SD card, and with no flashcard mounted (a flashcard has its own SD/Slot-1 switch).
static bool supported(void) {
	return ms().slot1CartInList && isDSiMode() && !sys().arm7SCFGLocked()
		&& sdFound() && !flashcardFound() && !ms().secondaryDevice;
}

static bool cardInserted(void) {
	return (REG_SCFG_MC & BIT(0)) == 0; // SCFG_MC bit 0: 0 = card inserted, 1 = ejected
}

// Writes the card's header and banner as a minimal .nds file, unless it already
// holds this card.
static bool writeStandIn(const sNDSHeaderExt &cardHeader) {
	sNDSHeaderExt header = cardHeader;

	FILE *existing = fopen(slot1CartPath(), "rb");
	if (existing) {
		sNDSHeaderExt old;
		const bool same = fread(&old, 1, sizeof(old), existing) == sizeof(old)
			&& memcmp(old.gameCode, header.gameCode, sizeof(header.gameCode)) == 0
			&& old.romversion == header.romversion
			&& old.headerCRC16 == header.headerCRC16;
		fclose(existing);
		if (same)
			return true;
	}

	static sNDSBannerExt banner;
	memset(&banner, 0, sizeof(banner));
	const bool hasBanner = header.bannerOffset != 0;
	if (hasBanner) {
		cardRead(header.bannerOffset, &banner, sizeof(banner), false);
		header.bannerOffset = STANDIN_BANNER_OFFSET;
	}

	mkdir("sd:/_nds/TWiLightMenu/cache", 0777);
	FILE *file = fopen(slot1CartPath(), "wb");
	if (!file)
		return false;

	static u8 padding[STANDIN_BANNER_OFFSET - sizeof(sNDSHeaderExt)];
	bool ok = fwrite(&header, 1, sizeof(header), file) == sizeof(header)
		&& fwrite(padding, 1, sizeof(padding), file) == sizeof(padding);
	if (ok && hasBanner)
		ok = fwrite(&banner, 1, sizeof(banner), file) == sizeof(banner);
	fclose(file);

	if (!ok)
		remove(slot1CartPath());
	return ok;
}

bool slot1CartPrepare(void) {
	if (!supported()) {
		cardChecked = cardListed = false;
		return false;
	}

	lastInserted = cardInserted();
	if (!lastInserted) {
		cardChecked = cardListed = false;
		return false;
	}
	if (cardChecked)
		return cardListed;

	// Read each inserted card once; it's read again only after being taken out.
	cardChecked = true;
	cardListed = false;

	const int err = cardInit();
	if (err == 0) {
		cardListed = writeStandIn(ndsCardHeader);
		logPrint("Slot-1 card %.4s %s\n", ndsCardHeader.gameCode, cardListed ? "listed" : "could not be listed");
	} else {
		logPrint("Slot-1 card could not be read (error %d)\n", err);
	}

	// Power off the card until it's launched, as the DS Classic menu does.
	disableSlot1();

	return cardListed;
}

bool slot1CartChanged(void) {
	return supported() && cardInserted() != lastInserted;
}
