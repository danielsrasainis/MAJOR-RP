/*****************************************************************************
 *
 *  PROJECT:     MAJOR RP (based on Multi Theft Auto: San Andreas)
 *  FILE:        Shared/sdk/majorrp_cache_core.h
 *  PURPOSE:     Scrambled resource cache: format shared by every client module
 *               (deathmatch writes the files; deathmatch, the GUI and the browser read them)
 *
 *  File layout: "MRPC\x01\0\0\0" + contents XOR keystream. The key is random per PC and kept in
 *  the MAJOR RP application settings ("majorrp" / "cache-key", 64 hex characters).
 *
 *****************************************************************************/
#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

namespace MajorRpCacheCore
{
    constexpr char   MAGIC[] = "MRPC\x01\x00\x00\x00";  // 8 byte header, version 1
    constexpr size_t MAGIC_LEN = 8;

    inline bool HasMagic(const char* pData, size_t uiSize)
    {
        return pData && uiSize >= MAGIC_LEN && memcmp(pData, MAGIC, MAGIC_LEN) == 0;
    }

    // Expand the 64 hex character key into a 4 KB table with xorshift128
    inline void BuildTable(const std::string& strKey, unsigned char* pTable4096)
    {
        uint32_t s[4] = {};
        for (int i = 0; i < 4; i++)
            s[i] = strtoul(strKey.substr(i * 8, 8).c_str(), nullptr, 16) ^ (0x9E3779B9u * (i + 1));
        for (int i = 0; i < 4; i++)
            if (!s[i])
                s[i] = 0x6D2B79F5u + i;
        for (size_t i = 0; i < 4096; i += 4)
        {
            uint32_t t = s[3];
            t ^= t << 11;
            t ^= t >> 8;
            s[3] = s[2];
            s[2] = s[1];
            s[1] = s[0];
            t ^= s[0] ^ (s[0] >> 19);
            s[0] = t;
            memcpy(&pTable4096[i], &t, 4);
        }
    }

    // XOR is its own inverse: the same call scrambles and unscrambles
    inline void Xor(const unsigned char* pTable4096, char* pData, size_t uiSize)
    {
        for (size_t i = 0; i < uiSize; i++)
            pData[i] ^= pTable4096[i & 4095] ^ static_cast<unsigned char>((i >> 12) * 0x9D);
    }

    // Read side for modules that never create the key. pData is unscrambled in place (header skipped);
    // returns the offset where the real contents start (0 = not scrambled or no key on this PC).
    // GetKey is called only for scrambled data.
    template <class TGetKey>
    inline size_t UnscrambleInPlace(char* pData, size_t uiSize, TGetKey GetKey)
    {
        if (!HasMagic(pData, uiSize))
            return 0;
        static unsigned char table[4096];
        static bool          bReady = false;
        if (!bReady)
        {
            const std::string strKey = GetKey();
            if (strKey.length() != 64)
                return 0;
            BuildTable(strKey, table);
            bReady = true;
        }
        Xor(table, pData + MAGIC_LEN, uiSize - MAGIC_LEN);
        return MAGIC_LEN;
    }
}  // namespace MajorRpCacheCore
