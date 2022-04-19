#include "MenuNew.h"
#include "CPad.h"
#include "CRadar.h"
#include "CSprite2d.h"
#include "CTxdStore.h"
#include "CStreaming.h"

#define clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))

using namespace plugin;

void CMenuNew::DrawMap() {
	CSprite2d::DrawRect(CRect(-5.0f, -5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));

	const float mapHalfSize = GetMenuMapWholeSize() / 2;
	CRect rect;
	CRGBA col = { 255, 255, 255, 255 };

	float mapZoom = GetMenuMapTileSize() * m_fMapZoom;
	rect.left = m_vMapBase.x;
	rect.top = m_vMapBase.y;
	rect.right = rect.left + mapZoom;
	rect.bottom = rect.top + mapZoom;

	StreamRadarSections();

	for (int y = 0; y < RADAR_NUM_TILES; y++) {
		for (int x = 0; x < RADAR_NUM_TILES; x++) {
			DrawRadarSectionMap(x, y, CRect((rect.left - mapHalfSize), (rect.top - mapHalfSize), (rect.right - mapHalfSize), (rect.bottom - mapHalfSize)), col);

			rect.left += mapZoom;
			rect.right = rect.left + mapZoom;
		}
		rect.left = m_vMapBase.x;
		rect.right = rect.left + mapZoom;

		rect.top += mapZoom;
		rect.bottom = rect.top + mapZoom;
	}

	DrawCrosshair(m_vCrosshair.x, m_vCrosshair.y);
}

void CMenuNew::DrawCrosshair(float x, float y) {
	float lineSize = SCREEN_COORD(2.0f);
	CRGBA lineCol = CRGBA(234, 171, 54, 255);

	// Ver
	CSprite2d::DrawRect(CRect(((x) - lineSize), 0.0f, ((x) + lineSize), SCREEN_HEIGHT), lineCol);

	// Hor
	CSprite2d::DrawRect(CRect(0.0f, ((y) - lineSize), SCREEN_WIDTH, ((y) + lineSize)), lineCol);
}

void CMenuNew::StreamRadarSections() {
	if (CStreaming::ms_disableStreaming)
		return;

	for (int i = 0; i < RADAR_NUM_TILES; ++i) {
		for (int j = 0; j < RADAR_NUM_TILES; ++j) {
			int index = i + RADAR_NUM_TILES * j;
			int r = gRadarTxdIds[index];

			CStreaming::RequestModel(r + 5500, GAME_REQUIRED | KEEP_IN_MEMORY);
			CStreaming::LoadRequestedModels();
		};
	}
}

void CMenuNew::MapInput() {
	CPad* pad = CPad::GetPad(0);

	m_vCrosshair.x = SCREEN_WIDTH / 2;
	m_vCrosshair.y = SCREEN_HEIGHT / 2;

	if (m_bShowMouse) {
		m_vCrosshair.x = static_cast<float>(m_nMouseTempPosX);
		m_vCrosshair.y = static_cast<float>(m_nMouseTempPosY);

		bool leftMousebutton = pad->NewMouseControllerState.LMB;
		bool rightMousebutton = pad->NewMouseControllerState.LMB;
		bool mapZoomOut = pad->NewMouseControllerState.WHEELDN && !pad->OldMouseControllerState.WHEELDN;
		bool mapZoomIn = pad->NewMouseControllerState.WHEELUP && !pad->OldMouseControllerState.WHEELUP;

		if (leftMousebutton) {
			m_vMapBase.x += static_cast<float>(m_nMouseTempPosX - m_nMouseOldPosX);
			m_vMapBase.y += static_cast<float>(m_nMouseTempPosY - m_nMouseOldPosY);
		}
		else if (mapZoomIn)
			DoMapZoomInOut(false);
		else if (mapZoomOut)
			DoMapZoomInOut(true);
	}

	const float halfMapSize = GetMenuMapWholeSize() / 2;
	if (m_vMapBase.x + halfMapSize < (SCREEN_WIDTH / 2))
		m_vMapBase.x = (SCREEN_WIDTH / 2) - halfMapSize;

	if (m_vMapBase.x - halfMapSize > (SCREEN_WIDTH / 2))
		m_vMapBase.x = (SCREEN_WIDTH / 2) + halfMapSize;

	if (m_vMapBase.y + halfMapSize < (SCREEN_HEIGHT / 2))
		m_vMapBase.y = (SCREEN_HEIGHT / 2) - halfMapSize;

	if (m_vMapBase.y - halfMapSize > 0.0f)
		m_vMapBase.y = halfMapSize;
}

void CMenuNew::DoMapZoomInOut(bool out) {
	const float value = (!out ? 1.1f : 1.0f / 1.1f);

	if (out && m_fMapZoom > 2.1f || !out && m_fMapZoom < 8.1f) {
		m_fMapZoom *= value;

		const float halfMapSize = GetMenuMapWholeSize() / 2;
		m_vMapBase.x += ((m_vCrosshair.x) - m_vMapBase.x) * (1.0f - value);
		m_vMapBase.y += ((m_vCrosshair.y) - m_vMapBase.y) * (1.0f - value);
	}
}

void CMenuNew::ResetMap() {
	m_fMapZoom = 2.1f;
	m_vMapBase = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	m_vCrosshair = m_vMapBase;
}

void CMenuNew::DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col) {
	int index = x + RADAR_NUM_TILES * y;
	RwTexture* texture = NULL;

	index = clamp(index, 0, (RADAR_NUM_TILES * RADAR_NUM_TILES) - 1);

	int r = gRadarTxdIds[index];

	RwTexDictionary* txd = CTxdStore::ms_pTxdPool->GetAt(r)->m_pRwDictionary;
	if (txd)
		texture = GetFirstTexture(txd);

	bool inBounds = (x >= 0 && x <= RADAR_NUM_TILES - 1) && (y >= 0 && y <= RADAR_NUM_TILES - 1);

	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERMIPLINEAR);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);

	if (inBounds) {
		if (texture) {
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(texture));
			
			CSprite2d::SetVertices(rect, col, col, col, col, 0);
			RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
		}
	}
}

float CMenuNew::GetMenuMapTileSize() {
	const float tileSize = SCREEN_COORD(48.0f);
	return tileSize;
}

float CMenuNew::GetMenuMapWholeSize() {
	const float mapWholeSize = GetMenuMapTileSize() * RADAR_NUM_TILES;
	return mapWholeSize * m_fMapZoom;
}
