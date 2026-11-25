/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        mods/deathmatch/logic/CTransferBox.cpp
 *  PURPOSE:     Transfer box GUI
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include <StdInc.h>
#include "majorrp_config.h"

#define TRANSFERBOX_HEIGHT         58
#define TRANSFERBOX_ICONSIZE       20
#define TRANSFERBOX_PROGRESSHEIGHT 28
#define TRANSFERBOX_YSTART         20
#define TRANSFERBOX_SPACER         11

CTransferBox::CTransferBox(TransferBoxType transferType) : m_GUI(g_pCore->GetGUI())
{
    switch (transferType)
    {
        case TransferBoxType::MAP_DOWNLOAD:
            m_titleProgressPrefix = _("Map download progress:");
            break;
        default:
            m_titleProgressPrefix = _("Download Progress:");
    }

    m_visible.set(TB_VISIBILITY_CLIENT_SCRIPT);
    m_visible.set(TB_VISIBILITY_SERVER_SCRIPT);

    g_pCore->GetCVars()->Get("always_show_transferbox", m_alwaysVisible);

#if MAJORRP_SINGLE_SKIN
    m_majorRP = true;
    CreateMajorRPWindow();
#else
    CreateTransferWindow();
#endif
}

// MAJOR RP: modern download box (card, logo, percentage, slim bar, speed and time left)
void CTransferBox::CreateMajorRPWindow()
{
    const float W = 560, H = 206, PAD = 20;
    CVector2D   screenSize = m_GUI->GetResolution();

    auto image = [&](std::unique_ptr<CGUIStaticImage>& img, CGUIElement* parent, const char* file, float x, float y, float w, float h) {
        img.reset(m_GUI->CreateStaticImage(parent));
        img->SetFrameEnabled(false);
        img->LoadFromFile(file);
        img->SetPosition(CVector2D(x, y));
        img->SetSize(CVector2D(w, h));
        img->SetProperty("MousePassThroughEnabled", "True");
    };
    auto label = [&](std::unique_ptr<CGUILabel>& lbl, CGUIElement* parent, const char* text, const char* font, float x, float y, float w, float h,
                     unsigned char r, unsigned char g, unsigned char b) {
        lbl.reset(m_GUI->CreateLabel(parent, text));
        lbl->SetFont(font);
        lbl->SetPosition(CVector2D(x, y));
        lbl->SetSize(CVector2D(w, h));
        lbl->SetTextColor(r, g, b);
        lbl->SetVerticalAlign(CGUI_ALIGN_VERTICALCENTER);
    };

    // Top-level card image (no window frame, so the rounded corners stay clean)
    m_card.reset(m_GUI->CreateStaticImage());
    m_card->SetFrameEnabled(false);
    m_card->LoadFromFile("cgui\\images\\majorrp\\tb_card.png");
    m_card->SetPosition(CVector2D(screenSize.fX * 0.5f - W * 0.5f, screenSize.fY * 0.82f - H * 0.5f));
    m_card->SetSize(CVector2D(W, H));
    m_card->SetAlwaysOnTop(true);
    m_card->SetProperty("MousePassThroughEnabled", "True");
    m_card->SetVisible(false);
    image(m_logo, m_card.get(), "cgui\\images\\majorrp\\tb_logo.png", PAD, 16, 94, 60);

    label(m_title, m_card.get(), "CONNECTING TO MAJOR RP", "majorrp-title-big", 130, 18, 300, 26, 240, 234, 228);
    label(m_subtitle, m_card.get(), "Downloading server files", "default-normal", 130, 46, 300, 22, 160, 150, 142);
    label(m_percent, m_card.get(), "0%", "majorrp-big", W - PAD - 130, 20, 130, 44, 240, 234, 228);
    m_percent->SetHorizontalAlign(CGUI_ALIGN_RIGHT);

    m_trackWidth = W - PAD * 2;
    image(m_track, m_card.get(), "cgui\\images\\majorrp\\tb_track.png", PAD, 90, m_trackWidth, 8);
    image(m_fill, m_card.get(), "cgui\\images\\majorrp\\tb_fill.png", PAD, 90, 1, 8);

    const char* keys[3] = {"DOWNLOADED", "SPEED", "TIME LEFT"};
    const float boxW = (m_trackWidth - 20) / 3;
    for (int i = 0; i < 3; ++i)
    {
        const float x = PAD + i * (boxW + 10);
        image(m_statBox[i], m_card.get(), "cgui\\images\\majorrp\\tb_stat.png", x, 112, boxW, 46);
        label(m_statKey[i], m_card.get(), keys[i], "default-bold-small", x + 12, 116, boxW - 16, 16, 255, 140, 26);
        label(m_statValue[i], m_card.get(), "-", "majorrp-stat", x + 12, 132, boxW - 16, 22, 240, 234, 228);
    }

    label(m_footer, m_card.get(), "To cancel, press ESC and choose Disconnect.", "default-normal", PAD, 170, m_trackWidth, 24, 160, 150, 142);
}

