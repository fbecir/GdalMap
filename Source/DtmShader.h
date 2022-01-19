//==============================================================================
// DtmShader.h
// Created: 19 Jan 2022 4:41:34pm
// Author:  FBecirspahic
// Estompage / ombrage des MNT (tire de XDtmShader, adapte a JUCE)
//==============================================================================

#pragma once

#include <JuceHeader.h>

class DtmShader {
protected:
  double CoefEstompage(float altC, float altH, float altD, float angleH = 135., float angleV = 45.);
  double CoefPente(float altC, float altH, float altD);
  int Isohypse(float altC, float altH, float altD);

  bool EstompLine(float* lineR, float* lineS, float* lineT, juce::uint32 w, juce::uint8* rgb, juce::uint32 num);

  static double m_dNoData;
  static double m_dZ0;
  static double m_dZ1;
  static double m_dZ2;
  static double m_dZ3;
  static double m_dZ4;
  static int    m_nMode;
  static juce::uint8   m_HZColor[3];
  static juce::uint8   m_Z0Color[3];
  static juce::uint8   m_ZColor[15];
  static double XPI;

  double  m_dGSD;   // Pas terrain du MNT

public:
  DtmShader(double gsd = 25.) { m_dGSD = gsd; }

  static void SetPref32Bits(double nodata, double z0, double z1, double z2,
    double z3, double z4, int mode);
  static void SetCol32Bits(juce::uint8* Color, bool set = true);

  bool ConvertArea(float* area, juce::uint32 w, juce::uint32 h, juce::uint8* rgb);
  bool ConvertImage(juce::Image* rawImage, juce::Image* rgbImage);
};