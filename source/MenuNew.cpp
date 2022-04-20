#include "MenuNew.h"
#include "CPad.h"
#include "CSprite2d.h"
#include "CTxdStore.h"
#include "CStreaming.h"
#include "CWorld.h"
#include "Utility.h"

using namespace plugin;

void CMenuNew::DrawMap() {
	CSprite2d::DrawRect(CRect(-5.0f, -5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT + 5.0f), CRGBA(0, 0, 0, 255));

	const float mapHalfSize = GetMenuMapWholeSize() / 2;
	CRect rect;
	CRGBA col = { 255, 255, 255, static_cast<unsigned char>(FadeIn(255))};

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

	DrawBlips();

	DrawCrosshair(m_vCrosshair.x, m_vCrosshair.y);
}

void CMenuNew::DrawCrosshair(float x, float y) {
	float lineSize = Scale(2.0f);
	CRGBA lineCol = CRGBA(234, 171, 54, FadeIn(255));

	// Ver
	CSprite2d::DrawRect(CRect(((x) - lineSize), 0.0f, ((x) + lineSize), SCREEN_HEIGHT), lineCol);

	// Hor
	CSprite2d::DrawRect(CRect(0.0f, ((y) - lineSize), SCREEN_WIDTH, ((y) + lineSize)), lineCol);
}

void CMenuNew::DrawBlips() {
	// Draw blips
	CPed* playa = FindPlayerPed();
	CBlip* trace = CRadar::ms_RadarTrace;
	for (int i = 0; i < 32; i++) {
		if (!trace[i].m_bInUse)
			continue;

		CVector2D pos = WorldToMap(trace[i].m_vecPos);
		CRGBA col = CRadar::GetRadarTraceColour(trace[i].m_nColour, true);
		unsigned short id = trace[i].m_nRadarSprite;

		if (id != RADAR_SPRITE_NONE) {
			switch (id) {
			default:
				CSprite2d* sprite = pRadarSprites[id];
				if (sprite)
					DrawSpriteWithRotation(sprite, pos.x, pos.y, Scale(24.0f), Scale(24.0f), 0.0f, CRGBA(255, 255, 255, FadeIn(255)));
				break;
			}
		}
		else {
			static int level = 0;
			static int time = 0;
			if (time < CTimer::m_snTimeInMillisecondsPauseMode) {
				level++;
				time = 800 + CTimer::m_snTimeInMillisecondsPauseMode;
			}

			if (level > 2)
				level = 0;

			switch (level) {
			case 1:
				DrawTriangle(pos.x, pos.y, Scale(16.0f), DegToRad(0.0f), CRGBA(0, 0, 0, FadeIn(255)));
				DrawTriangle(pos.x, pos.y, Scale(14.0f), DegToRad(0.0f), CRGBA(col.r, col.g, col.b, FadeIn(255)));
;				break;
			case 2:
				DrawTriangle(pos.x, pos.y, Scale(16.0f), DegToRad(180.0f), CRGBA(0, 0, 0, FadeIn(255)));
				DrawTriangle(pos.x, pos.y, Scale(14.0f), DegToRad(180.0f), CRGBA(col.r, col.g, col.b, FadeIn(255)));
				break;
			default:
				DrawSpriteWithRotation(NULL, pos.x, pos.y, Scale(16.0f), Scale(16.0f), 0.0f, CRGBA(0, 0, 0, FadeIn(255)));
				DrawSpriteWithRotation(NULL, pos.x, pos.y, Scale(14.0f), Scale(14.0f), 0.0f, CRGBA(col.r, col.g, col.b, FadeIn(255)));
				break;
			}
		}
	}

	if (playa) {
		// Draw player
		CVector2D pos = WorldToMap(playa->GetPosition());
		float angle = FindPlayerHeading();
		CSprite2d* sprite = pRadarSprites[RADAR_SPRITE_CENTRE];

		if (sprite && flashItem(1000, 200))
			DrawSpriteWithRotation(sprite, pos.x, pos.y, Scale(24.0f), Scale(24.0f), angle, CRGBA(255, 255, 255, FadeIn(255)));
	}
}

