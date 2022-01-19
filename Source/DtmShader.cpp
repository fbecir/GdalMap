//==============================================================================
// DtmShader.cpp
// Created: 19 Jan 2022 4:41:34pm
// Author:  FBecirspahic
// Estompage / ombrage des MNT (tire de XDtmShader, adapte a JUCE)
//==============================================================================

#include <cmath>
#include "DtmShader.h"

// Preferences d'affichage
double DtmShader::XPI = 3.14159265358979323846;
double DtmShader::m_dNoData = -999.;
double DtmShader::m_dZ0 = 0.;
double DtmShader::m_dZ1 = 200.;
double DtmShader::m_dZ2 = 400.;
double DtmShader::m_dZ3 = 600.;
double DtmShader::m_dZ4 = 5500.;
int DtmShader::m_nMode = 6;

/*
juce::uint8 DtmShader::m_HZColor[3] = { 255, 0, 0 };
juce::uint8 DtmShader::m_Z0Color[3] = { 3, 34, 76 };
juce::uint8 DtmShader::m_ZColor[15] = { 64, 128, 128, 255, 255, 0, 255, 128, 0,
                                        128, 64, 0, 240, 240, 240 };
                                        */
juce::uint8 DtmShader::m_HZColor[3] = { 0, 0, 255 };
juce::uint8 DtmShader::m_Z0Color[3] = { 76, 34, 3 };
juce::uint8 DtmShader::m_ZColor[15] = { 128, 128, 64, 0, 255, 255, 0, 128, 255,
                                        0, 64, 128, 240, 240, 240 };



void DtmShader::SetPref32Bits(double nodata, double z0, double z1, double z2,
  double z3, double z4, int mode)
{
  m_dNoData = nodata;
  m_dZ0 = z0;
  m_dZ1 = z1;
  m_dZ2 = z2;
  m_dZ3 = z3;
  m_dZ4 = z4;
  m_nMode = mode;
}

void DtmShader::SetCol32Bits(juce::uint8* Color, bool set)
{
  if (set) {
    ::memcpy(m_HZColor, Color, 3 * sizeof(juce::uint8));
    ::memcpy(m_Z0Color, &Color[3], 3 * sizeof(juce::uint8));
    ::memcpy(m_ZColor, &Color[6], 15 * sizeof(juce::uint8));
  }
  else {
    ::memcpy(Color, m_HZColor, 3 * sizeof(juce::uint8));
    ::memcpy(&Color[3], m_Z0Color, 3 * sizeof(juce::uint8));
    ::memcpy(&Color[6], m_ZColor, 15 * sizeof(juce::uint8));
  }
}

//-----------------------------------------------------------------------------
// Fonction d'estompage d'un pixel à partir de l'altitude du pixel, de l'altitude du pixel juste au dessus,
// de l'altitude du pixel juste à sa droite
//-----------------------------------------------------------------------------
double DtmShader::CoefEstompage(float altC, float altH, float altD, float angleH, float angleV)
{
  double deltaX = m_dGSD, deltaY = m_dGSD; // angleH = 135, angleV = 45;
  if (m_dGSD <= 0.)
    deltaX = deltaY = 25.;
  if ((m_dGSD > 0.) && (m_dGSD < 0.1))    // Donnees en geographiques
    deltaX = deltaY = m_dGSD * 111319.49;  // 1 degre a l'Equateur

  // Recherche de la direction de la normale a la surface du mnt
  double dY[3], dX[3], normale[3];

  // Initialisation des vecteurs de la surface du pixel considéré
  dY[0] = 0;
  dY[1] = deltaY;
  dY[2] = (double)(altH - altC);

  dX[0] = deltaX;
  dX[1] = 0;
  dX[2] = (double)(altD - altC);

  // Direction de la normale a la surface
  normale[0] = -(dY[1] * dX[2]);
  normale[1] = -(dX[0] * dY[2]);
  normale[2] = dX[0] * dY[1];

  // Determination de l'angle entre la normale et la direction de la lumière
  // Modification des teintes en fonction de cet angle
  double correction;
  double DirLum[3];
  
  // Passage d'une représentation angulaire de la direction de la lumiere en représentation cartésienne
  DirLum[0] = cos(XPI / 180 * angleV * -1) * sin(XPI / 180 * angleH);
  DirLum[1] = cos(XPI / 180 * angleV * -1) * cos(XPI / 180 * angleH);
  DirLum[2] = sin(XPI / 180 * angleV * -1);

  // Correction correspond au cosinus entre la normale et la lumiere
  double scalaire;
  scalaire = (normale[0] * DirLum[0]) + (normale[1] * DirLum[1]) + (normale[2] * DirLum[2]);

  double normenormale;
  double normeDirLum;
  normenormale = (normale[0] * normale[0]) + (normale[1] * normale[1]) + (normale[2] * normale[2]);

  normeDirLum = (DirLum[0] * DirLum[0]) + (DirLum[1] * DirLum[1]) + (DirLum[2] * DirLum[2]);
  if (!normenormale == 0 && !normeDirLum == 0)
    correction = scalaire / sqrt(normenormale * normeDirLum);
  else correction = 0;

  // Traduction de cette correction en correction finale
  if (correction >= 0)
    correction = 0;
  else correction = -correction;

  return correction;
}

