#pragma once
#include "L_Bitmap.h"
#include "commctrl.h"

#define EXPORT __declspec(dllexport)

#define  B_AREA(pBitmap)         pBitmap->Width * pBitmap->Height

typedef struct
{
   double   **dY;
   double   **dU;
   double   **dV;
}
YUVREF, *pYUVREF;

typedef struct
{
   double         subPixel_R[256];
   double         subPixel_G[256];
   double         subPixel_B[256];
}
RGB_IMAGE_PMF, *pRGB_IMAGE_PMF,
RGB_IMAGE_CDF, *pRGB_IMAGE_CDF,
RGB_SUBPIXEL_COUNT;

typedef struct
{
   double         subPixel_Y[256];
   double         subPixel_U[256];
   double         subPixel_V[256];
}
YUV_IMAGE_PMF, *pYUV_IMAGE_PMF,
YUV_IMAGE_CDF, *pYUV_IMAGE_CDF,
YUV_SUBPIXEL_COUNT;

typedef enum { FOUR = 4, EIGHT = 8 } B_NEIGHBORNUMBER;

EXPORT void B_GrayScale(pBITMAPHANDLE);
EXPORT void B_HistogramEqualize_RGB(pBITMAPHANDLE);
EXPORT void B_HistogramEqualize_YUV(pBITMAPHANDLE);
EXPORT void B_MinFilter(pBITMAPHANDLE, int);
EXPORT void B_MaxFilter(pBITMAPHANDLE, int);
EXPORT void B_MedFilter(pBITMAPHANDLE, int);
EXPORT void B_MeanFilter(pBITMAPHANDLE, int);
EXPORT void B_RegionGrow(pBITMAPHANDLE, int, int, int, B_NEIGHBORNUMBER);


// TRAFFIC LIGHT FUNCTIONS //

extern   TCHAR       g_szTLClass[];
extern   HINSTANCE   g_hinst;
extern   HMENU       g_hMenuInit, g_hMenuTL;
extern   HMENU       g_hMenuInitWindow, g_hMenuTLWindow;
extern   HWND        g_hwndClient, g_hwndFrame;

typedef enum { RED, GREEN, BLUE } B_LIGHTCOLOR;

typedef struct
{
   int iCx;
   int iCy;
}
B_POINT;

typedef struct
{
   int            iLightCount;
   int            *piTimerID;
   int            *piLightPeriod;
   B_LIGHTCOLOR   *pStartColor;
   B_POINT        *piLightCoordinates;
   WNDPROC        origProc;
   int            memoryLocation;
}
TLCREATESTRUCT, *pTLCREATESTRUCT;

typedef struct tagTLDATA
{
   int            iLightCount;
   int            *piTimerID;
   int            *piLightPeriod;
   B_LIGHTCOLOR   *plightColor;
   B_POINT        *piLightCoordinates;
}
TLDATA, *PTLDATA;

EXPORT LRESULT CALLBACK TLProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

static   BOOL        OnCreate(HWND, CREATESTRUCT*);
static   VOID        OnTimer(HWND, UINT);
static   VOID        OnSize(HWND, UINT, int, int);
static   VOID        OnPaint(HWND);
static   VOID        OnClose(HWND);
static   VOID        OnDestroy(HWND);

void B_CreateTrafficLight(HWND, TLCREATESTRUCT);
void B_DrawLight(HWND, int, int);
void B_DrawCircle(HWND hwnd, int radius, int xPos, int yPos, int red, int green, int blue, int cxClient, int cyClient);