CVector2D CMenuNew::WorldToMap(CVector in) {
	CVector2D out;

	float x = (in.x + (MAP_SIZE / 2)) / MAP_SIZE;
	float y = ((MAP_SIZE / 2) - in.y) / MAP_SIZE;
	x *= GetMenuMapWholeSize();
	y *= GetMenuMapWholeSize();

	x += m_vMapBase.x - GetMenuMapWholeSize() / 2;
	y += m_vMapBase.y - GetMenuMapWholeSize() / 2;

	out.x = x;
	out.y = y;
	return out;
}

CVector CMenuNew::MapToWorld(CVector2D in) {
	CVector out;

	in.x = (in.x + GetMenuMapWholeSize() / 2) - m_vMapBase.x;
	in.y = m_vMapBase.y - (in.y + GetMenuMapWholeSize() / 2);

	in.x /= GetMenuMapWholeSize();
	in.y /= GetMenuMapWholeSize();

	float w = (MAP_SIZE / 2) / MAP_SIZE;
	out.x = (in.x - w) * MAP_SIZE;
	out.y = (w + in.y) * MAP_SIZE;
	out.z = CWorld::FindGroundZForCoord(out.x, out.y);
	return out;
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

	if (clearInput) {
		pad->Clear(true);
		clearInput = false;
	}

	const float halfMapSize = GetMenuMapWholeSize() / 2;
	bool leftBound = (m_vMapBase.x - halfMapSize < 0.0f);
	bool rightBound = (m_vMapBase.x + halfMapSize > SCREEN_WIDTH);
	bool topBound = (m_vMapBase.y - halfMapSize < 0.0f);
	bool bottomBound = (m_vMapBase.y + halfMapSize > SCREEN_HEIGHT);
	bool leftMousebutton = pad->NewMouseControllerState.LMB;
	bool rightMousebutton = pad->NewMouseControllerState.RMB && !pad->OldMouseControllerState.RMB;
	bool mapZoomOut = pad->NewMouseControllerState.WHEELDN && !pad->OldMouseControllerState.WHEELDN;
	bool mapZoomIn = pad->NewMouseControllerState.WHEELUP && !pad->OldMouseControllerState.WHEELUP;

	mapZoomOut |= pad->NewKeyState.pgdn;
	mapZoomIn |= pad->NewKeyState.pgup;

	mapZoomOut |= pad->NewState.LeftShoulder2;
	mapZoomIn |= pad->NewState.RightShoulder2;

	rightMousebutton |= (pad->NewKeyState.enter && !pad->OldKeyState.enter) || (pad->NewKeyState.extenter && !pad->OldKeyState.extenter);
	rightMousebutton |= (pad->NewState.ButtonCross && !pad->OldState.ButtonCross);
		
	CVector2D previousMapBase = m_vMapBase;
	if (m_bShowMouse) {
		m_vCrosshair.x = static_cast<float>(m_nMouseTempPosX);
		m_vCrosshair.y = static_cast<float>(m_nMouseTempPosY);

		if (leftMousebutton && (leftBound || rightBound || topBound || bottomBound)) {
			m_vMapBase.x += static_cast<float>(m_nMouseTempPosX - m_nMouseOldPosX);
			m_vMapBase.y += static_cast<float>(m_nMouseTempPosY - m_nMouseOldPosY);
		}
	}
	else {
		const int mult = 4.0f;
		float right = pad->NewKeyState.left ? -1.0f : pad->NewKeyState.right ? 1.0f : 0.0f;
		float up = pad->NewKeyState.up ? -1.0f : pad->NewKeyState.down ? 1.0f : 0.0f;

		right += (pad->NewState.DPadLeft || pad->NewState.LeftStickX < 0.0f) ? -1.0f : (pad->NewState.DPadRight || pad->NewState.LeftStickX > 0.0f) ? 1.0f : 0.0f;
		up += (pad->NewState.DPadUp || pad->NewState.LeftStickY < 0.0f) ? -1.0f : (pad->NewState.DPadDown || pad->NewState.LeftStickY > 0.0f) ? 1.0f : 0.0f;

		right = clamp(right, -1.0f, 1.0f);
		up = clamp(up, -1.0f, 1.0f);

		if (isNearlyEqualF(m_vCrosshair.x, SCREEN_WIDTH / 2, 64.0f)) {
			if ((right < 0.0f && leftBound || right > 0.0f && rightBound)) {
				m_vCrosshair.x = SCREEN_WIDTH / 2;
				m_vMapBase.x -= right * mult;
			}
		}

		if (isNearlyEqualF(m_vCrosshair.y, SCREEN_HEIGHT / 2, 64.0f)) {
			if ((up < 0.0f && topBound || up > 0.0f && bottomBound)) {
				m_vCrosshair.y = SCREEN_HEIGHT / 2;
				m_vMapBase.y -= up * mult;
			}
		}
		m_vCrosshair.x += right * mult;
		m_vCrosshair.y += up * mult;
	}

	if (mapZoomIn)
		DoMapZoomInOut(false);
	else if (mapZoomOut)
		DoMapZoomInOut(true);

	if (rightMousebutton)
		SetWaypoint(m_vCrosshair.x, m_vCrosshair.y);

	m_vCrosshair.x = clamp(m_vCrosshair.x, m_vMapBase.x - halfMapSize, m_vMapBase.x + halfMapSize);
	m_vCrosshair.y = clamp(m_vCrosshair.y, m_vMapBase.y - halfMapSize, m_vMapBase.y + halfMapSize);

	m_vMapBase.x = clamp(m_vMapBase.x, (SCREEN_WIDTH / 2) - halfMapSize, (SCREEN_WIDTH / 2) + halfMapSize);
	m_vMapBase.y = clamp(m_vMapBase.y, (SCREEN_HEIGHT / 2) - halfMapSize, halfMapSize);

	if (m_fMapZoom <= MAP_ZOOM_MIN) {
		ResetMap();
	}
}