//-----------------------------------------------------------------------------
// Calcul de la pente
//-----------------------------------------------------------------------------
double DtmShader::CoefPente(float altC, float altH, float altD)
{
  double d = m_dGSD;
  if (d <= 0.) d = 25;
  double costheta = d / sqrt((altD - altC) * (altD - altC) + (altH - altC) * (altH - altC) + d * d);
  return costheta;
}

//-----------------------------------------------------------------------------
// Indique si l'on a une isohypse
//-----------------------------------------------------------------------------
int DtmShader::Isohypse(float altC, float altH, float altD)
{
  double step = m_dZ1 - m_dZ0;
  int nb_isoC = (int)ceil(altC / step);
  int nb_isoH = (int)ceil(altH / step);
  int nb_isoD = (int)ceil(altD / step);
  int nb_iso = round(altC / step);

  if (nb_isoC != nb_isoH)
    return nb_iso;
  if (nb_isoC != nb_isoD)
    return nb_iso;

  return -9999;
}

//-----------------------------------------------------------------------------
// Calcul de l'estompage sur une ligne
//-----------------------------------------------------------------------------
bool DtmShader::EstompLine(float* lineR, float* lineS, float* lineT, juce::uint32 W, juce::uint8* rgb, juce::uint32 num)
{
  float* ptr = lineS;
  double coef = 1., r, g, b;
  float val;
  int index = 0, nb_iso;
  for (juce::uint32 i = 0; i < W; i++) {
    val = *ptr;
    r = g = b = 255;
    if (val <= m_dNoData) { // Hors zone
      ::memcpy(&rgb[3 * i], m_HZColor, 3 * sizeof(juce::uint8));
      ptr++;
      continue;
    }
    if (val <= m_dZ0) { // Sous la mer
      coef = (m_dZ0 - val);
      index = -1;
      r = m_Z0Color[0];
      g = m_Z0Color[1];
      b = m_Z0Color[2];
    }

    if ((val > m_dZ0) && (val < m_dZ1)) {
      coef = (m_dZ1 - val) / (m_dZ1 - m_dZ0);
      index = 0;
      r = m_ZColor[0] * coef + m_ZColor[3] * (1 - coef);
      g = m_ZColor[1] * coef + m_ZColor[4] * (1 - coef);
      b = m_ZColor[2] * coef + m_ZColor[5] * (1 - coef);
    }
    if ((val >= m_dZ1) && (val < m_dZ2)) {
      coef = (m_dZ2 - val) / (m_dZ2 - m_dZ1);
      index = 3;
      r = m_ZColor[3] * coef + m_ZColor[6] * (1 - coef);
      g = m_ZColor[4] * coef + m_ZColor[7] * (1 - coef);
      b = m_ZColor[5] * coef + m_ZColor[8] * (1 - coef);
    }
    if ((val >= m_dZ2) && (val < m_dZ3)) {
      coef = (m_dZ3 - val) / (m_dZ3 - m_dZ2);
      index = 6;
      r = m_ZColor[6] * coef + m_ZColor[9] * (1 - coef);
      g = m_ZColor[7] * coef + m_ZColor[10] * (1 - coef);
      b = m_ZColor[8] * coef + m_ZColor[11] * (1 - coef);
    }
    if ((val >= m_dZ3) && (val < m_dZ4)) {
      coef = (m_dZ4 - val) / (m_dZ4 - m_dZ3);
      index = 9;
      r = m_ZColor[9] * coef + m_ZColor[12] * (1 - coef);
      g = m_ZColor[10] * coef + m_ZColor[13] * (1 - coef);
      b = m_ZColor[11] * coef + m_ZColor[14] * (1 - coef);
    }

    if (val >= m_dZ4) {
      index = 12;
    }

    switch (m_nMode) {
    case 0: coef = 1.;
      break;
    case 1: // Estompage
      if (num == 0) {
        if (i < (W - 1))
          coef = CoefEstompage(lineT[i], val, lineT[i + 1]);
        else
          coef = CoefEstompage(lineS[i - 1], lineR[i - 1], val);
      }
      else {
        if (i < (W - 1))
          coef = CoefEstompage(val, lineR[i], lineS[i + 1]);
        else
          coef = CoefEstompage(lineS[i - 1], lineR[i - 1], val);
      }
      break;
    case 2: // Pente
      if (num == 0) {
        if (i < (W - 1))
          coef = CoefPente(lineT[i], val, lineT[i + 1]);
        else
          coef = CoefPente(lineS[i - 1], lineR[i - 1], val);
      }
      else {
        if (i < (W - 1))
          coef = CoefPente(val, lineR[i], lineS[i + 1]);
        else
          coef = CoefPente(lineS[i - 1], lineR[i - 1], val);
      }
      break;
    case 3: // Aplat de couleurs
      coef = 1.;
      if (index >= 0) {
        r = m_ZColor[index];
        g = m_ZColor[index + 1];
        b = m_ZColor[index + 2];
      }
      break;
    case 4: // Aplat + estompage
      if (num == 0) {
        if (i < (W - 1))
          coef = CoefEstompage(lineT[i], val, lineT[i + 1]);
        else
          coef = CoefEstompage(lineS[i - 1], lineR[i - 1], val);
      }
      else {
        if (i < (W - 1))
          coef = CoefEstompage(val, lineR[i], lineS[i + 1]);
        else
          coef = CoefEstompage(lineS[i - 1], lineR[i - 1], val);
      }
      if (index >= 0) {
        r = m_ZColor[index];
        g = m_ZColor[index + 1];
        b = m_ZColor[index + 2];
      }
      break;
    case 5: // Isohypses
      coef = 1.;
      r = g = b = 255;
      if (num == 0) {
        if (i < (W - 1))
          nb_iso = Isohypse(lineT[i], val, lineT[i + 1]);
        else
          nb_iso = Isohypse(lineS[i - 1], lineR[i - 1], val);
      }
      else {
        if (i < (W - 1))
          nb_iso = Isohypse(val, lineR[i], lineS[i + 1]);
        else
          nb_iso = Isohypse(lineS[i - 1], lineR[i - 1], val);
      }

      if (nb_iso > -9999) {
        double cote = nb_iso * (m_dZ1 - m_dZ0);
        if (cote > m_dZ4) index = 12;
        if (cote <= m_dZ4) index = 9;
        if (cote <= m_dZ3) index = 6;
        if (cote <= m_dZ2) index = 3;
        if (cote <= m_dZ1) index = 0;
        if (cote <= m_dZ0) index = -1;

        if (index >= 0) {
          r = m_ZColor[index];
          g = m_ZColor[index + 1];
          b = m_ZColor[index + 2];
        }
        else {
          r = m_Z0Color[0];
          g = m_Z0Color[1];
          b = m_Z0Color[2];
        }
      }

      break;
    case 6: // Estompage leger
      if (num == 0) {
        if (i < (W - 1))
          coef = CoefEstompage(lineT[i], val, lineT[i + 1], 135., 65.);
        else
          coef = CoefEstompage(lineS[i - 1], lineR[i - 1], val, 135., 65.);
      }
      else {
        if (i < (W - 1))
          coef = CoefEstompage(val, lineR[i], lineS[i + 1], 135., 65.);
        else
          coef = CoefEstompage(lineS[i - 1], lineR[i - 1], val, 135., 65.);
      }
      break;
    }

    rgb[3 * i] = round(r * coef);
    rgb[3 * i + 1] = round(g * coef);
    rgb[3 * i + 2] = round(b * coef);
    ptr++;
  }

  return true;
}


