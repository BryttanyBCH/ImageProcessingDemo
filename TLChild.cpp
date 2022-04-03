#include "StdAfx.h"
#include "B_Processors.h"

TCHAR       g_szTLClass[] = TEXT("MdiTLChild");
HINSTANCE   g_hinst;
HMENU       g_hMenuInit, g_hMenuTL;
HMENU       g_hMenuInitWindow, g_hMenuTLWindow;
HWND        g_hwndClient, g_hwndFrame;

int         memoryLocation;

EXPORT LRESULT CALLBACK TLProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
   memoryLocation = ((pTLCREATESTRUCT)dwRefData)->memoryLocation;

   switch (message)
   {
   case WM_CREATE:
   {
      OnCreate(hwnd, (CREATESTRUCT*)dwRefData);
      CallWindowProc(((pTLCREATESTRUCT)dwRefData)->origProc, hwnd, message, wParam, lParam);
   }
   break;
   case WM_TIMER:
   {
      OnTimer(hwnd, (UINT)(wParam));
      CallWindowProc(((pTLCREATESTRUCT)dwRefData)->origProc, hwnd, message, wParam, lParam);
   }
   break;
   case WM_SIZE:
   {
      CallWindowProc(((pTLCREATESTRUCT)dwRefData)->origProc, hwnd, message, wParam, lParam);
      OnSize(hwnd, (UINT)wParam, LOWORD(lParam), HIWORD(lParam));
   }
   break;
   case WM_PAINT:
   {
      CallWindowProc(((pTLCREATESTRUCT)dwRefData)->origProc, hwnd, message, wParam, lParam);
      OnPaint(hwnd);
   }
   break;
   case WM_DESTROY:
   {
      OnDestroy(hwnd);
      CallWindowProc(((pTLCREATESTRUCT)dwRefData)->origProc, hwnd, message, wParam, lParam);
   }
   break;
   }

   return DefSubclassProc(hwnd, message, wParam, lParam);
}

static BOOL OnCreate(HWND hwnd, CREATESTRUCT* lpCreateStruct)
{
   pTLCREATESTRUCT pTLCreate = (pTLCREATESTRUCT)lpCreateStruct;

   PTLDATA pTLData = (PTLDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TLDATA));

   pTLData->iLightCount = pTLCreate->iLightCount;
   pTLData->piTimerID = pTLCreate->piTimerID;
   pTLData->piLightPeriod = pTLCreate->piLightPeriod;
   pTLData->plightColor = pTLCreate->pStartColor;
   pTLData->piLightCoordinates = pTLCreate->piLightCoordinates;

   SetWindowLong(hwnd, memoryLocation, (long)pTLData);

   for (int i = 0; i < pTLData->iLightCount; i++)
   {
      SetTimer(hwnd, pTLData->piTimerID[i], pTLData->piLightPeriod[i], NULL);
   }

   g_hwndClient = GetParent(hwnd);
   g_hwndFrame = GetParent(g_hwndClient);

   return TRUE;
}

static VOID OnTimer(HWND hwnd, UINT id)
{
   PTLDATA pTLData = (PTLDATA)GetWindowLong(hwnd, memoryLocation);
   for (int i = 0; i < pTLData->iLightCount; i++)
   {
      if (id == (UINT)pTLData->piTimerID[i])
      {
         B_DrawLight(hwnd, 1, pTLData->piTimerID[i]);
      }
   }
}

static VOID OnSize(HWND hwnd, UINT iState, int iCx, int iCy)
{
   PTLDATA pTLData = (PTLDATA)GetWindowLong(hwnd, memoryLocation);
   for (int i = 0; i < pTLData->iLightCount; i++)
   {
      B_DrawLight(hwnd, 0, pTLData->piTimerID[i]);
   }

   FORWARD_WM_SIZE(hwnd, iState, iCx, iCy, DefMDIChildProc);
}

static VOID OnPaint(HWND hwnd)
{
   PTLDATA pTLData = (PTLDATA)GetWindowLong(hwnd, memoryLocation);
   for (int i = 0; i < pTLData->iLightCount; i++)
   {
      B_DrawLight(hwnd, 0, pTLData->piTimerID[i]);
   }
}

