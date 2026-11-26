/*****************************************************************************
 *
 *  PROJECT:     MAJOR RP (based on Multi Theft Auto: San Andreas)
 *  FILE:        mods/deathmatch/logic/CMajorRpCache.h
 *  PURPOSE:     Scrambled resource cache (see CMajorRpCache.cpp)
 *
 *****************************************************************************/
#pragma once

#include "majorrp_cache_core.h"

class CBinaryFileInterface;

// File types stored scrambled in the resource cache (client scripts, models, images, sounds)
bool MajorRpCacheIsScrambleType(const SString& strPath);

// Read a cache file into memory, unscrambling it if needed. Works for plain files too.
bool MajorRpCacheLoad(const SString& strPath, SString& outData);
bool MajorRpCacheLoad(const SString& strPath, std::vector<char>& outData);

// Is this file stored scrambled?
bool MajorRpCacheIsScrambledFile(const SString& strPath);

// Checksum of the real (unscrambled) contents
CChecksum MajorRpCacheChecksum(const SString& strPath);

// Scramble a downloaded file in place if it is a scramble type and still plain
void MajorRpCacheScrambleFile(const SString& strPath);

// fileOpen on a scrambled file: read mode gets an in-memory file with the real contents;
// write mode turns the file back into a plain file first (returns nullptr: open it normally)
CBinaryFileInterface* MajorRpCacheOpenForScript(const SString& strPath, bool bWrite);