void CMenuNew::DoMapZoomInOut(bool out) {
	const float value = (!out ? 1.1f : 1.0f / 1.1f);

	if (out && m_fMapZoom > MAP_ZOOM_MIN || !out && m_fMapZoom < MAP_ZOOM_MAX) {
		m_fMapZoom *= value;

		const float halfMapSize = GetMenuMapWholeSize() / 2;
		m_vMapBase.x += ((m_vCrosshair.x) - m_vMapBase.x) * (1.0f - value);
		m_vMapBase.y += ((m_vCrosshair.y) - m_vMapBase.y) * (1.0f - value);
	}
}

void CMenuNew::ResetMap() {
	m_fMapZoom = MAP_ZOOM_MIN;
	m_vMapBase = { SCREEN_WIDTH / 2, GetMenuMapWholeSize() / 2 };
	m_vCrosshair.x = SCREEN_WIDTH / 2;
	m_vCrosshair.y = SCREEN_HEIGHT / 2;
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

void CMenuNew::SetWaypoint(float x, float y) {
	if (targetBlipIndex) {
		CRadar::ClearBlip(targetBlipIndex);
		targetBlipIndex = 0;
	}
	else {

		CVector pos = MapToWorld(CVector2D(x, y));
		int i = CRadar::SetCoordBlip(BLIP_COORD, pos, BLIP_COLOUR_RED, BLIP_DISPLAY_BOTH);
		CRadar::SetBlipSprite(i, RADAR_SPRITE_NONE);
		targetBlipIndex = i;
	}
}

float CMenuNew::GetMenuMapTileSize() {
	const float tileSize = Scale(48.0f);
	return tileSize;
}

float CMenuNew::GetMenuMapWholeSize() {
	const float mapWholeSize = GetMenuMapTileSize() * RADAR_NUM_TILES;
	return mapWholeSize * m_fMapZoom;
}
