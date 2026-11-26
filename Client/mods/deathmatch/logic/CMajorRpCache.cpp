/*****************************************************************************
 *
 *  PROJECT:     MAJOR RP (based on Multi Theft Auto: San Andreas)
 *  FILE:        mods/deathmatch/logic/CMajorRpCache.cpp
 *  PURPOSE:     Scrambled resource cache
 *
 *  Why: downloaded server files (client scripts, .txd/.dff/.col/.ifp models) used to sit in
 *  mods/deathmatch/resources as plain files anyone could copy. They are now stored scrambled
 *  with a key that is random per PC, and unscrambled only in memory when the game loads them.
 *  Files stay in the cache (no re-download). Copying the cache to another PC gives unreadable
 *  files there. The scramble is a fast XOR keystream: about the cost of reading the file.
 *
 *  It stops casual copying and model rippers, not someone who reverse engineers the client.
 *  Unreadable/old files are handled by the normal checksum check: they get downloaded again.
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CMajorRpCache.h"
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

namespace
{
    const char   MAGIC[] = "MRPC\x01\x00\x00\x00";  // 8 byte header, version 1
    const size_t MAGIC_LEN = 8;

    unsigned char g_table[4096];
    bool          g_bTableReady = false;

    void InitTable()
    {
        if (g_bTableReady)
            return;

        // Per-PC key, made once and kept in the MAJOR RP settings in the registry
        SString strKey = GetApplicationSetting("majorrp", "cache-key");
        if (strKey.length() != 64)
        {
            unsigned char raw[32];
            if (BCryptGenRandom(nullptr, raw, sizeof(raw), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
                for (int i = 0; i < 32; i++)
                    raw[i] = static_cast<unsigned char>(rand() ^ (GetTickCount32() >> (i % 24)));
            strKey.clear();
            for (unsigned char c : raw)
                strKey += SString("%02x", c);
            SetApplicationSetting("majorrp", "cache-key", strKey);
        }

        // Expand the key into a 4 KB table with xorshift128
        uint32_t s[4] = {};
        for (int i = 0; i < 4; i++)
            s[i] = strtoul(strKey.substr(i * 8, 8).c_str(), nullptr, 16) ^ (0x9E3779B9u * (i + 1));
        for (int i = 0; i < 4; i++)
            if (!s[i])
                s[i] = 0x6D2B79F5u + i;
        for (size_t i = 0; i < sizeof(g_table); i += 4)
        {
            uint32_t t = s[3];
            t ^= t << 11;
            t ^= t >> 8;
            s[3] = s[2];
            s[2] = s[1];
            s[1] = s[0];
            t ^= s[0] ^ (s[0] >> 19);
            s[0] = t;
            memcpy(&g_table[i], &t, 4);
        }
        g_bTableReady = true;
    }

    // XOR is its own inverse: the same call scrambles and unscrambles
    void Xor(char* pData, size_t uiSize)
    {
        InitTable();
        for (size_t i = 0; i < uiSize; i++)
            pData[i] ^= g_table[i & 4095] ^ static_cast<unsigned char>((i >> 12) * 0x9D);
    }

    bool HasMagic(const char* pData, size_t uiSize)
    {
        return uiSize >= MAGIC_LEN && memcmp(pData, MAGIC, MAGIC_LEN) == 0;
    }
}  // namespace

bool MajorRpCacheIsScrambleType(const SString& strPath)
{
    const SString strExt = ExtractExtension(strPath).ToLower();
    return strExt == "lua" || strExt == "luac" || strExt == "txd" || strExt == "dff" || strExt == "col" || strExt == "ifp";
}

bool MajorRpCacheLoad(const SString& strPath, SString& outData)
{
    if (!FileLoad(strPath, outData))
        return false;
    if (HasMagic(outData.data(), outData.size()))
    {
        outData.erase(0, MAGIC_LEN);
        Xor(&outData[0], outData.size());
    }
    return true;
}

bool MajorRpCacheLoad(const SString& strPath, std::vector<char>& outData)
{
    if (!FileLoad(strPath, outData))
        return false;
    if (HasMagic(outData.data(), outData.size()))
    {
        outData.erase(outData.begin(), outData.begin() + MAGIC_LEN);
        if (!outData.empty())
            Xor(outData.data(), outData.size());
    }
    return true;
}

bool MajorRpCacheIsScrambledFile(const SString& strPath)
{
    SString strHead;
    return FileLoad(strPath, strHead, MAGIC_LEN) && HasMagic(strHead.data(), strHead.size());
}

CChecksum MajorRpCacheChecksum(const SString& strPath)
{
    if (!MajorRpCacheIsScrambledFile(strPath))
        return CChecksum::GenerateChecksumFromFileUnsafe(strPath);
    SString strData;
    if (!MajorRpCacheLoad(strPath, strData))
        return CChecksum();
    return CChecksum::GenerateChecksumFromBuffer(strData.data(), strData.size());
}

void MajorRpCacheScrambleFile(const SString& strPath)
{
    if (!MajorRpCacheIsScrambleType(strPath))
        return;
    SString strData;
    if (!FileLoad(strPath, strData) || HasMagic(strData.data(), strData.size()))
        return;
    if (!strData.empty())
        Xor(&strData[0], strData.size());
    // Write next to it, then swap, so a crash never leaves a half-written file
    const SString strTemp = strPath + ".mrptmp";
    if (FileSave(strTemp, SStringX(MAGIC, MAGIC_LEN) + strData) && FileDelete(strPath) && FileRename(strTemp, strPath))
        return;
    FileDelete(strTemp);
}
