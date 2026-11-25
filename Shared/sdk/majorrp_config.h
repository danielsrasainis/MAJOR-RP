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
