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
	setOpaque(true);
	startTimerHz(60);
}

MapView::~MapView()
{
	m_MapThread.stopThread(5000);
}

void MapView::paint (juce::Graphics& g)
{
	//g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background
	//g.setColour(juce::Colours::grey);
	//g.drawRect(getLocalBounds(), 1);   // draw an outline around the component

	if ((m_bZoom)||(m_bSelect)) {
		g.drawImageAt(m_Image, 0, 0);
		g.drawRect(juce::Rectangle<int>(m_StartPt, m_DragPt+m_StartPt), 1);
		return;
	}
	if (m_bDrag) {
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
		g.drawImageAt(m_Image, m_DragPt.x, m_DragPt.y);
		//m_MapThread.Draw(g, m_DragPt.x, m_DragPt.y);
		DrawDecoration(g, m_DragPt.x, m_DragPt.y);
		return;
	}
	m_MapThread.Draw(g);
	DrawDecoration(g);
}

void MapView::resized()
{
	auto b = getLocalBounds();
	if (b.isEmpty())
		return;
	m_Image = juce::Image(juce::Image::PixelFormat::ARGB, b.getWidth(), b.getHeight(), true);
	RenderMap();
}

//==============================================================================
// Affichage des coordonnees, de l'emprise, de l'echelle ...
//==============================================================================
void MapView::DrawDecoration(juce::Graphics& g, int deltaX, int deltaY)
{
	// Affichage des coordonnees, de l'emprise, de l'echelle ...
	OGREnvelope env = m_MapThread.Envelope();
	g.setFont(10.0f);
	auto b = getLocalBounds();
	juce::Rectangle<int> R(0, b.getHeight() - 15, b.getWidth(), 15);
	g.setColour(juce::Colours::darkgrey);
	g.setOpacity(0.5);
	g.fillRect(R);
	R.reduce(5, 0);
	g.setColour(juce::Colours::white);
	g.setOpacity(1.);
	g.drawText(juce::String(env.MinX - deltaX*m_dScale, 2) + " ; " + juce::String(env.MinY + deltaY * m_dScale, 2), R, juce::Justification::centredLeft);
	g.drawText(juce::String(m_dX, 2) + " ; " + juce::String(m_dY, 2), R, juce::Justification::centred);
	g.drawText(juce::String("1/") + juce::String(ComputeCartoScale(), 0), R, juce::Justification::centredRight);

	R = juce::Rectangle<int>(0, 0, b.getWidth(), 15);
	g.setColour(juce::Colours::darkgrey);
	g.setOpacity(0.5);
	g.fillRect(R);
	R.reduce(5, 0);
	g.setColour(juce::Colours::white);
	g.setOpacity(1.);
	g.drawText(juce::String(env.MaxX - deltaX * m_dScale, 2) + " ; " + juce::String(env.MaxY + deltaY * m_dScale, 2), R, juce::Justification::centredRight);
	g.drawText(juce::String(m_MapThread.NumObjects()), R, juce::Justification::centred);
}

//==============================================================================
// Lancement du thread de dessin des donnees
//==============================================================================
void MapView::RenderMap(bool raster, bool vector, bool overlay)
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
	m_MapThread.SetUpdate(raster, vector, overlay);

	m_MapThread.startThread();
}

//==============================================================================
// Gestion de la souris
//==============================================================================
void MapView::mouseDown(const juce::MouseEvent& event)
{
	juce::Graphics imaG(m_Image);
	m_MapThread.Draw(imaG);
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
// Echelle cartograpique 1 : cartoscale
//==============================================================================
double MapView::ComputeCartoScale(double cartoscale)
{
	juce::Desktop* desktop = &juce::Desktop::getInstance();
	const juce::Displays* displays = &(desktop->getDisplays());
	if (displays == nullptr)
		return 0.;
	const juce::Displays::Display* display = displays->getPrimaryDisplay();
	if (display == nullptr)
		return 0;
	double factor = (0.0254 / (display->dpi / display->scale));
	if (cartoscale <= 0.)	// On cherche le denominateur de l'echelle cartographique
		return m_dScale / factor;
	// Sinon on fixe l'echelle de la vue a partir de l'echelle cartographique
	m_dScale = cartoscale * factor;
	return 0.;
}

//==============================================================================
// Fixe l'envelope totale de la vue
//==============================================================================
void MapView::SetFrame(OGREnvelope env)
{ 
	auto b = getLocalBounds();
	double X = b.getCentreX(), Y = b.getCentreY();
	if (m_Frame.IsInit())
		Pixel2Ground(X, Y);
	else {
		X = (env.MinX + env.MaxX) * 0.5;
		Y = (env.MinY + env.MaxY) * 0.5;
		ComputeCartoScale(10000);
	}
	m_Frame = env;
	m_dX0 = env.MinX;
	m_dY0 = env.MaxY;
	CenterView(X, Y);
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
	m_Base->SelectFeatures(env, m_MapThread.SpatialRef());
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
	m_Base->SelectFeatures(env, m_MapThread.SpatialRef());
	sendActionMessage("UpdateSelectFeatures");
}
