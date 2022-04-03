// B_Processors.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "B_Processors.h"
#include "B_DataStructs.h"

COLORREF** B_CreateAuxImage(pBITMAPHANDLE pBitmap) 
{
   COLORREF** output;

   output = (COLORREF**)malloc(pBitmap->Height * sizeof(COLORREF*));
   for (int i = 0; i < pBitmap->Height; i++)
   {
      output[i] = (COLORREF*)malloc(pBitmap->Width * sizeof(COLORREF));
   }

   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         output[j][i] = L_GetPixelColor(pBitmap, j, i);
      }
   }

   return output;
}

double** B_DoubleMalloc_2D(int iHeight, int iWidth)
{
   double **output;
   output = (double**)malloc((iHeight) * sizeof(double*));
   for (int i = 0; i < iHeight; i++)
   {
      output[i] = (double*)malloc((iWidth) * sizeof(double));
   }
   return output;
}

int** B_IntMalloc_2D(int iHeight, int iWidth)
{
   int** output;
   output = (int**)malloc((iHeight) * sizeof(int*));
   for (int i = 0; i < iHeight; i++)
   {
      output[i] = (int*)malloc((iWidth) * sizeof(int));
   }
   return output;
}

BOOL** B_BoolMalloc_2D(int iHeight, int iWidth)
{
   BOOL **output;
   output = (BOOL**)malloc((iHeight) * sizeof(BOOL*));
   for (int i = 0; i < iHeight; i++)
   {
      output[i] = (BOOL*)malloc((iWidth) * sizeof(BOOL));
   }
   return output;
}

YUVREF RGBtoYUV(pBITMAPHANDLE pBitmap)
{
   YUVREF output = { 0 };
   double **dYout, **dUout, **dVout;

   COLORREF **auxImage = B_CreateAuxImage(pBitmap);
   
   dYout = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);
   dUout = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);
   dVout = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);

   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         dYout[j][i] = round((      (0.299000)*GetRValue(auxImage[j][i]) + (0.587000)*GetGValue(auxImage[j][i]) + (0.114000)*GetBValue(auxImage[j][i])));
         dUout[j][i] = round((128 + (-.168736)*GetRValue(auxImage[j][i]) + (-.331264)*GetGValue(auxImage[j][i]) + (0.500000)*GetBValue(auxImage[j][i])));
         dVout[j][i] = round((128 + (0.500000)*GetRValue(auxImage[j][i]) + (-.418688)*GetGValue(auxImage[j][i]) + (-.081312)*GetBValue(auxImage[j][i])));
      }
   }
   
   output.dY = dYout;
   output.dU = dUout;
   output.dV = dVout;

   free(auxImage);

   return output;
}

EXPORT void B_GrayScale(pBITMAPHANDLE pBitmap)
{
   COLORREF    colorGot, colorToPut;
   int         luminosity = 0;

   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         colorGot = L_GetPixelColor(pBitmap, j, i);
         luminosity = (int)round((0.299)*GetRValue(colorGot) + (0.587)*GetGValue(colorGot) + (0.114)*GetBValue(colorGot));
         colorToPut = RGB(luminosity, luminosity, luminosity);
         L_PutPixelColor(pBitmap, j, i, colorToPut);
      }
   }

   return;
}

