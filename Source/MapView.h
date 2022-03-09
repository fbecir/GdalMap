//==============================================================================
// MapView.h
//
// Author : F.Becirspahic
// Date : 10/12/2021
//==============================================================================

#pragma once

#include <JuceHeader.h>
#include "ogrsf_frmts.h"

#include "MapThread.h"

class GDALDataset;
class GeoBase;

//==============================================================================
/*
*/
class MapView  : public juce::Component, private juce::Timer, public juce::ActionBroadcaster
{
public:
  MapView();
  ~MapView() override;
  
  void SetFrame(OGREnvelope env);
  void ZoomWorld();
  void ZoomEnvelope(const OGREnvelope& env, double buffer = 0.);
  void CenterView(const double& X, const double& Y);
  void Pixel2Ground(double& X, double& Y);
  void Ground2Pixel(double& X, double& Y);
  void SetBase(GeoBase* base) { m_MapThread.stopThread(-1); m_Base = base; resized(); }
  void StopThread() { m_MapThread.stopThread(-1); m_Image.clear(m_Image.getBounds()); RenderMap(); }
  void RenderMap(bool overlay = true, bool raster = true, bool dtm = true, bool vector = true, bool force_vector = false);
  void SelectFeatures(juce::Point<int>);
  void SelectFeatures(const double& X0, const double& Y0, const double& X1, const double& Y1);
  void DrawDecoration(juce::Graphics&, int deltaX = 0, int deltaY = 0);
  double ComputeCartoScale(double cartoscale = 0.);

  void paint (juce::Graphics&) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent& event) override;
  void mouseMove(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event) override;
  void mouseDrag(const juce::MouseEvent& event) override;
  void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
  void mouseDoubleClick(const juce::MouseEvent& event) override;

private:
  OGREnvelope m_Frame;
  double				m_dX0;
  double				m_dY0;
  double        m_dScale;
  bool					m_bDrag;
  bool          m_bZoom;
  bool          m_bSelect;
  double        m_dX;
  double        m_dY;
  double        m_dZ;
  juce::Point<int>  m_StartPt;
  juce::Point<int>  m_DragPt;
  juce::Image   m_Image;    // Image de la vue
  MapThread     m_MapThread;
  GeoBase*      m_Base;

  void timerCallback() override { repaint();}

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MapView)
};