static VOID OnDestroy(HWND hwnd)
{
   PTLDATA pTLData = (PTLDATA)GetWindowLong(hwnd, memoryLocation);

   free(pTLData->piLightCoordinates);
   free(pTLData->piLightPeriod);
   free(pTLData->piTimerID);
   free(pTLData->plightColor);

   HeapFree(GetProcessHeap(), 0, pTLData);
}


void B_DrawLight(HWND hwnd, int advance, int timerID)
{
   HBRUSH               hBrush;
   PAINTSTRUCT          ps;
   HDC                  hdc = BeginPaint(hwnd, &ps);
   RECT                 rect;

   PTLDATA pTLData = (PTLDATA)GetWindowLong(hwnd, memoryLocation);

   for (int i = 0; i < pTLData->iLightCount; i++)
   {
      if (timerID == pTLData->piTimerID[i])
      {
         int cxCenter = pTLData->piLightCoordinates[i].iCx;
         int cyCenter = pTLData->piLightCoordinates[i].iCy;

         if (cxCenter <= 0 || cyCenter <= 0)
         {
            return;
         }

         SetRect(&rect, cxCenter / 2 - 100, cyCenter / 2 + 200,
            cxCenter / 2 + 100, cyCenter / 2 - 200);

         hBrush = CreateSolidBrush(RGB(0xc9, 0xc9, 0xc9));

         hdc = GetDC(hwnd);
         FillRect(hdc, &rect, hBrush);
         ReleaseDC(hwnd, hdc);
         DeleteObject(hBrush);

         switch (pTLData->plightColor[i])
         {
         case RED:
            B_DrawCircle(hwnd, 50, 0, 125, 0xFF, 0x00, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, 0, 0x00, 0x33, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, -125, 0x00, 0x00, 0x33, cxCenter, cyCenter);
            if (advance == 1)
            {
               pTLData->plightColor[i] = GREEN;
            }
            if (advance == -1)
            {
               pTLData->plightColor[i] = BLUE;
            }
            EndPaint(hwnd, &ps);
            return;
         case GREEN:
            B_DrawCircle(hwnd, 50, 0, 125, 0x33, 0x00, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, 0, 0x00, 0xFF, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, -125, 0x00, 0x00, 0x33, cxCenter, cyCenter);
            if (advance == 1)
            {
               pTLData->plightColor[i] = BLUE;
            }
            if (advance == -1)
            {
               pTLData->plightColor[i] = RED;
            }
            EndPaint(hwnd, &ps);
            return;
         case BLUE:
            B_DrawCircle(hwnd, 50, 0, 125, 0x33, 0x00, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, 0, 0x00, 0x33, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, -125, 0x00, 0x00, 0xff, cxCenter, cyCenter);
            if (advance == 1)
            {
               pTLData->plightColor[i] = RED;
            }
            if (advance == -1)
            {
               pTLData->plightColor[i] = GREEN;
            }
            EndPaint(hwnd, &ps);
            return;
         default:
            B_DrawCircle(hwnd, 50, 0, 125, 0x33, 0x00, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, 0, 0x00, 0x33, 0x00, cxCenter, cyCenter);
            B_DrawCircle(hwnd, 50, 0, -125, 0x00, 0x00, 0x33, cxCenter, cyCenter);
            return;
         }
      }
   }
}

void B_DrawCircle(HWND hwnd, int radius, int xPos, int yPos, int red, int green, int blue, int cxCenter, int cyCenter)
{
   HBRUSH      hBrushDef;
   HBRUSH      hBrushSet;
   HDC         hdc = GetDC(hwnd);
   RECT        rect;

   if (cxCenter <= 0 || cyCenter <= 0)
   {
      return;
   }

   SetRect(&rect, cxCenter / 2 - radius + xPos, cyCenter / 2 + radius - yPos,
      cxCenter / 2 + radius + xPos, cyCenter / 2 - radius - yPos);

   hBrushDef = CreateSolidBrush(RGB(red, green, blue));
   hBrushSet = (HBRUSH)SelectObject(hdc, hBrushDef);

   Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);

   SelectObject(hdc, hBrushSet);

   ReleaseDC(hwnd, hdc);
   DeleteObject(hBrushDef);
}