EXPORT void B_HistogramEqualize_RGB(pBITMAPHANDLE pBitmap)
{
   COLORREF             **colorGot, colorToPut;
   RGB_SUBPIXEL_COUNT   count;
   RGB_IMAGE_PMF        imagePMF;
   RGB_IMAGE_CDF        imageCDF;
   
   colorGot = B_CreateAuxImage(pBitmap);
   
   // Pre-Fill to Zero
   for (int i = 0; i < 256; i++)
   {
      count.subPixel_R[i] = 0;
      count.subPixel_G[i] = 0;
      count.subPixel_B[i] = 0;
   }

   // Count subpixel values
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         count.subPixel_R[GetRValue(colorGot[j][i])]++;
         count.subPixel_G[GetGValue(colorGot[j][i])]++;
         count.subPixel_B[GetBValue(colorGot[j][i])]++;
      }
   }
   
   // Make PMF
   for (int i = 0; i < 256; i++)
   {
      imagePMF.subPixel_R[i] = count.subPixel_R[i] / (B_AREA(pBitmap));
      imagePMF.subPixel_G[i] = count.subPixel_G[i] / (B_AREA(pBitmap));
      imagePMF.subPixel_B[i] = count.subPixel_B[i] / (B_AREA(pBitmap));
   }
   
   //Make CDF
   double tempRSum = 0, tempGSum = 0, tempBSum = 0;
   for (int i = 0; i < 256; i++)
   {
      tempRSum += imagePMF.subPixel_R[i];
      imageCDF.subPixel_R[i] = round(tempRSum * 255);

      tempGSum += imagePMF.subPixel_G[i];
      imageCDF.subPixel_G[i] = round(tempGSum * 255);

      tempBSum += imagePMF.subPixel_B[i];
      imageCDF.subPixel_B[i] = round(tempBSum * 255);
   }
   
   // Write Image
   double Rnew = 0.0, Gnew = 0.0, Bnew = 0.0;
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         colorToPut = RGB( (int)imageCDF.subPixel_R[GetRValue(colorGot[j][i])],
                           (int)imageCDF.subPixel_G[GetGValue(colorGot[j][i])],
                           (int)imageCDF.subPixel_B[GetBValue(colorGot[j][i])]);

         L_PutPixelColor(pBitmap, j, i, colorToPut);

      }
   }

   free(colorGot);

   return;
}

EXPORT void B_HistogramEqualize_YUV(pBITMAPHANDLE pBitmap)
{
   COLORREF             **colorGot, colorToPut;
   YUVREF               colorGot_YUV, colorToPut_YUV;
   YUV_SUBPIXEL_COUNT   count;
   YUV_IMAGE_PMF        imagePMF;
   YUV_IMAGE_CDF        imageCDF;
   
   colorGot = B_CreateAuxImage(pBitmap);
   colorGot_YUV = RGBtoYUV(pBitmap);
   
   colorToPut_YUV.dY = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);
   colorToPut_YUV.dU = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);
   colorToPut_YUV.dV = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);

   // Pre-Fill to Zero
   for (int i = 0; i < 256; i++)
   {
      count.subPixel_Y[i] = 0;
      count.subPixel_U[i] = 0;
      count.subPixel_V[i] = 0;
   }

   // Count subpixel values
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         count.subPixel_Y[(int)colorGot_YUV.dY[j][i]]++;
         count.subPixel_U[(int)colorGot_YUV.dU[j][i]]++;
         count.subPixel_V[(int)colorGot_YUV.dV[j][i]]++;
      }
   }
   
   // Make PMF
   for (int i = 0; i < 256; i++)
   {
      imagePMF.subPixel_Y[i] = count.subPixel_Y[i] / (B_AREA(pBitmap));
      imagePMF.subPixel_U[i] = count.subPixel_U[i] / (B_AREA(pBitmap));
      imagePMF.subPixel_V[i] = count.subPixel_V[i] / (B_AREA(pBitmap));
   }
   
   //Make CDF
   double tempRSum = 0, tempGSum = 0, tempBSum = 0;
   for (int i = 0; i < 256; i++)
   {
      tempRSum += imagePMF.subPixel_Y[i];
      imageCDF.subPixel_Y[i] = round(tempRSum * 255);

      tempGSum += imagePMF.subPixel_U[i];
      imageCDF.subPixel_U[i] = round(tempGSum * 255);

      tempBSum += imagePMF.subPixel_V[i];
      imageCDF.subPixel_V[i] = round(tempBSum * 255);
   }
   
   // Write Image
   double Rnew = 0.0, Gnew = 0.0, Bnew = 0.0;
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         colorToPut_YUV.dY[j][i] = imageCDF.subPixel_Y[(int)colorGot_YUV.dY[j][i]];
         colorToPut_YUV.dU[j][i] = colorGot_YUV.dU[j][i];
         colorToPut_YUV.dV[j][i] = colorGot_YUV.dV[j][i];

         // YUV -> RGB conversions from wikipedia
         Rnew = round(colorToPut_YUV.dY[j][i]                                              + ((1.4075) * (colorToPut_YUV.dV[j][i] - 128)));
         Gnew = round(colorToPut_YUV.dY[j][i] - ((0.3455) * (colorGot_YUV.dU[j][i] - 128)) - ((0.7169) * (colorToPut_YUV.dV[j][i] -128)));
         Bnew = round(colorToPut_YUV.dY[j][i] + ((1.7790) * (colorGot_YUV.dU[j][i] - 128)));

         // Manage "Out of Bounds" Colors
         if (Rnew > 255) { Rnew = 255; }
         if (Rnew < 0) { Rnew = 0; }

         if (Gnew > 255) { Gnew = 255; }
         if (Gnew < 0) { Gnew = 0; }

         if (Bnew > 255) { Bnew = 255; }
         if (Bnew < 0) { Bnew = 0; }

         colorToPut = RGB(Rnew, Gnew, Bnew);

         L_PutPixelColor(pBitmap, j, i, colorToPut);

      }
   }

   free(colorGot);
   free(colorGot_YUV.dY);
   free(colorGot_YUV.dU);
   free(colorGot_YUV.dV);
   free(colorToPut_YUV.dY);
   free(colorToPut_YUV.dU);
   free(colorToPut_YUV.dV);

   return;
}

