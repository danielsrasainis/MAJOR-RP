/*****************************************************************************
 *
 *  PROJECT:     MAJOR RP (based on Multi Theft Auto: San Andreas)
 *  FILE:        mods/deathmatch/logic/CMajorRpGuard.cpp
 *  PURPOSE:     Report injected DLLs and running cheat tools to the server
 *
 *  Why: this fork can't use MTA's closed anti-cheat service, so the client reports
 *  suspicious things itself. It only REPORTS (server event "majorrp:acReport", handled by
 *  the server's anticheat resource, which warns admins and logs). It never kicks or blocks,
 *  because overlays (Discord, Steam, NVIDIA, OBS, ...) also inject DLLs and must keep working.
 *
 *****************************************************************************/

#include "StdInc.h"
#include <psapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "psapi.lib")

namespace
{
    // DLLs from normal software that injects into games. Lower case, file name only.
    const char* const g_szKnownGoodDlls[] = {
        "discordhook.dll",     "discord_overlay.dll", "gameoverlayrenderer.dll",
        "steamclient.dll",     "nvspcap.dll",         "nvd3dum.dll",
        "nvldumd.dll",         "nvwgf2um.dll",        "nvapi.dll",
        "nvcuda.dll",          "aticfx32.dll",        "atiumdag.dll",
        "atiuxpag.dll",        "amdxc32.dll",         "igdumdim32.dll",
        "igd9dxva32.dll",      "rtsshooks.dll",       "rtsshooks64.dll",
        "graphics-hook32.dll", "obs-vulkan32.dll",    "action_x86.dll",
        "bdcamvk32.dll",       "fraps32.dll",         "overwolf.dll",
        "rzchromasdk.dll",     "medal-hook32.dll",
    };

    // Window titles / process names of common cheat tools (lower case, substring match on process name)
    const char* const g_szCheatTools[] = {"cheatengine",   "cheat engine",   "artmoney",        "ollydbg", "x32dbg",  "x64dbg",
                                          "processhacker", "systeminformer", "extremeinjector", "xenos",   "injector"};

    std::set<SString> g_reported;
    unsigned long     g_ulNextScan = 0;

    bool IsUnderDir(const SString& strPath, const SString& strDir)
    {
        if (strDir.empty())
            return false;
        SString a = strPath.ToLower().Replace("/", "\\");
        SString b = strDir.ToLower().Replace("/", "\\").TrimEnd("\\") + "\\";
        return a.substr(0, b.length()) == b;
    }

    void Report(const char* szKind, const SString& strDetails)
    {
        if (!g_reported.insert(SString("%s|%s", szKind, *strDetails)).second)
            return;  // once per session
        CClientPlayer* pLocal = g_pClientGame ? g_pClientGame->GetLocalPlayer() : nullptr;
        if (!pLocal)
            return;
        CLuaArguments args;
        args.PushString(szKind);
        args.PushString(strDetails);
        CStaticFunctionDefinitions::TriggerServerEvent("majorrp:acReport", *pLocal, args);
    }

    void ScanModules()
    {
        const SString strGta = GetLaunchPath();    // gta_sa.exe folder
        const SString strMta = CalcMTASAPath("");  // MAJOR RP install folder
        const SString strWin = GetSystemWindowsPath();

        HMODULE hMods[1024];
        DWORD   cbNeeded = 0;
        if (!EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded))
            return;
        for (DWORD i = 0; i < std::min<DWORD>(cbNeeded / sizeof(HMODULE), 1024); i++)
        {
            wchar_t wszPath[MAX_PATH] = {};
            if (!GetModuleFileNameExW(GetCurrentProcess(), hMods[i], wszPath, MAX_PATH))
                continue;
            SString strPath = ToUTF8(wszPath);
            if (IsUnderDir(strPath, strWin) || IsUnderDir(strPath, strMta) || IsUnderDir(strPath, strGta))
                continue;
            SString strName = ExtractFilename(strPath).ToLower();
            bool    bKnown = false;
            for (const char* szGood : g_szKnownGoodDlls)
                bKnown |= strName == szGood;
            if (!bKnown)
                Report("dll", strPath);
        }
    }

    void ScanProcesses()
    {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE)
            return;
        PROCESSENTRY32W pe = {sizeof(pe)};
        for (BOOL bOk = Process32FirstW(hSnap, &pe); bOk; bOk = Process32NextW(hSnap, &pe))
        {
            SString strName = ToUTF8(pe.szExeFile).ToLower();
            for (const char* szTool : g_szCheatTools)
                if (strName.find(szTool) != std::string::npos)
                    Report("tool", strName);
        }
        CloseHandle(hSnap);
    }
}  // namespace

// Called from CClientGame::DoPulses while joined. First scan 20s after joining, then every 2 minutes.
void MajorRpGuardPulse()
{
    const unsigned long ulNow = GetTickCount32();
    if (g_ulNextScan == 0)
    {
        g_ulNextScan = ulNow + 20000;
        return;
    }
    if (ulNow < g_ulNextScan)
        return;
    g_ulNextScan = ulNow + 120000;
    ScanModules();
    ScanProcesses();
}

// Called when leaving a server, so the next server gets its own reports
void MajorRpGuardReset()
{
    g_reported.clear();
    g_ulNextScan = 0;
}
