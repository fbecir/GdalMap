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

  bool EstompLine(float* lineR, float* lineS, float* lineT, juce::uint32 w, juce::uint8* rgba, juce::uint32 num);

  static double XPI;
  double  m_dGSD;   // Pas terrain du MNT

public:
  DtmShader(double gsd = 25.);
  bool ConvertImage(juce::Image* rawImage, juce::Image* rgbImage);

  enum class ShaderMode { Altitude = 0, Shading, Light_Shading, Free_Shading, Slope, Colour, Shading_Colour, Contour};

  static std::vector<double> m_Z;     // Plages d'altitude
  static std::vector<juce::Colour> m_Colour;  // Plages de couleur
  static ShaderMode    m_Mode;  // Mode d'affichage
  static double m_dIsoStep; // Pas des isohypses
  static double m_dSolarAzimuth;  // Angle azimuthal en degres
  static double m_dSolarZenith;   // Angle zenithal en degres

  static bool AddAltitude(double z);
};