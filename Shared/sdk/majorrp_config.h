//
// Shared/sdk/majorrp_config.h
//
// MAJOR RP fork settings. Change values here, rebuild, done.
// Based on Multi Theft Auto: San Andreas (GPL-3.0).
//
#pragma once

// Name shown in menus, window labels and dialogs
#define MAJORRP_PRODUCT_NAME "MAJOR RP"

// The only server this client connects to.
// First version: localhost. Later: a domain name, e.g. "play.majorrp.example".
#define MAJORRP_SERVER_HOST "127.0.0.1"
#define MAJORRP_SERVER_PORT 22003

// 1 = refuse connections to any other host/port (console "connect", mtasa:// links, redirects)
#define MAJORRP_LOCK_TO_SERVER 1

// 1 = never contact MTA's update servers (they would offer official MTA builds)
#define MAJORRP_DISABLE_UPDATER 1

// Registry key (HKLM) and C:\ProgramData folder, kept apart from stock MTA so both can be installed
#define MAJORRP_REGISTRY_PATH   "Software\\MAJOR RP All"
#define MAJORRP_COMMON_DATA_DIR "MAJOR RP All"

// 1 = an unzipped build sets itself up on first run (asks for admin once):
//     creates ProgramData folders and the registry key and lets normal users write to them
#define MAJORRP_PORTABLE_SETUP 1

// 1 = don't ask for admin on every start when MTA's closed anti-cheat service check fails.
//     That check never passes for this fork, so the prompts only repeated without fixing anything.
#define MAJORRP_NO_SERVICE_ADMIN_PROMPT 1

// 1 = hide the red "feature-branch build" bar that GitHub-built (CI) clients show at the top of the menu.
//     The small version label in the bottom-right corner stays.
#define MAJORRP_HIDE_TEST_BUILD_BANNER 1

// About window
#define MAJORRP_OWNER_NAME  "Danex"
#define MAJORRP_DISCORD_URL "discord.gg/majorrp"  // placeholder: put your real invite here
#define MAJORRP_SOURCE_URL  "github.com/danielsrasainis/MAJOR-RP"

// 1 = hide MTA's "The Latest News" headlines on the main menu
#define MAJORRP_HIDE_MENU_NEWS 1

// English only: no language button on the menu, Settings shows only English, locale forced to en_US
#define MAJORRP_ENGLISH_ONLY 1

// One GUI skin only (Shared/data/MTA San Andreas/skins/MAJOR RP); the skin chooser is hidden
#define MAJORRP_SINGLE_SKIN 1
#define MAJORRP_GUI_SKIN    "MAJOR RP"

// Don't show the Discord Rich Presence data-sharing question on the menu
#define MAJORRP_NO_DISCORD_PROMPT 0

// Discord Rich Presence: Application ID from discord.com/developers and the name of the uploaded art asset.
// Until MAJOR RP has its own Discord app, the MTA app is used (shows the MTA logo).
#define MAJORRP_DISCORD_APP_ID     "468493322583801867"
#define MAJORRP_DISCORD_ASSET      "mta_logo_round"
#define MAJORRP_DISCORD_ASSET_TEXT "MAJOR RP"

// Main menu v2: pill buttons with hover highlight, hover/click sounds, slow background pan, logo fade-in
#define MAJORRP_MENU_V2 1

// 1 = slow zoom/pan of the menu background (menu v2). Off: Danex prefers a still background.
#define MAJORRP_MENU_BG_PAN 0

// 1 = in the ESC menu while connected, DISCONNECT replaces PLAY (menu: Disconnect, Settings, About, Quit)
#define MAJORRP_INGAME_HIDE_PLAY 1
