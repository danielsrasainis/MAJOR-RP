/*****************************************************************************
 *
 *  PROJECT:     MAJOR RP (based on Multi Theft Auto: San Andreas)
 *  FILE:        mods/deathmatch/logic/CMajorRpCache.h
 *  PURPOSE:     Scrambled resource cache (see CMajorRpCache.cpp)
 *
 *****************************************************************************/
#pragma once

// File types stored scrambled in the resource cache (client scripts and models)
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