void CTransferBox::CreateTransferWindow()
{
    // Find our largest piece of text, so we can size accordingly
    std::string largeTextSample = m_titleProgressPrefix + " " + SString(_("%s of %s"), "999.99 kB", "999.99 kB");
    float       fTransferBoxWidth = m_GUI->GetTextExtent(largeTextSample.c_str(), "default-bold-small");
    fTransferBoxWidth = std::max<float>(fTransferBoxWidth, m_GUI->GetTextExtent(_("Disconnect to cancel download"), "default-normal"));

    // Add some padding to our text for the size of the window
    fTransferBoxWidth += 80;

    CVector2D screenSize = m_GUI->GetResolution();

    m_window.reset(m_GUI->CreateWnd());
    m_window->SetText("");
    m_window->SetAlpha(0.7f);
    m_window->SetVisible(false);
    m_window->SetAlwaysOnTop(true);
    m_window->SetCloseButtonEnabled(false);
    m_window->SetSizingEnabled(false);
    m_window->SetPosition(CVector2D(screenSize.fX * 0.5f - fTransferBoxWidth * 0.5f, screenSize.fY * 0.85f - TRANSFERBOX_HEIGHT * 0.5f));
    m_window->SetSize(CVector2D(fTransferBoxWidth, TRANSFERBOX_HEIGHT));  // relative 0.35, 0.225

    m_progressBar.reset(m_GUI->CreateProgressBar(m_window.get()));
    m_progressBar->SetPosition(CVector2D(0, TRANSFERBOX_YSTART));
    m_progressBar->SetSize(CVector2D(fTransferBoxWidth, TRANSFERBOX_HEIGHT - TRANSFERBOX_YSTART - TRANSFERBOX_SPACER));

    m_infoLabel.reset(m_GUI->CreateLabel(m_progressBar.get(), _("Disconnect to cancel download")));
    float fTempX = (m_progressBar->GetSize().fX - m_GUI->GetTextExtent(m_infoLabel->GetText().c_str()) - TRANSFERBOX_ICONSIZE - 4) * 0.5f;
    m_infoLabel->SetPosition(CVector2D(fTempX + TRANSFERBOX_ICONSIZE + 4, 0));
    m_infoLabel->SetSize(CVector2D(fTransferBoxWidth, TRANSFERBOX_PROGRESSHEIGHT));
    m_infoLabel->SetTextColor(0, 0, 0);
    m_infoLabel->SetVerticalAlign(CGUI_ALIGN_VERTICALCENTER);

    for (size_t i = 0; i < m_iconImages.size(); ++i)
    {
        SString filePath("cgui\\images\\transferset\\%u.png", i + 1);
        m_iconImages[i].reset(m_GUI->CreateStaticImage(m_progressBar.get()));
        m_iconImages[i]->SetFrameEnabled(false);
        m_iconImages[i]->SetPosition(CVector2D(fTempX, ((TRANSFERBOX_PROGRESSHEIGHT) / 2) - (TRANSFERBOX_ICONSIZE / 2)));
        m_iconImages[i]->SetSize(CVector2D(TRANSFERBOX_ICONSIZE, TRANSFERBOX_ICONSIZE));
        m_iconImages[i]->LoadFromFile(filePath);
        m_iconImages[i]->SetVisible(false);
    }

    m_iconIndex = 0;
    m_iconImages[m_iconIndex]->SetVisible(true);
}

