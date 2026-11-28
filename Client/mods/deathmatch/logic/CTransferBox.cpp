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

// MAJOR RP: download box v5 (Danex, 2026-11-28): bottom of the screen, only the header, the ring with the
// percentage and "125 / 139 MB" under it. No resource names, speed, time left or cancel button.
void CTransferBox::CreateMajorRPWindow()
{
    const float R = 170;  // ring size
    const float W = 400, H = 78 + R;
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
                     unsigned char r, unsigned char g, unsigned char b)
    {
        lbl.reset(m_GUI->CreateLabel(parent, text));
        lbl->SetFont(font);
        lbl->SetPosition(CVector2D(x, y));
        lbl->SetSize(CVector2D(w, h));
        lbl->SetTextColor(r, g, b);
        lbl->SetVerticalAlign(CGUI_ALIGN_VERTICALCENTER);
        lbl->SetHorizontalAlign(CGUI_ALIGN_HORIZONTALCENTER);
    };

    // Invisible container (fully transparent image) at the bottom centre of the screen
    m_card.reset(m_GUI->CreateStaticImage());
    m_card->SetFrameEnabled(false);
    m_card->LoadFromFile("cgui\\images\\majorrp\\tb3_none.png");
    m_card->SetPosition(CVector2D(screenSize.fX * 0.5f - W * 0.5f, screenSize.fY - H - 30));
    m_card->SetSize(CVector2D(W, H));
    m_card->SetAlwaysOnTop(true);
    m_card->SetProperty("MousePassThroughEnabled", "True");
    m_card->SetVisible(false);

    // "DOWNLOADING" + "MAJOR RP server files" (image, with letter spacing and a soft shadow)
    image(m_header, m_card.get(), "cgui\\images\\majorrp\\tb3_header.png", 0, 0, W, 74);

    // Ring with the percentage drawn in: ring/p000.png ... p100.png, swapped as the download moves
    image(m_ring, m_card.get(), "cgui\\images\\majorrp\\ring\\p000.png", (W - R) / 2, 78, R, R);
    m_ringFrame = 0;
    // "125 / 139 MB" under the percentage, inside the ring (m_resourceName used to show the resource name)
    label(m_resourceName, m_card.get(), "", "majorrp-stat", (W - 150) / 2, 78 + R / 2 + 12, 150, 18, 255, 140, 26);
    // ESC opens the menu, where Disconnect cancels the download
}

void CTransferBox::SetCurrentName(const char* szName)
{
    // MAJOR RP: resource names are not shown (the line under the percentage shows the downloaded MB)
    (void)szName;
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
        const int    iFrame = std::min(100, static_cast<int>(fraction * 100));
        if (iFrame != m_ringFrame)
        {
            m_ringFrame = iFrame;
            m_ring->LoadFromFile(SString("cgui\\images\\majorrp\\ring\\p%03d.png", iFrame));
        }

        // "125 / 139 MB" (KB for small downloads)
        if (m_downloadTotalSize >= 1024 * 1024)
            m_resourceName->SetText(
                SString("%d / %d MB", static_cast<int>(downloadedSizeTotal >> 20), static_cast<int>((m_downloadTotalSize + (1 << 20) - 1) >> 20)).c_str());
        else
            m_resourceName->SetText(
                SString("%d / %d KB", static_cast<int>(downloadedSizeTotal >> 10), static_cast<int>((m_downloadTotalSize + 1023) >> 10)).c_str());
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
        return;  // stock MTA camera and radar behind the download box (Danex, 2026-11-28)

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
        if (visible)
            m_card->BringToFront();
        return;
    }
    m_window->SetVisible(visible);
}