//-----------------------------------------------------------------------------
// Calcul de l'estompage
//-----------------------------------------------------------------------------
bool DtmShader::ConvertArea(float* area, juce::uint32 w, juce::uint32 h, juce::uint8* rgb)
{
  EstompLine(area, area, &area[w], w, rgb, 0);
  for (juce::uint32 i = 1; i < h - 1; i++) {
    EstompLine(&area[(i - 1) * w], &area[i * w], &area[(i + 1) * w], w, &rgb[i * w * 3L], i);
  }
  EstompLine(&area[(h - 2) * w], &area[(h - 1) * w], &area[(h - 1) * w], w, &rgb[(h - 1) * w * 3L], h - 1);
  return true;
}

bool DtmShader::ConvertImage(juce::Image* rawImage, juce::Image* rgbImage)
{
  if ((rawImage->getWidth() != rgbImage->getWidth()) || (rawImage->getHeight() != rgbImage->getHeight()))
    return false;
  if (rawImage->getFormat() != juce::Image::PixelFormat::ARGB)
    return false;
  if (rgbImage->getFormat() != juce::Image::PixelFormat::RGB)
    return false;
  int w = rawImage->getWidth(), h = rawImage->getHeight();

  juce::Image::BitmapData rawData(*rawImage, juce::Image::BitmapData::readWrite);
  juce::Image::BitmapData rgbData(*rgbImage, juce::Image::BitmapData::readWrite);

  EstompLine((float*)rawData.getLinePointer(0), (float*)rawData.getLinePointer(0), (float*)rawData.getLinePointer(1), 
              w, rgbData.getLinePointer(0), 0);
  for (juce::uint32 i = 1; i < h - 1; i++) {
    EstompLine((float*)rawData.getLinePointer(i - 1), (float*)rawData.getLinePointer(i), (float*)rawData.getLinePointer(i + 1), 
                w, rgbData.getLinePointer(i), i);
  }
  EstompLine((float*)rawData.getLinePointer(h - 2), (float*)rawData.getLinePointer(h - 1), (float*)rawData.getLinePointer(h - 1),
                w, rgbData.getLinePointer(h - 1), h - 1);
  return true;

}