EXPORT void B_MinFilter(pBITMAPHANDLE pBitmap, int kernalSize)
{
   COLORREF**     auxImage = B_CreateAuxImage(pBitmap);

   RGB_IMAGE_PMF  imagePMF;

   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < 256; i++)
      {
         imagePMF.subPixel_R[i] = 0;
         imagePMF.subPixel_G[i] = 0;
         imagePMF.subPixel_B[i] = 0;
      }

      for (int i = 0; i < pBitmap->Width; i++)
      {
         if (i == 0)
         {
            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               for (int c = i - kernalSize / 2; c < i + kernalSize / 2 + 1; c++)
               {
                  int r_temp;
                  int c_temp;

                  if (r < 0) { r_temp = 0; }
                  else { r_temp = r; }

                  if (c < 0) { c_temp = 0; }
                  else { c_temp = c; }

                  if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

                  if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

                  imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]++;
                  imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]++;
                  imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]++;
               }
            }
         }
         else
         {
            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               int c = i + kernalSize / 2;

               int r_temp;
               int c_temp;

               if (r < 0) { r_temp = 0; }
               else { r_temp = r; }

               if (c < 0) { c_temp = 0; }
               else { c_temp = c; }

               if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

               if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

               imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]++;
               imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]++;
               imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]++;
            }

            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               int c = i - kernalSize / 2 - 1;

               int r_temp;
               int c_temp;

               if (r < 0) { r_temp = 0; }
               else { r_temp = r; }

               if (c < 0) { c_temp = 0; }
               else { c_temp = c; }

               if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

               if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

               imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]--;
               imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]--;
               imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]--;
            }
         }

         // Get Mins
         int min_red = 0, min_gre = 0, min_blu = 0;
         for (int i = 0; i < 256; i++)
         {
            if (imagePMF.subPixel_R[i] != 0)
            {
               min_red = i;
               break;
            }
         }

         for (int i = 0; i < 256; i++)
         {
            if (imagePMF.subPixel_G[i] != 0)
            {
               min_gre = i;
               break;
            }
         }

         for (int i = 0; i < 256; i++)
         {
            if (imagePMF.subPixel_B[i] != 0)
            {
               min_blu = i;
               break;
            }
         }

         // Write Image
         L_PutPixelColor(pBitmap, j, i, RGB(min_red, min_gre, min_blu));
      }
   }

   free(auxImage);

   return;
}

