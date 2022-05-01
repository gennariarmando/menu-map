#pragma once
#include "CTimer.h"
#include "extensions/Screen.h"
#include "CSprite2d.h"

#define defaultScreenWidth 640.0f
#define defaultScreenHeight 448.0f
#define defaultAspectRatio defaultScreenWidth / defaultScreenHeight
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
#ifdef GTA3
    float& fScreenAspectRatio = *(float*)0x5F53C0;
#else
    float& fScreenAspectRatio = *(float*)0x94DD38;
#endif
    return fScreenAspectRatio;
}

static float ScaleX(float a) {
    return static_cast<float>(a * SCREEN_WIDTH / defaultScreenWidth) * defaultAspectRatio / GetAspectRatio();
}

static float ScaleY(float a) {
    return static_cast<float>(a * SCREEN_HEIGHT / defaultScreenHeight);
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

static void DrawUnfilledRect(float x, float y, float thinkness, float w, float h, CRGBA const& col) {
    float line = thinkness;

    x -= (w) / 2;
    y -= (h) / 2;
    CSprite2d::DrawRect(CRect(x, y, x + (w), y + line), col);
    CSprite2d::DrawRect(CRect(x + (w), y, x + (w)-line, y + (h)), col);
    CSprite2d::DrawRect(CRect(x, y + (h), x + (w), y + (h)-line), col);
    CSprite2d::DrawRect(CRect(x, y, x + line, y + (h)), col);
}

static void DrawWayPoint(float x, float y, float w, float h) {
    DrawUnfilledRect(x, y, ScaleY(3.0f), w, h, CRGBA(0, 0, 0, 255));
    DrawUnfilledRect(x, y, ScaleY(1.0f), w * 0.85f, h * 0.85f, CRGBA(255, 0, 0, 255));
}

static void DrawLevel(float x, float y, float w, float h, int type, CRGBA const& col) {
    switch (type) {
    case 1:
        DrawTriangle(x, y, h * 1.6f, DegToRad(0.0f), CRGBA(0, 0, 0, 255));
        DrawTriangle(x, y, h, DegToRad(0.0f), col);
        break;
    case 2:
        DrawTriangle(x, y, h * 1.6f, DegToRad(180.0f), CRGBA(0, 0, 0, 255));
        DrawTriangle(x, y, h, DegToRad(180.0f), col);
        break;
    default:
        CSprite2d::DrawRect(CRect(x - (w * 0.65f), y - (h * 0.65f), x + (w * 0.65f), y + (h * 0.65f)), CRGBA(0, 0, 0, 255));
        CSprite2d::DrawRect(CRect(x - (w * 0.5f), y - (h * 0.5f), x + (w * 0.5f), y + (h * 0.5f)), col);
        break;
    }
}
