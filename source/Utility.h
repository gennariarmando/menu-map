#pragma once
#include "CTimer.h"
#include "extensions/Screen.h"
#include "CSprite2d.h"

#define flashItem(on, off) (CTimer::m_snTimeInMillisecondsPauseMode % on + off < on)
#define clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))
#define isNearlyEqualF(a, b, t) (fabs(a - b) <= t)

static float DegToRad(float x) {
	return (x * M_PI / 180.0f);
}

static float RadToDeg(float x) {
	return (x * 180.0f / M_PI);
}

static float GetAspectRatio() {
	float& fScreenAspectRatio = *(float*)0x5F53C0;
	return fScreenAspectRatio;
}

static float Scale(float a) {
	return static_cast<int>(a * GetAspectRatio());
}

static void Draw2DPolygon(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, const CRGBA& color) {
	CSprite2d::SetVertices(x1, y1, x2, y2, x3, y3, x4, y4, color, color, color, color);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, 0);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)(color.a != 255));
	RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
}

static CVector2D RotateUV(CVector2D& uv, float rotation, CVector2D center) {
	CVector2D v = {
		cos(rotation) * (uv.x - center.x) + sin(rotation) * (uv.y - center.y) + center.x,
		cos(rotation) * (uv.y - center.y) - sin(rotation) * (uv.x - center.x) + center.y
	};

	uv = v;
	return v;
}

static void RotateVertices(CVector2D* rect, float x, float y, float angle) {
	float xold, yold;
	//angle /= 57.2957795;
	float _cos = cosf(angle);
	float _sin = sinf(angle);
	for (unsigned int i = 0; i < 4; i++) {
		xold = rect[i].x;
		yold = rect[i].y;
		rect[i].x = x + (xold - x) * _cos + (yold - y) * _sin;
		rect[i].y = y - (xold - x) * _sin + (yold - y) * _cos;
	}
}

static void DrawSpriteWithRotation(CSprite2d* sprite, float x, float y, float w, float h, float angle, CRGBA const& col) {
	CVector2D posn[4];
	posn[1].x = x - (w * 0.5f); posn[1].y = y - (h * 0.5f); posn[0].x = x + (w * 0.5f); posn[0].y = y - (h * 0.5f);
	posn[3].x = x - (w * 0.5f); posn[3].y = y + (h * 0.5f);	posn[2].x = x + (w * 0.5f); posn[2].y = y + (h * 0.5f);
	
	RotateVertices(posn, x, y, angle);

	if (sprite)
		sprite->Draw(
			posn[3].x, posn[3].y, posn[2].x, posn[2].y,
			posn[1].x, posn[1].y, posn[0].x, posn[0].y, CRGBA(col));
	else
		CSprite2d::DrawRect(CRect(x - (w * 0.5f), y - (h * 0.5f), x + (w * 0.5f), y + (h * 0.5f)), col);
}

static void DrawTriangle(float x, float y, float scale, float angle, CRGBA const& col) {
	CVector2D posn[4];
	float w = scale;
	float h = scale;

	posn[1].x = x - (w * 0.5f); posn[1].y = y - (h * 0.5f); posn[0].x = x + (w * 0.5f); posn[0].y = y - (h * 0.5f);
	posn[3].x = x; posn[3].y = y + (h * 0.5f);	posn[2].x = x; posn[2].y = y + (h * 0.5f);

	RotateVertices(posn, x, y, angle);
	Draw2DPolygon(posn[0].x, posn[0].y, posn[1].x, posn[1].y, posn[2].x, posn[2].y, posn[3].x, posn[3].y, CRGBA(col));
}

static void DrawWayPoint(float x, float y, float scale, CRGBA const& col) {
	float line = Scale(2.0f);

	x -= (scale) / 2;
	y -= (scale) / 2;
	CSprite2d::DrawRect(CRect(x, y, x + (scale), y + line), col);
	CSprite2d::DrawRect(CRect(x + (scale) , y, x + (scale) - line, y + (scale)), col);
	CSprite2d::DrawRect(CRect(x, y + (scale), x + (scale), y + (scale) - line), col);
	CSprite2d::DrawRect(CRect(x, y, x + line, y + (scale)), col);
}
