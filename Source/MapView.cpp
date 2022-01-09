//==============================================================================
// MapView.cpp
//
// Author : F.Becirspahic
// Date : 10/12/2021
//==============================================================================

#include <JuceHeader.h>
#include "MapView.h"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()

//==============================================================================
MapView::MapView() : m_MapThread("MapThread")
{
  m_Transform = juce::AffineTransform(1.f, 0.f, 0.f, 0.f, -1.f, 0.f);
	m_dX0 = m_dY0 = m_dX = m_dY = 0.;
	m_dScale = 1.0;
	m_bDrag = m_bZoom = m_bSelect = false;
	m_Base = nullptr;
	startTimerHz(60);
}

MapView::~MapView()
{
	m_MapThread.stopThread(1000);
}

void MapView::paint (juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background
	g.setColour(juce::Colours::grey);
	g.drawRect(getLocalBounds(), 1);   // draw an outline around the component

	if ((m_bZoom)||(m_bSelect)) {
		m_MapThread.Draw(g);
		g.drawRect(juce::Rectangle<int>(m_StartPt, m_DragPt+m_StartPt), 1);
		return;
	}
	if (m_bDrag) {
		m_MapThread.Draw(g, m_DragPt.x, m_DragPt.y);
		return;
	}
	if (m_MapThread.NumObjects() < 1) {
		g.drawImageAt(m_Image, 0, 0);
		return;
	}
	m_Image.clear(m_Image.getBounds());
	m_MapThread.Draw(g);

	g.setColour(juce::Colours::white);
	g.setFont(10.0f);
	OGREnvelope env = m_MapThread.Envelope();
	g.drawText(juce::String(env.MaxX, 2) + " ; " + juce::String(env.MaxY, 2), getLocalBounds(), juce::Justification::topRight);
	g.drawText(juce::String(env.MinX, 2) + " ; " + juce::String(env.MinY, 2), getLocalBounds(), juce::Justification::bottomLeft);
	g.drawText(juce::String(m_MapThread.NumObjects()), getLocalBounds(), juce::Justification::centredTop);
	g.drawText(juce::String(m_dX, 2) + " ; " + juce::String(m_dY, 2), getLocalBounds(), juce::Justification::centredBottom);
}

void MapView::resized()
{
	RenderMap();
}

void MapView::mouseDown(const juce::MouseEvent& event)
{
	m_StartPt = event.getPosition();
	setMouseCursor(juce::MouseCursor(juce::MouseCursor::CrosshairCursor));
	if (event.mods.isCtrlDown()) {
		m_bZoom = true;
		return;
	}
	if (event.mods.isShiftDown()) {
		m_bSelect = true;
		return;
	}
	m_bZoom = m_bSelect = false;
	m_bDrag = true;
	setMouseCursor(juce::MouseCursor(juce::MouseCursor::DraggingHandCursor));
}

void MapView::mouseMove(const juce::MouseEvent& event)
{
	m_dX = event.x;
	m_dY = event.y;
	Pixel2Ground(m_dX, m_dY);
}

void MapView::mouseDrag(const juce::MouseEvent& event)
{
	m_DragPt.x = event.getDistanceFromDragStartX();
	m_DragPt.y = event.getDistanceFromDragStartY();
	if ((m_DragPt.x != 0)&&(m_DragPt.y != 0)&&(!m_bZoom)&&(!m_bSelect))
		repaint();
}

void MapView::mouseUp(const juce::MouseEvent& event)
{
	setMouseCursor(juce::MouseCursor(juce::MouseCursor::NormalCursor));
	double X0 = m_StartPt.x, Y0 = m_StartPt.y, X1 = m_StartPt.x + m_DragPt.x, Y1 = m_StartPt.y + m_DragPt.y;
	Pixel2Ground(X0, Y0);
	Pixel2Ground(X1, Y1);

	if ((abs(m_DragPt.x) > 1) || (abs(m_DragPt.y) > 1)) {
		auto b = getLocalBounds();
		if (m_bZoom) {
			m_dScale /= (b.getWidth() / m_DragPt.x + b.getHeight() / m_DragPt.y) * 0.5;
			CenterView((X0 + X1)*0.5, (Y0 + Y1)*0.5);
		}
		if (m_bSelect) {
			SelectFeatures(X0, Y0, X1, Y1);
		}
		if ((!m_bZoom)&&(!m_bSelect)) {
			m_dX0 -= m_DragPt.x * m_dScale;
			m_dY0 += m_DragPt.y * m_dScale;
			m_Image = createComponentSnapshot(b);
			RenderMap();
		}
	}
	else {
		if (event.mods.isShiftDown())
			SelectFeatures(event.getPosition());
	}
	m_bDrag = m_bZoom = m_bSelect = false;
	m_DragPt = juce::Point<int>(0, 0);
}

