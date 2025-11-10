#pragma once
#include "CTimer.h"
#include "extensions/Screen.h"
#include "CSprite2d.h"
#include "CText.h"

#define DEFAULT_SCREEN_WIDTH 640.0f
#define DEFAULT_SCREEN_HEIGHT 480.0f
#define DEFAULT_SCREEN_ASPECT_RATIO DEFAULT_SCREEN_WIDTH / DEFAULT_SCREEN_HEIGHT
#define flashItem(on, off) (CTimer::m_snTimeInMillisecondsPauseMode % on + off < on)
static float GetAspectRatio() {
#ifdef GTA3
    float& fScreenAspectRatio = *(float*)0x5F53C0;
#elif GTAVC
    float& fScreenAspectRatio = *(float*)0x94DD38;
#elif GTASA
    float& fScreenAspectRatio = CDraw::ms_fAspectRatio;
#endif
    return fScreenAspectRatio;
}

#define SCREEN_ASPECT_RATIO GetAspectRatio() // (SCREEN_WIDTH / SCREEN_HEIGHT)

static float fScaleMult = 1.0f;

static void SetScaleMult(float mult) {
    fScaleMult = mult;
}

static float ScaleX(float x) {
    float f = ((x) * (float)SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH) * DEFAULT_SCREEN_ASPECT_RATIO / SCREEN_ASPECT_RATIO;
    return f * fScaleMult;
}

static float ScaleXKeepCentered(float x) {
    float f = ((SCREEN_WIDTH == DEFAULT_SCREEN_WIDTH) ? (x) : (SCREEN_WIDTH - ScaleX(DEFAULT_SCREEN_WIDTH)) / 2 + ScaleX((x)));
    return f;
}

static float ScaleY(float y) {
    float f = ((y) * (float)SCREEN_HEIGHT / DEFAULT_SCREEN_HEIGHT);
    return f * fScaleMult;
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

static void DrawWayPoint(float x, float y, float w, float h, CRGBA col) {
    DrawUnfilledRect(x, y, ScaleY(3.0f), w, h, CRGBA(0, 0, 0, col.a));
    DrawUnfilledRect(x, y, ScaleY(1.0f), w * 0.85f, h * 0.85f, col);
}

static void DrawLevel(float x, float y, float w, float h, int type, CRGBA const& col) {
    switch (type) {
    case 1:
        DrawTriangle(x, y + ScaleY(0.5f), h * 1.5f, plugin::DegToRad(0.0f), CRGBA(0, 0, 0, 255));
        DrawTriangle(x, y, h, plugin::DegToRad(0.0f), col);
        break;
    case 2:
        DrawTriangle(x, y - ScaleY(0.5f), h * 1.5f, plugin::DegToRad(180.0f), CRGBA(0, 0, 0, 255));
        DrawTriangle(x, y, h, plugin::DegToRad(180.0f), col);
        break;
    default:
        CSprite2d::DrawRect(CRect(x - (w * 0.65f), y - (h * 0.65f), x + (w * 0.65f), y + (h * 0.65f)), CRGBA(0, 0, 0, 255));
        CSprite2d::DrawRect(CRect(x - (w * 0.5f), y - (h * 0.5f), x + (w * 0.5f), y + (h * 0.5f)), col);
        break;
    }
}

static wchar_t UpperCaseTable[128] = {
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
    139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137,
    138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
    149, 173, 173, 175, 176, 177, 178, 179, 180, 181, 182,
    183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193,
    194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204,
    205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
    227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
    238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248,
    249, 250, 251, 252, 253, 254, 255
};

static wchar_t FrenchUpperCaseTable[128] = {
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
    139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 65, 65, 65, 65, 132, 133, 69, 69, 69, 69, 73, 73,
    73, 73, 79, 79, 79, 79, 85, 85, 85, 85, 173, 173, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186,
    187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
    198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
    209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
    231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241,
    242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
    253, 254, 255
};

static wchar_t GetUpperCase(wchar_t c) {
    if (c >= 'a' && c <= 'z')
        return c - 32;

    switch (TheText.encoding) {
    case 'e':
        break;
    case 'f':
        if (c >= 128 && c <= 255)
            return FrenchUpperCaseTable[c - 128];
        break;
    case 'g':
    case 'i':
    case 's':
        if (c >= 128 && c <= 255)
            return UpperCaseTable[c - 128];
        break;
    default:
        break;
    }
    return c;
}

static wchar_t GetLowerCase(wchar_t c) {
    if (c >= 'A' && c <= 'Z')
        return c + 32;
}

static std::wstring wbuff = {};
static wchar_t* UpperCase(const wchar_t* s) {
    wbuff = s;
    for (auto& it : wbuff)
        it = GetUpperCase(it);

    return (wchar_t*)wbuff.c_str();
}

static wchar_t* LowerCase(wchar_t* s) {
    wbuff = s;
    for (auto& it : wbuff)
        it = GetLowerCase(it);

    return (wchar_t*)wbuff.c_str();
}

static float Ease(float start, float end, float t) {
    t = std::clamp(t, 0.0f, 1.0f);

    if (t < 0.5f) {
        t = 4.0f * t * t * t;
    }
    else {
        t = 1.0f - std::pow(-2.0f * t + 2.0f, 3) / 2.0f;
    }

    return start + (end - start) * t;
}
