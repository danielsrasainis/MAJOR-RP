# MAJOR RP client (fork of Multi Theft Auto: San Andreas)

MAJOR RP is a fork of [Multi Theft Auto: San Andreas](https://github.com/multitheftauto/mtasa-blue),
licensed under GPL-3.0. The original copyright and license (LICENSE) apply to this code.

## What this fork changes

| Area | File | Change |
|---|---|---|
| Settings in one place | `Shared/sdk/majorrp_config.h` | Name, server address/port, lock, updater switch, registry/ProgramData names |
| Build type | `Shared/sdk/version.h` | `VERSION_TYPE_UNSTABLE` (fork netcode, can't mix with stock MTA) |
| Main menu | `Client/core/CMainMenu.cpp` | One "CONNECT TO MAJOR" button + Settings, About, Quit. No server browser, Host Game, Map Editor |
| Connect lock | `Client/core/CConnectManager.cpp` | Refuses any host/port other than the configured one |
| Updater | `Client/core/CVersionUpdater.cpp` | Never contacts MTA update servers |
| Side-by-side install | `Shared/sdk/SharedUtil.Misc.hpp`, installers | Registry `HKLM\Software\MAJOR RP All`, `C:\ProgramData\MAJOR RP All` |
| Branding | `CGUI.cpp`, `CCredits.cpp`, `CTrayIcon.cpp`, `loader.rc`, menu images | MAJOR RP name and placeholder artwork |
| Installer | `Shared/installer/nightly.nsi` | Product name, install folder, does not take over `mtasa://` |
| Server | `Server/mods/deathmatch/mtaserver.conf` | `disableac 5,6,21`, name MAJOR RP, not announced to the MTA server list |

## Localhost test (first milestone)

You need: Windows 10/11, Visual Studio 2026 (Desktop C++ + "C++ MFC for latest build tools (x86 & x64)"),
Git, and an installed copy of GTA: San Andreas (v1.0 US/EU, or the Steam copy, which MTA downgrades).

```bat
git clone <this repo> majorrp
cd majorrp
win-install-data.bat          :: downloads net/netc dlls, CEF, data files. Answer "y" to install resources
win-create-projects.bat       :: generates Build\MTASA.sln
win-build.bat Release Win32   :: or open Build\MTASA.sln, pick Release / Win32, Build Solution
```

Output goes to `Bin\`.

1. Copy your gamemode resources into `Bin\server\mods\deathmatch\resources\` (and your `acl.xml`, database files).
2. Start the server: `Bin\server\MTA Server.exe`. It listens on port 22003.
3. Start the client: `Bin\Multi Theft Auto.exe`. On first start it asks for the GTA: San Andreas folder.
4. Click **CONNECT TO MAJOR**. It connects to `127.0.0.1:22003`.

Server and client must be built from the same commit, otherwise the network versions differ.

## Going public later

1. Change `MAJORRP_SERVER_HOST` in `Shared/sdk/majorrp_config.h` to your domain, and rebuild.
2. Replace `Bin\mta\netc.dll` with the fork-support one: `https://mirror-cdn.multitheftauto.com/bdata/fork-support/netc.dll`.
3. Build the **Nightly** configuration.
4. Build the server for your VPS (Linux: `./linux-build.sh --config=release`) from the same commit.
5. Build the installer with NSIS from `Shared/installer/nightly.nsi`, and sign the exe/dlls/installer.
6. Publish this source (GPL-3.0) and link it from the About screen and download page.

## Known limits of this first version

- The exe is still called `Multi Theft Auto.exe`, the game window title and crash dialogs still say "MTA: San Andreas", and the installer text still mentions Multi Theft Auto in places. Renaming touches the launcher, loader, crash handler and installer together, so it's a separate step.
- Artwork is a placeholder (logo, background, button). Replace the PNGs in `Shared/data/MTA San Andreas/MTA/cgui/images/` and `.../locale/en_US/menu_connect_major.png` (white text, 34 px high, max 390 px wide).
- The anti-cheat for forks is weak. Plan your own server-side checks.
- Not compiled yet: these changes were written against mtasa-blue commit 55e4f6d without a Windows build.

## Updating an existing server

The download ships the server settings as `*.default` files in `server\mods\deathmatch\`
(`mtaserver.conf.default`, `acl.xml.default`, ...). Copying the zip over an existing server never
overwrites your own `mtaserver.conf`, `acl.xml` or other settings.
On a brand-new server, rename those files once to drop `.default`.
