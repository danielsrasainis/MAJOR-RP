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

// MAJOR RP: download box v4 (no card, bottom of the screen: header, ring with percentage, resource name, stats, cancel pill)
void CTransferBox::CreateMajorRPWindow()
{
    const float W = 400, H = 400;
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
    const float R = 210;
    image(m_ring, m_card.get(), "cgui\\images\\majorrp\\ring\\p000.png", (W - R) / 2, 78, R, R);
    m_ringFrame = 0;
    label(m_resourceName, m_card.get(), "", "majorrp-stat", (W - 150) / 2, 78 + R / 2 + 18, 150, 18, 255, 140, 26);

    const char* keys[3] = {"DOWNLOADED", "SPEED", "TIME LEFT"};
    const float boxW = 101, gap = 16, x0 = (W - (boxW * 3 + gap * 2)) / 2;
    for (int i = 0; i < 3; ++i)
    {
        const float x = x0 + i * (boxW + gap);
        image(m_statBox[i], m_card.get(), "cgui\\images\\majorrp\\tb3_stat.png", x, 300, boxW, 46);
        label(m_statKey[i], m_card.get(), keys[i], "default-small", x, 304, boxW, 16, 160, 160, 166);
        label(m_statValue[i], m_card.get(), "-", "majorrp-stat", x, 321, boxW, 20, 255, 255, 255);
    }

    // The mouse cursor is hidden while downloading: ESC opens the menu, where Disconnect cancels
    image(m_cancel, m_card.get(), "cgui\\images\\majorrp\\tb3_cancel.png", (W - 122) / 2, 362, 122, 38);
}

// MAJOR RP: while the server files download after joining, show the same moving camera as the
// account login panel (instead of MTA's default Vinewood camera with the radar), so download -> login feels
// like one scene. Camera start and drift copy account/c_login.lua loginScreens[1] and c_login_panel.lua slideScreen.
void CTransferBox::MajorRpBackdropPulse()
{
    const CVector2D              screen = m_GUI->GetResolution();

    if (!m_backdropActive)
    {
        m_backdropActive = true;
        m_camPos = CVector(46.309753417969f, -1698.5400390625f, 51.138675689697f);
        m_camLook = CVector(118.90101623535f, -1766.6478271484f, 41.555904388428f);
        m_slide = 0;
        m_backdropStartTick = GetTickCount64_();
        g_pGame->GetHud()->SetComponentVisible(HUD_ALL, false);
        // Put the (not yet spawned) local player near the view so the area streams in, like the login does
        if (CClientPlayer* pLocal = g_pClientGame->GetLocalPlayer())
            pLocal->SetPosition(CVector(118.9f, -1766.6f, 4.0f));
    }

    // The server's fadeCamera on join turns the zone name back on, so keep the area and vehicle name hidden
    CHud* pHud = g_pGame->GetHud();
    for (eHudComponent component : {HUD_AREA_NAME, HUD_VEHICLE_NAME})
        if (pHud->IsComponentVisible(component))
            pHud->SetComponentVisible(component, false);

    // Camera: same start point and slow sideways drift as the login panel
    m_slide += 0.0005f;
    const float fMove = sinf(m_slide) * 0.15f;
    m_camPos.fX += fMove;
    m_camLook.fX += fMove;
    CStaticFunctionDefinitions::SetCameraMatrix(m_camPos, &m_camLook, 0, 70);

    // Smooth start: the area is still streaming in for the first frames, so start from a dark screen
    // and fade the camera in (no black flicker)
    const float         fAge = static_cast<float>(GetTickCount64_() - m_backdropStartTick);
    const float         fCover = 1.0f - std::max(0.0f, std::min(1.0f, (fAge - 250.0f) / 900.0f));
    CGraphicsInterface* pGraphics = g_pCore->GetGraphics();

    // Darkening like the login panel (0x64), starting from almost black
    const unsigned long ulDark = static_cast<unsigned long>(std::max(100.0f, 245.0f * fCover));
    pGraphics->DrawRectQueued(0, 0, screen.fX, screen.fY, (ulDark << 24) | 0x0A0806, false);
}

void CTransferBox::MajorRpStopBackdrop() const
{
    if (!m_backdropActive)
        return;
    m_backdropActive = false;
    g_pGame->GetHud()->SetComponentVisible(HUD_ALL, true);  // scripts (account, hud) set their own HUD right after
    g_pGame->GetHud()->SetComponentVisible(HUD_AREA_NAME, false);  // like MTA after connecting: the next fadeCamera brings it back
}

void CTransferBox::SetCurrentName(const char* szName)
{
    if (!m_majorRP || !szName || m_currentName == szName)
        return;
    m_currentName = szName;
    m_resourceName->SetText(szName);
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

        // "84 / 129 MB"
        if (m_downloadTotalSize >= 1024 * 1024)
            m_statValue[0]->SetText(
                SString("%d / %d MB", static_cast<int>(downloadedSizeTotal >> 20), static_cast<int>((m_downloadTotalSize + (1 << 20) - 1) >> 20)).c_str());
        else
            m_statValue[0]->SetText(
                SString("%d / %d KB", static_cast<int>(downloadedSizeTotal >> 10), static_cast<int>((m_downloadTotalSize + 1023) >> 10)).c_str());
        if (m_speed > 0)
        {
            m_statValue[1]->SetText(SString("%s/s", GetDataUnit(static_cast<uint64_t>(m_speed)).c_str()).c_str());
            const uint64_t left = (m_downloadTotalSize > downloadedSizeTotal) ? m_downloadTotalSize - downloadedSizeTotal : 0;
            const int      seconds = static_cast<int>(left / m_speed);
            m_statValue[2]->SetText(SString("%d:%02d", seconds / 60, seconds % 60).c_str());  // "0:07"
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
    {
        if (m_card->IsVisible())
            MajorRpBackdropPulse();
        return;
    }

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
        if (!visible)
            MajorRpStopBackdrop();
        m_card->SetVisible(visible);
        if (visible)
            m_card->BringToFront();
        return;
    }
    m_window->SetVisible(visible);
}