EXPORT void B_MaxFilter(pBITMAPHANDLE pBitmap, int kernalSize)
{
   COLORREF**     auxImage = B_CreateAuxImage(pBitmap);

   RGB_IMAGE_PMF  imagePMF;

   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < 256; i++)
      {
         imagePMF.subPixel_R[i] = 0;
         imagePMF.subPixel_G[i] = 0;
         imagePMF.subPixel_B[i] = 0;
      }

      for (int i = 0; i < pBitmap->Width; i++)
      {
         if (i == 0)
         {
            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               for (int c = i - kernalSize / 2; c < i + kernalSize / 2 + 1; c++)
               {
                  int r_temp;
                  int c_temp;

                  if (r < 0) { r_temp = 0; }
                  else { r_temp = r; }

                  if (c < 0) { c_temp = 0; }
                  else { c_temp = c; }

                  if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

                  if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

                  imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]++;
                  imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]++;
                  imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]++;
               }
            }
         }
         else
         {
            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               int c = i + kernalSize / 2;

               int r_temp;
               int c_temp;

               if (r < 0) { r_temp = 0; }
               else { r_temp = r; }

               if (c < 0) { c_temp = 0; }
               else { c_temp = c; }

               if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

               if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

               imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]++;
               imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]++;
               imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]++;
            }

            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               int c = i - kernalSize / 2 - 1;

               int r_temp;
               int c_temp;

               if (r < 0) { r_temp = 0; }
               else { r_temp = r; }

               if (c < 0) { c_temp = 0; }
               else { c_temp = c; }

               if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

               if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

               imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]--;
               imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]--;
               imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]--;
            }
         }

         // Get Maxes
         int max_red = 0, max_gre = 0, max_blu = 0;
         for (int i = 255; i >= 0; i--)
         {
            if (imagePMF.subPixel_R[i] != 0)
            {
               max_red = i;
               break;
            }
         }

         for (int i = 255; i >= 0; i--)
         {
            if (imagePMF.subPixel_G[i] != 0)
            {
               max_gre = i;
               break;
            }
         }

         for (int i = 255; i >= 0; i--)
         {
            if (imagePMF.subPixel_B[i] != 0)
            {
               max_blu = i;
               break;
            }
         }

         // Write Image
         L_PutPixelColor(pBitmap, j, i, RGB(max_red, max_gre, max_blu));
      }
   }

   free(auxImage);

   return;
}

EXPORT void B_MedFilter(pBITMAPHANDLE pBitmap, int kernalSize)
{
   COLORREF**     auxImage = B_CreateAuxImage(pBitmap);

   RGB_IMAGE_PMF  imagePMF;

   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < 256; i++)
      {
         imagePMF.subPixel_R[i] = 0;
         imagePMF.subPixel_G[i] = 0;
         imagePMF.subPixel_B[i] = 0;
      }

      for (int i = 0; i < pBitmap->Width; i++)
      {
         if (i == 0)
         {
            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               for (int c = i - kernalSize / 2; c < i + kernalSize / 2 + 1; c++)
               {
                  int r_temp;
                  int c_temp;

                  if (r < 0) { r_temp = 0; }
                  else { r_temp = r; }

                  if (c < 0) { c_temp = 0; }
                  else { c_temp = c; }

                  if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

                  if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

                  imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]++;
                  imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]++;
                  imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]++;
               }
            }
         }
         else
         {
            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               int c = i + kernalSize / 2;

               int r_temp;
               int c_temp;

               if (r < 0) { r_temp = 0; }
               else { r_temp = r; }

               if (c < 0) { c_temp = 0; }
               else { c_temp = c; }

               if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

               if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

               imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]++;
               imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]++;
               imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]++;
            }

            for (int r = j - kernalSize / 2; r < j + kernalSize / 2 + 1; r++)
            {
               int c = i - kernalSize / 2 - 1;

               int r_temp;
               int c_temp;

               if (r < 0) { r_temp = 0; }
               else { r_temp = r; }

               if (c < 0) { c_temp = 0; }
               else { c_temp = c; }

               if (r >= pBitmap->Height) { r_temp = pBitmap->Height - 1; }

               if (c >= pBitmap->Width) { c_temp = pBitmap->Width - 1; }

               imagePMF.subPixel_R[GetRValue(auxImage[r_temp][c_temp])]--;
               imagePMF.subPixel_G[GetGValue(auxImage[r_temp][c_temp])]--;
               imagePMF.subPixel_B[GetBValue(auxImage[r_temp][c_temp])]--;
            }
         }

         // Get Meds
         int med_red = 0, med_gre = 0, med_blu = 0;
         int med_count = 0;
         for (int i = 0; i < 256; i++)
         {
            med_count += imagePMF.subPixel_R[i];

            if (med_count >= kernalSize * kernalSize / 2 + 1)
            {
               med_red = i;
               med_count = 0;
               break;
            }
         }

         for (int i = 0; i < 256; i++)
         {
            med_count += imagePMF.subPixel_G[i];

            if (med_count >= kernalSize * kernalSize / 2 + 1)
            {
               med_gre = i;
               med_count = 0;
               break;
            }
         }

         for (int i = 0; i < 256; i++)
         {
            med_count += imagePMF.subPixel_B[i];

            if (med_count >= kernalSize * kernalSize / 2 + 1)
            {
               med_blu = i;
               med_count = 0;
               break;
            }
         }

         // Write Image
         L_PutPixelColor(pBitmap, j, i, RGB(med_red, med_gre, med_blu));
      }
   }

   free(auxImage);

   return;
}

