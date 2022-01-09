//==============================================================================
// MapThread.h
//
// Author : F.Becirspahic
// Date : 16/12/2021
//==============================================================================

#pragma once

#include <JuceHeader.h>
#include "ogrsf_frmts.h"
#include "GeoBase.h"

class GDALDataset;

class MapThread : public juce::Thread {
public:
  MapThread(const juce::String& threadName, size_t threadStackSize = 0);
  virtual ~MapThread();

	void SetDimension(int w, int h);
  void SetEnvelope(const double& x0, const double& y0, const double& x1, const double& y1);
  void SetTransform(const double& X0, const double& Y0, const double& scale) { m_dX0 = X0; m_dY0 = Y0; m_dScale = scale; }
  void SetBase(GeoBase* base) { m_Base = base; }

  juce::int64 NumObjects() { return m_nNumObjects; }
  OGREnvelope Envelope() { return m_Env; }

	virtual void 	run() override;
	void Draw(juce::Graphics& g, int x0 = 0, int y0 = 0);

private:
  juce::Image m_Image;
  GeoBase*    m_Base;
  double        m_dX0, m_dY0, m_dScale; // Transformation terrain -> pixel
  double*       m_Pt;
  int           m_nPtAlloc;
  juce::Path    m_Path;
  bool          m_bFill;        // Indique que le path doit etre rempli
  juce::int64   m_nNumObjects;  // Nombre d'objets affiches dans la vue
  OGREnvelope   m_Env;

  bool AllocPoints(int numPt);

  void DrawLayer(GeoBase::VectorLayer* layer);
  void DrawGeometry(juce::Graphics&, const OGRGeometry*);
  void DrawPoint(const OGRGeometry*);
  void DrawPolygon(const OGRGeometry*);
  void DrawMultiPolygon(const OGRGeometry*);
  void DrawLineString(const OGRGeometry*);
  void DrawMultiLineString(const OGRGeometry*);
  void DrawMultiPoint(const OGRGeometry*);
  void DrawCurve(const OGRCurve*);
  void DrawLinearRing(const OGRLinearRing*);

  void DrawLayer(GeoBase::RasterLayer* layer);
  void DrawRaster(GDALDataset* poDataset);
};