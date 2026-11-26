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
    using MajorRpCacheCore::HasMagic;
    using MajorRpCacheCore::MAGIC;
    using MajorRpCacheCore::MAGIC_LEN;

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
        MajorRpCacheCore::BuildTable(strKey, g_table);
        g_bTableReady = true;
    }

    void Xor(char* pData, size_t uiSize)
    {
        InitTable();
        MajorRpCacheCore::Xor(g_table, pData, uiSize);
    }

    // Read-only file in memory, so fileOpen/fileRead in scripts see the real contents of scrambled files
    class CMemoryFile final : public CBinaryFileInterface
    {
    public:
        explicit CMemoryFile(SString&& data) : m_data(std::move(data)) {}
        bool FOpen(const char*, const char*, bool) override { return true; }
        void FClose() override {}
        bool FEof() override { return m_pos >= static_cast<int>(m_data.size()); }
        void FFlush() override {}
        int  FTell() override { return m_pos; }
        void FSeek(int iOffset, int iOrigin) override
        {
            const int iBase = iOrigin == SEEK_CUR ? m_pos : iOrigin == SEEK_END ? static_cast<int>(m_data.size()) : 0;
            m_pos = std::max(0, std::min(static_cast<int>(m_data.size()), iBase + iOffset));
        }
        int FRead(void* pData, uint uiSize) override
        {
            const int iCount = std::max(0, std::min(static_cast<int>(uiSize), static_cast<int>(m_data.size()) - m_pos));
            if (iCount)
                memcpy(pData, m_data.data() + m_pos, iCount);
            m_pos += iCount;
            return iCount;
        }
        int FWrite(const void*, uint) override { return 0; }

    private:
        SString m_data;
        int     m_pos = 0;
    };
}  // namespace

bool MajorRpCacheIsScrambleType(const SString& strPath)
{
    const SString strExt = ExtractExtension(strPath).ToLower();
    // scripts and models
    if (strExt == "lua" || strExt == "luac" || strExt == "txd" || strExt == "dff" || strExt == "col" || strExt == "ifp")
        return true;
    // images (dxCreateTexture/dxDrawImage 2D, GUI images, browser pages) and sounds (playSound/playSound3D)
    return strExt == "png" || strExt == "jpg" || strExt == "jpeg" || strExt == "bmp" || strExt == "mp3" || strExt == "ogg" || strExt == "wav";
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

CBinaryFileInterface* MajorRpCacheOpenForScript(const SString& strPath, bool bWrite)
{
    if (!MajorRpCacheIsScrambledFile(strPath))
        return nullptr;
    SString strData;
    if (!MajorRpCacheLoad(strPath, strData))
        return nullptr;
    if (bWrite)
    {
        // A script wants to change the file: store it plain again and let the normal file code open it
        FileSave(strPath, strData);
        return nullptr;
    }
    return new CMemoryFile(std::move(strData));
}