EXPORT void B_MeanFilter(pBITMAPHANDLE pBitmap, int kernalSize)
{
   COLORREF**  auxImage = B_CreateAuxImage(pBitmap);
   double**    intImage_R = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);
   double**    intImage_G = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);
   double**    intImage_B = B_DoubleMalloc_2D(pBitmap->Height, pBitmap->Width);

   // Preset to Zero
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         intImage_R[j][i] = 0;
         intImage_G[j][i] = 0;
         intImage_B[j][i] = 0;
      }
   }

   // Make intImage
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         if (j == 0 && i == 0)
         {
            intImage_R[j][i] = GetRValue(auxImage[0][0]);
            intImage_G[j][i] = GetGValue(auxImage[0][0]);
            intImage_B[j][i] = GetBValue(auxImage[0][0]);
         }
         else if (j == 0)
         {
            intImage_R[j][i] = intImage_R[j][i - 1] +
               GetRValue(auxImage[j][i]);

            intImage_G[j][i] = intImage_G[j][i - 1] +
               GetGValue(auxImage[j][i]);

            intImage_B[j][i] = intImage_B[j][i - 1] +
               GetBValue(auxImage[j][i]);
         }
         else if (i == 0)
         {
            intImage_R[j][i] = intImage_R[j - 1][i] +
               GetRValue(auxImage[j][i]);

            intImage_G[j][i] = intImage_G[j - 1][i] +
               GetGValue(auxImage[j][i]);

            intImage_B[j][i] = intImage_B[j - 1][i] +
               GetBValue(auxImage[j][i]);
         }
         else
         {
            intImage_R[j][i] = intImage_R[j][i - 1] +
               intImage_R[j - 1][i] -
               intImage_R[j - 1][i - 1] +
               GetRValue(auxImage[j][i]);

            intImage_G[j][i] = intImage_G[j][i - 1] +
               intImage_G[j - 1][i] -
               intImage_G[j - 1][i - 1] +
               GetGValue(auxImage[j][i]);

            intImage_B[j][i] = intImage_B[j][i - 1] +
               intImage_B[j - 1][i] -
               intImage_B[j - 1][i - 1] +
               GetBValue(auxImage[j][i]);
         }

      }
   }

   // Calc. Averages
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         B_POINT botRi, topRi, botLe, topLe;
         double average_R, average_G, average_B;

         botRi.iCx = i + kernalSize / 2;
         botRi.iCy = j + kernalSize / 2;

         topRi.iCx = i + kernalSize / 2;
         topRi.iCy = j - kernalSize / 2 - 1;

         botLe.iCx = i - kernalSize / 2 - 1;
         botLe.iCy = j + kernalSize / 2;

         topLe.iCx = i - kernalSize / 2 - 1;
         topLe.iCy = j - kernalSize / 2 - 1;

         if (botRi.iCx >= pBitmap->Width) { botRi.iCx = pBitmap->Width - 1; }
         if (botRi.iCy >= pBitmap->Height) { botRi.iCy = pBitmap->Height - 1; }

         if (topRi.iCx >= pBitmap->Width) { topRi.iCx = pBitmap->Width - 1; }
         if (topRi.iCy < 0) { topRi.iCy = 0; }

         if (botLe.iCx < 0) { botLe.iCx = 0; }
         if (botLe.iCy >= pBitmap->Height) { botLe.iCy = pBitmap->Height - 1; }

         if (topLe.iCx < 0) { topLe.iCx = 0; }
         if (topLe.iCy < 0) { topLe.iCy = 0; }

         average_R = intImage_R[botRi.iCy][botRi.iCx] -
            intImage_R[topRi.iCy][topRi.iCx] -
            intImage_R[botLe.iCy][botLe.iCx] +
            intImage_R[topLe.iCy][topLe.iCx];
         average_R /= kernalSize * kernalSize;

         average_G = intImage_G[botRi.iCy][botRi.iCx] -
            intImage_G[topRi.iCy][topRi.iCx] -
            intImage_G[botLe.iCy][botLe.iCx] +
            intImage_G[topLe.iCy][topLe.iCx];
         average_G /= kernalSize * kernalSize;

         average_B = intImage_B[botRi.iCy][botRi.iCx] -
            intImage_B[topRi.iCy][topRi.iCx] -
            intImage_B[botLe.iCy][botLe.iCx] +
            intImage_B[topLe.iCy][topLe.iCx];
         average_B /= kernalSize * kernalSize;

         L_PutPixelColor(pBitmap, j, i, RGB(average_R, average_G, average_B));
      }
   }

   free(auxImage);
   free(intImage_R);
   free(intImage_G);
   free(intImage_B);

   return;
}