void MapView::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
	double X = event.getPosition().x, Y = event.getPosition().y;
	Pixel2Ground(X, Y);
	if (wheel.deltaY < 0.)
		m_dScale *= sqrt(2.0);
	else
		m_dScale *= (1. / sqrt(2.0));
	CenterView(X, Y);
}

void MapView::mouseDoubleClick(const juce::MouseEvent& event)
{
	double X = event.getPosition().x, Y = event.getPosition().y;
	Pixel2Ground(X, Y);
	CenterView(X, Y);
}

//==============================================================================
// Lancement du thread de dessin des donnees
//==============================================================================
void MapView::RenderMap()
{
	m_MapThread.signalThreadShouldExit();
	if (m_MapThread.isThreadRunning()) {
		//m_MapThread.waitForThreadToExit(1000);
		if (!m_MapThread.stopThread(-1))
			return;
	}

	auto b = getLocalBounds();
	m_MapThread.SetDimension(b.getWidth(), b.getHeight());
	m_MapThread.SetEnvelope(m_dX0, m_dY0, m_dX0 + b.getWidth() * m_dScale, m_dY0 - b.getHeight() * m_dScale);
	m_MapThread.SetBase(m_Base);
	m_MapThread.SetTransform(m_dX0, m_dY0, m_dScale);

	m_MapThread.startThread();
}

//==============================================================================
// Zoom sur l'emprise des donnees
//==============================================================================
void MapView::ZoomWorld()
{
	if (!m_Frame.IsInit())
		return;
	auto b = getLocalBounds();
	double scaleX = (m_Frame.MaxX - m_Frame.MinX) / b.getWidth();
	double scaleY = (m_Frame.MaxY - m_Frame.MinY) / b.getHeight();
	m_dScale = (scaleX > scaleY) ? scaleX : scaleY;
	CenterView((m_Frame.MaxX + m_Frame.MinX) * 0.5, (m_Frame.MaxY + m_Frame.MinY) * 0.5);
}

//==============================================================================
// Zoom sur une emprise
//==============================================================================
void MapView::ZoomEnvelope(const OGREnvelope& env, double buffer)
{
	auto b = getLocalBounds();
	double scaleX = (env.MaxX - env.MinX + 2 * buffer) / b.getWidth();
	double scaleY = (env.MaxY - env.MinY + 2 * buffer) / b.getHeight();
	m_dScale = (scaleX > scaleY) ? scaleX : scaleY;
	CenterView((env.MaxX + env.MinX) * 0.5, (env.MaxY + env.MinY) * 0.5);
}

//==============================================================================
// Centre la vue sur un point terrain
//==============================================================================
void MapView::CenterView(const double& X, const double& Y)
{
	auto b = getLocalBounds();
	m_dX0 = X - b.getWidth() * 0.5 * m_dScale;
	m_dY0 = Y + b.getHeight() * 0.5 * m_dScale;
	RenderMap();
}

//==============================================================================
// Conversion Pixel -> Terrain
//==============================================================================
void MapView::Pixel2Ground(double& X, double& Y)
{
	X = m_dX0 + X * m_dScale;
	Y = m_dY0 - Y * m_dScale;
}

//==============================================================================
// Conversion Terrain -> Pixel
//==============================================================================
void MapView::Ground2Pixel(double& X, double& Y)
{
	X = (X - m_dX0) / m_dScale;
	Y = (m_dY0 - Y) / m_dScale;
}

//==============================================================================
// Selection des vecteurs se trouvant proche du point P (coordoonees pixel)
//==============================================================================
void MapView::SelectFeatures(juce::Point<int> P)
{
	if (m_Base == nullptr)
		return;
	OGREnvelope env;
	double X = P.x - 1, Y = P.y - 1;
	Pixel2Ground(X, Y);
	env.Merge(X, Y);
	X = P.x + 1; Y = P.y + 1;
	Pixel2Ground(X, Y);
	env.Merge(X, Y);
	m_Base->SelectFeatures(env);
	sendActionMessage("UpdateSelectFeatures");
}

//==============================================================================
// Selection des vecteurs se trouvant dans une enveloppe (coordonnees terrain)
//==============================================================================
void MapView::SelectFeatures(const double& X0, const double& Y0, const double& X1, const double& Y1)
{
	OGREnvelope env;
	env.Merge(X0, Y0);
	env.Merge(X1, Y1);
	m_Base->SelectFeatures(env);
	sendActionMessage("UpdateSelectFeatures");
}
