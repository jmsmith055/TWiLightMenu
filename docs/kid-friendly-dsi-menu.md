# Kid-friendly DSi menu

This fork adds three settings that make the Nintendo DSi, 3DS, SEGA Saturn and Homebrew Launcher UIs easier for young children to use. Normally these UIs look like a file browser, where you have to open folders to find a game. With the new settings, the menu shows one list:

1. the game card in Slot-1, if one is inserted;
2. the DSi's own apps, such as PictoChat and Nintendo DSi Camera;
3. every game on the SD card, in alphabetical order.

The fork also fixes a blank Slot-1 label in the SELECT menu, makes Docker builds more reliable, and adds a way to build a package with only the files the DSi needs.

## The new settings

The settings are on the **GUI** page of TWiLight Menu++ Settings, after **Directories/Folders**.

| Setting | Default | `settings.ini` key (`[SRLOADER]`) | What it does |
| --- | --- | --- | --- |
| All games in one list | Off | `FLAT_ROM_LIST` | Shows every game in the default folder and its subfolders in a single list. |
| Game card in list | On | `SLOT1_CART_IN_LIST` | Shows the inserted game card as the first entry. |
| DSi apps in list | On | `DSI_APPS_IN_LIST` | Shows the DSi's built-in apps and installed DSiWare after the game card. |

### All games in one list

When this is on:

- The list starts in the default ROM folder (`ROM_FOLDER`). If that folder is missing, it starts at the root of the SD card (or the flashcard).
- Games are listed from that folder and up to three levels of subfolders below it. All games are sorted together, using the menu's usual sort order.
- Folders aren't shown, and **B** doesn't go up a folder, so children can't get lost in the folder tree.
- These folders are never searched: names starting with `.` or `_` (including `_nds`), `saves`, `ramdisks`, `System Volume Information`, `Nintendo 3DS`, `DCIM`, `luma`, `3ds`, `gm9`, `cias`, `title`, `ticket`, `shared1`, `shared2`, `sys`, `tmp`, `import`, `progress`, `photo` and `private`.
- The list holds up to 1024 games when DSi features are available, and 512 otherwise. That's the same limit as a normal folder.

Each game still works as if it had been opened from its own folder:

- Saves go in the `saves` folder next to the game, as usual. For example, `Games/NintendoDS/game.nds` saves to `Games/NintendoDS/saves/game.sav`.
- Per-game settings, box art and custom icons are found by file name, as before, so existing files keep working.
- After a game is launched, `ROM_FOLDER` is set to that game's folder, and the list opens on that game next time.

Suggested SD card layout:

```
sd:/
├─ Games/            <- set as the default ROM folder
│  ├─ NintendoDS/
│  │  ├─ game.nds
│  │  └─ saves/
│  └─ GameBoyAdvance/
├─ Tools/            <- outside Games, so it isn't listed
└─ _nds/
```

Tools such as GodMode9i can go in a folder outside the default ROM folder, so they stay out of the children's list.

### Game card in list

The inserted game card is listed first. When no card is inserted, nothing is shown in its place. Inserting or removing a card refreshes the list straight away and moves the cursor back to the first entry.

- Pressing **A** on the card launches it, the same as **Launch Slot-1** in the SELECT menu.
- The card's icon and title come from its banner. The card's header and banner are copied to `sd:/_nds/TWiLightMenu/cache/slot1card.nds`, so the menu shows them like any other game. A card is read only once per insertion, and the file is rewritten only when a different card is inserted. The card is then switched off until it's launched.
- The card can't be hidden or deleted (**X**), and it has no per-game settings (**Y**).
- This needs DSi mode with SCFG access, running from the SD card with no flashcard mounted. That's the normal setup on a DSi with Unlaunch or hiyaCFW. On other setups, the setting does nothing.

### DSi apps in list

The DSi's apps are read from its internal memory (NAND) and listed after the game card:

- built-in apps from `nand:/title/00030005` (PictoChat, DS Download Play, Nintendo DSi Camera, Nintendo DSi Sound and others);
- installed DSiWare from `nand:/title/00030004`.

TWiLight Menu++ itself isn't listed. System Settings and the Nintendo DSi Shop are kept in a separate folder (`00030015`) and aren't listed either. Each app's file is found from its title metadata, and its icon and title come from its own banner. The internal memory is only ever read, never written. It's scanned once each time the menu starts.

Pressing **A** on an app boots it through Unlaunch, the same way **Exit to DSi Menu** boots the DSi Menu. Apps can't be hidden, deleted or given per-game settings.

