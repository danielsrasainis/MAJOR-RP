/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        mods/deathmatch/logic/CTransferBox.h
 *  PURPOSE:     Header for transfer box class
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include <memory>
#include <bitset>

enum class TransferBoxType : uint8_t
{
    RESOURCE_DOWNLOAD,
    MAP_DOWNLOAD,
};

class CTransferBox
{
public:
    explicit CTransferBox(TransferBoxType transferType);

    void Show();
    void Hide();

    void DoPulse();

    bool IsVisible() const { return m_visible[TB_VISIBILITY_MTA]; }

    void SetDownloadProgress(uint64_t downloadedSizeTotal);
    void SetCurrentName(const char* szName);  // MAJOR RP: resource being downloaded

    void     AddToDownloadTotalSize(uint64 bytes) { m_downloadTotalSize += bytes; }
    uint64_t GetDownloadTotalSize() const { return m_downloadTotalSize; }

    bool SetClientVisibility(bool visible);
    bool SetServerVisibility(bool visible);

    bool SetAlwaysVisible(bool visible);
    bool IsAlwaysVisible() const { return m_alwaysVisible; }

private:
    void CreateTransferWindow();
    void CreateMajorRPWindow();  // MAJOR RP: modern download box

    void UpdateWindowVisibility() const;

    std::string m_titleProgressPrefix;
    uint64_t    m_downloadTotalSize = 0;

    CGUI*                                            m_GUI;
    std::unique_ptr<CGUIWindow>                      m_window;
    std::unique_ptr<CGUIProgressBar>                 m_progressBar;
    std::unique_ptr<CGUILabel>                       m_infoLabel;
    std::array<std::unique_ptr<CGUIStaticImage>, 10> m_iconImages;
    size_t                                           m_iconIndex;
    CElapsedTime                                     m_iconTimer;

    // MAJOR RP: modern download box
    bool                                            m_majorRP = false;
    std::unique_ptr<CGUIStaticImage>                m_card;
    std::unique_ptr<CGUIStaticImage>                m_header;
    std::unique_ptr<CGUIStaticImage>                m_ring;
    std::unique_ptr<CGUIStaticImage>                m_cancel;
    std::unique_ptr<CGUILabel>                      m_resourceName;
    std::string                                     m_currentName;
    int                                             m_ringFrame = -1;
    std::array<std::unique_ptr<CGUIStaticImage>, 3> m_statBox;
    std::array<std::unique_ptr<CGUILabel>, 3>       m_statKey;
    std::array<std::unique_ptr<CGUILabel>, 3>       m_statValue;
    uint64_t                                        m_lastBytes = 0;
    long long                                       m_lastTick = 0;
    double                                          m_speed = 0;  // bytes per second (smoothed)

    enum VisibilitySource
    {
        TB_VISIBILITY_MTA,
        TB_VISIBILITY_CLIENT_SCRIPT,
        TB_VISIBILITY_SERVER_SCRIPT,
        TB_VISIBILITY_SOURCES_SIZE,
    };

    std::bitset<TB_VISIBILITY_SOURCES_SIZE> m_visible;
    bool                                    m_alwaysVisible = true;
};