void CTransferBox::Show()
{
    m_visible.set(TB_VISIBILITY_MTA);
    UpdateWindowVisibility();
}

void CTransferBox::Hide()
{
    m_visible.reset(TB_VISIBILITY_MTA);
    UpdateWindowVisibility();

    m_downloadTotalSize = 0;
    m_lastTick = 0;
    m_lastBytes = 0;
    m_speed = 0;
}

void CTransferBox::SetDownloadProgress(uint64_t downloadedSizeTotal)
{
    if (m_majorRP)
    {
        const double fraction = m_downloadTotalSize ? std::min(1.0, static_cast<double>(downloadedSizeTotal) / m_downloadTotalSize) : 0.0;
        m_percent->SetText(SString("%d%%", static_cast<int>(fraction * 100)).c_str());
        m_fill->SetSize(CVector2D(std::max(8.0f, static_cast<float>(m_trackWidth * fraction)), 8));

        // Smoothed download speed
        const long long now = GetTickCount64_();
        if (m_lastTick == 0 || downloadedSizeTotal < m_lastBytes)
        {
            m_lastTick = now;
            m_lastBytes = downloadedSizeTotal;
        }
        else if (now - m_lastTick >= 500)
        {
            const double instant = (downloadedSizeTotal - m_lastBytes) * 1000.0 / (now - m_lastTick);
            m_speed = (m_speed <= 0) ? instant : m_speed * 0.7 + instant * 0.3;
            m_lastTick = now;
            m_lastBytes = downloadedSizeTotal;
        }

        m_statValue[0]->SetText(SString("%s / %s", GetDataUnit(downloadedSizeTotal).c_str(), GetDataUnit(m_downloadTotalSize).c_str()).c_str());
        if (m_speed > 0)
        {
            m_statValue[1]->SetText(SString("%s/s", GetDataUnit(static_cast<uint64_t>(m_speed)).c_str()).c_str());
            const uint64_t left = (m_downloadTotalSize > downloadedSizeTotal) ? m_downloadTotalSize - downloadedSizeTotal : 0;
            const int      seconds = static_cast<int>(left / m_speed);
            m_statValue[2]->SetText((seconds >= 60 ? SString("%d min %d s", seconds / 60, seconds % 60) : SString("%d s", seconds)).c_str());
        }
        return;
    }

    SString current = GetDataUnit(downloadedSizeTotal);
    SString total = GetDataUnit(m_downloadTotalSize);
    SString progress = m_titleProgressPrefix + " " + SString(_("%s of %s"), current.c_str(), total.c_str());
    m_window->SetText(progress.c_str());
    m_progressBar->SetProgress(static_cast<float>(static_cast<double>(downloadedSizeTotal) / m_downloadTotalSize));
}

void CTransferBox::DoPulse()
{
    if (m_majorRP)
        return;

    if (m_iconTimer.Get() >= 50)
    {
        m_iconTimer.Reset();
        m_iconImages[m_iconIndex]->SetVisible(false);
        m_iconIndex = (m_iconIndex + 1) % m_iconImages.size();
        m_iconImages[m_iconIndex]->SetVisible(true);
    }
}

bool CTransferBox::SetClientVisibility(bool visible)
{
    if (m_visible[TB_VISIBILITY_CLIENT_SCRIPT] == visible)
        return false;

    m_visible.set(TB_VISIBILITY_CLIENT_SCRIPT, visible);
    UpdateWindowVisibility();
    return true;
}

bool CTransferBox::SetServerVisibility(bool visible)
{
    if (m_visible[TB_VISIBILITY_SERVER_SCRIPT] == visible)
        return false;

    m_visible.set(TB_VISIBILITY_SERVER_SCRIPT, visible);
    UpdateWindowVisibility();
    return true;
}

bool CTransferBox::SetAlwaysVisible(bool visible)
{
    if (m_alwaysVisible == visible)
        return false;

    m_alwaysVisible = visible;
    UpdateWindowVisibility();
    return true;
}

void CTransferBox::UpdateWindowVisibility() const
{
    bool visible = m_visible.all() || (m_visible[TB_VISIBILITY_MTA] && m_alwaysVisible);
    if (m_majorRP)
    {
        m_card->SetVisible(visible);
        return;
    }
    m_window->SetVisible(visible);
}