This needs a retail DSi (not a 3DS or a development console) with Unlaunch installed, running in DSi mode with SCFG access from the SD card. On other setups, the setting does nothing.

### Setting up the kid-friendly menu

1. Put the games in one folder on the SD card, such as `sd:/Games`. Subfolders are fine.
2. In TWiLight Menu++ Settings, choose the **Nintendo DSi** UI (or 3DS, Saturn or Homebrew Launcher), then turn on **All games in one list**. **Game card in list** and **DSi apps in list** are already on.
3. Set the default ROM folder to that folder. You can do this by opening it once in the menu, or by setting `ROM_FOLDER = sd:/Games` in `sd:/_nds/TWiLightMenu/settings.ini`.
4. Optionally, use these existing settings to stop children from changing things:
   - **Prevent ROM hiding and deletion** (in Settings) stops **X** from hiding or deleting games.
   - Kiosk mode blocks **X**, **Y** and the settings menu. It can only be set in `settings.ini`: add `KIOSK_MODE = 1` under `[SRLOADER]`, and remove it to turn kiosk mode off.

## Other fixes

- **SELECT menu label:** in DS mode with SCFG enabled and no flashcard, the SELECT menu's Slot-1/SD entry could be selected but had no text. Its label is now drawn there too.
- **Deleting box art images:** file names are no longer used as `sprintf` format strings, and the paths are limited to `PATH_MAX`.

## Building

### Docker

`compile_docker.ps1` (Windows) and `compile_docker.sh` (Linux and macOS) build inside the `devkitpro/devkitarm:20241104` image, so the devkitARM version always matches. Both scripts take `make` arguments:

```
.\compile_docker.ps1 package
.\compile_docker.ps1 -C romsel_dsimenutheme
```

Changes from the original scripts:

- They stop with a clear message if Docker isn't installed or isn't running.
- They rebuild the image whenever the `Dockerfile` changes, instead of using an old image.
- They only ask for an interactive terminal when there is one, so they also work from other tools and CI.
- They return `make`'s exit code, so a failed build is reported as a failure.

The `Dockerfile` also fixes the image's package sources so `p7zip-full` and `python` install correctly, and it marks the mounted repository as a safe git folder.

### DSi-only package

`make package` builds files for every console and setup. To get only the files a DSi needs:

```
.\compile_docker.ps1 package
.\compile_docker.ps1 package-dsi
```

This runs `package_dsi.sh`, which follows the release workflow's DSi step and creates:

- `dsi-package/TWiLightMenu-DSi/`: copy its contents to the root of the DSi's SD card;
- `dsi-package/TWiLightMenu-DSi.7z`: the same files, archived.

To also download the latest nds-bootstrap, which is needed to launch DS games from the SD card, use `make package-dsi NDS_BOOTSTRAP=1`.

To update only the menu on a card that's already set up, copy `_nds/TWiLightMenu/dsimenu.srldr` and `_nds/TWiLightMenu/settings.srldr`.

## Source files

| File | Purpose |
| --- | --- |
| `romsel_dsimenutheme/arm9/source/fileBrowse.cpp` | Builds the flat list, adds the game card and DSi apps to the list, and handles launching. |
| `romsel_dsimenutheme/arm9/source/flatRomList.h` | Helpers for the `folder/game.nds` names used in the flat list. |
| `romsel_dsimenutheme/arm9/source/slot1Cart.cpp`, `.h` | Detects and reads the game card. |
| `romsel_dsimenutheme/arm9/source/dsiApps.cpp`, `.h` | Mounts NAND and finds the DSi's apps. |
| `romsel_dsimenutheme/arm9/source/iconTitle.cpp`, `perGameSettings.cpp` | Find icons, saves and per-game settings for games in the flat list. |
| `universal/.../twlmenusettings.*` | The three new settings. |
| `settings/arm9/source/main.cpp`, `language.inl` | The settings' entries in TWiLight Menu++ Settings. |
| `package_dsi.sh`, `Makefile` | The `package-dsi` target. |
| `Dockerfile`, `compile_docker.ps1`, `compile_docker.sh` | The Docker build. |

## Status

All of this builds. It hasn't been fully tested on DSi hardware yet. These still need checking:

- how fast the flat list loads with a large game collection;
- reading the game card and launching it from the list;
- booting each built-in app and DSiWare title directly through Unlaunch.

The new menu text is in English only, and it isn't on Crowdin yet.

This work was done with help from AI (Claude), and it should be reviewed and tested before it goes to the main TWiLight Menu++ project. That project's [AI usage guidelines](../AI-GUIDELINES.md) describe what's expected.