EXPORT void B_RegionGrow(pBITMAPHANDLE pBitmap, int iCy, int iCx, int distance_thres, B_NEIGHBORNUMBER neighborNumber)
{
   COLORREF **colorGot, colorToCompare;
   BOOL     **virtImage;
   B_Queue  nodeQueue;

   virtImage = B_BoolMalloc_2D(pBitmap->Height, pBitmap->Width);
   for (int j = 0; j < pBitmap->Height; j++)
   {
      for (int i = 0; i < pBitmap->Width; i++)
      {
         virtImage[j][i] = TRUE;
      }
   }

   if (iCy < 0) { iCy = 0; }
   if (iCx < 0) { iCx = 0; }

   if (iCy >= pBitmap->Height) { iCy = pBitmap->Height - 1; }
   if (iCx >= pBitmap->Width) { iCx = pBitmap->Width - 1; }

   B_POINT seedPoint = { iCx, iCy };

   colorGot = B_CreateAuxImage(pBitmap);
   colorToCompare = L_GetPixelColor(pBitmap, iCy, iCx);

   B_InitQueue(&nodeQueue, sizeof(B_POINT));
   B_PushQueue(&nodeQueue, &seedPoint);

   while ( !B_QueueIsEmpty(&nodeQueue) )
   {
      B_POINT* curr_point = (B_POINT*)B_QueueFront(&nodeQueue);
      int r = 0;
      int c = 0;

      for (int direction = 0; direction < neighborNumber; direction++)
      {
         switch (direction % neighborNumber)
         {
         case 0:
            r = curr_point->iCy - 1;
            c = curr_point->iCx;
            break;

         case 1:
            r = curr_point->iCy;
            c = curr_point->iCx - 1;
            break;

         case 2:
            r = curr_point->iCy + 1;
            c = curr_point->iCx;
            break;

         case 3:
            r = curr_point->iCy;
            c = curr_point->iCx + 1;
            break;
 
         case 4:
            r = curr_point->iCy - 1;
            c = curr_point->iCx - 1;
            break;

         case 5:
            r = curr_point->iCy + 1;
            c = curr_point->iCx - 1;
            break;

         case 6:
            r = curr_point->iCy + 1;
            c = curr_point->iCx + 1;
            break;

         case 7:
            r = curr_point->iCy - 1;
            c = curr_point->iCx + 1;
            break;
         }

         if ((r >= 0) && (c >= 0) && (r < pBitmap->Height) && (c < pBitmap->Width))
         {
            if (virtImage[r][c] == TRUE)
            {
               double distance2_calc = pow(GetRValue(colorToCompare) - GetRValue(colorGot[r][c]), 2) +
                                       pow(GetGValue(colorToCompare) - GetGValue(colorGot[r][c]), 2) +
                                       pow(GetBValue(colorToCompare) - GetBValue(colorGot[r][c]), 2);
               
               int distance_calc = (int)sqrt(distance2_calc);

               if (distance_calc <= distance_thres)
               {
                  B_POINT point = { c, r };
                  B_PushQueue(&nodeQueue, &point);
               }

               virtImage[r][c] = FALSE;
            }
         }

      }

      L_PutPixelColor(pBitmap, curr_point->iCy, curr_point->iCx, RGB(255, 0, 255));
      virtImage[curr_point->iCy][curr_point->iCx] = FALSE;
      B_PopQueue(&nodeQueue);
   }

   free(colorGot);
   free(virtImage);
}
