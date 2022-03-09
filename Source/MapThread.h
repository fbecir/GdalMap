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

  void SetWorld(const double& X0, const double& Y0, const double& scale, const int& W, const int& H, bool force_vector);
  void SetBase(GeoBase* base) { m_Base = base; }
  void SetUpdate(bool overlay, bool raster, bool dtm, bool vector);
  bool NeedUpdate() { return m_bRaster; }

  juce::int64 NumObjects() { return m_nNumObjects; }
  OGREnvelope Envelope() { return m_Env; }
  OGRSpatialReference* SpatialRef() { return &m_SpatialRef; }
  float GetZ(int u, int v);

	virtual void 	run() override;
	bool Draw(juce::Graphics& g, int x0 = 0, int y0 = 0);

private:
  juce::Image m_Raster;
  juce::Image m_Vector;
  juce::Image m_Overlay;
  juce::Image m_Dtm;
  juce::Image m_RawDtm;
  GeoBase*    m_Base;
  double        m_dX0, m_dY0, m_dScale; // Transformation terrain -> pixel
  bool          m_bRaster, m_bVector, m_bOverlay, m_bDtm; // Couches a dessiner
  bool          m_bRasterDone;
  double*       m_Pt;
  int           m_nPtAlloc;
  juce::Path    m_Path;
  bool          m_bFill;        // Indique que le path doit etre rempli
  juce::int64   m_nNumObjects;  // Nombre d'objets affiches dans la vue
  OGREnvelope   m_Env;
  OGRSpatialReference m_SpatialRef;
  juce::Rectangle<int>  m_ClipVector;

  bool AllocPoints(int numPt);
  void SetDimension(const int& w, const int& h);
  void PrepareImages(bool totalUpdate, int dX = 0, int dY = 0);

  void DrawLayer(GeoBase::VectorLayer* layer);
  void DrawGeometry(const OGRGeometry*);
  void DrawPoint(const OGRGeometry*);
  void DrawPolygon(const OGRGeometry*);
  void DrawMultiPolygon(const OGRGeometry*);
  void DrawLineString(const OGRGeometry*);
  void DrawMultiLineString(const OGRGeometry*);
  void DrawMultiPoint(const OGRGeometry*);
  void DrawCurve(const OGRCurve*);
  void DrawLinearRing(const OGRLinearRing*);

  bool DrawLayer(GeoBase::RasterLayer* layer, bool dtm = false);
  bool DrawRaster(GDALDataset* poDataset, float opacity = 1.f);
  bool DrawDtm(GDALDataset* poDataset, float opacity = 1.f);
  bool PrepareRasterDraw(GDALDataset* poDataset, int& U0, int& V0, int& win, int& hin, int& nbBand, 
                                                 int& R0, int& S0, int& wout, int& hout);

  void DrawSelection();
};