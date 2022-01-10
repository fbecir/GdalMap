//==============================================================================
// MapThread.cpp
//
// Author : F.Becirspahic
// Date : 16/12/2021
//==============================================================================

#include "MapThread.h"
#include "GeoBase.h"

MapThread::MapThread(const juce::String& threadName, size_t threadStackSize) : juce::Thread(threadName, threadStackSize) 
{ 
	m_bFill = false;
	m_nPtAlloc = 0;
	m_Pt = nullptr;
	m_Base = nullptr;
	m_nNumObjects = 0;
	m_dX0 = m_dY0 = 0.;
	m_dScale = 1.0;
	m_bRaster = m_bVector = m_bOverlay = true;
	m_SpatialRef.importFromEPSG(3857);
}

MapThread::~MapThread()
{
	if (m_Pt != nullptr)
		delete[] m_Pt;
}

bool MapThread::AllocPoints(int numPt)
{
	m_Path.preallocateSpace(3 * numPt + 1);
	if (numPt < m_nPtAlloc)
		return true;
	if (m_Pt != nullptr)
		delete[] m_Pt;
	m_Pt = new double[numPt * 2];
	if (m_Pt == nullptr)
		return false;
	m_nPtAlloc = numPt;
	return true;
}

void MapThread::SetDimension(int w, int h)
{
	if ((w != m_Vector.getWidth()) || (h != m_Vector.getHeight())) {
		m_Vector = juce::Image(juce::Image::PixelFormat::ARGB, w, h, true);
		m_Raster = juce::Image(juce::Image::PixelFormat::ARGB, w, h, true);
		m_Overlay = juce::Image(juce::Image::PixelFormat::ARGB, w, h, true);
	}
}

void MapThread::SetUpdate(bool raster, bool vector, bool overlay)
{
	m_bRaster = raster;
	m_bVector = vector;
	m_bOverlay = overlay;
	if (raster)
		m_Raster.clear(m_Raster.getBounds());
	if (vector)
		m_Vector.clear(m_Vector.getBounds());
	if (overlay)
		m_Overlay.clear(m_Overlay.getBounds());
}

void MapThread::SetEnvelope(const double& x0, const double& y0, const double& x1, const double& y1)
{
	m_Env = OGREnvelope();
	m_Env.Merge(x0, y0);
	m_Env.Merge(x1, y1);
}

void MapThread::run()
{
	m_nNumObjects = 0;
	if (m_Base == nullptr)
		return;
	// Affichage des couches raster
	if (m_bRaster) {
		for (int i = 0; i < m_Base->GetRasterLayerCount(); i++) {
			GeoBase::RasterLayer* poLayer = m_Base->GetRasterLayer(i);
			if (poLayer == nullptr)
				continue;
			DrawLayer(poLayer);
		}
	}
	// Affichage des couches vectorielles
	if (m_bVector) {
		for (int i = 0; i < m_Base->GetVectorLayerCount(); i++) {
			GeoBase::VectorLayer* poLayer = m_Base->GetVectorLayer(i);
			if (poLayer == nullptr)
				continue;
			poLayer->SetSpatialFilterRect(m_Env, &m_SpatialRef);
			DrawLayer(poLayer);
		}
	}
	// Affichage de la selection
	if (m_bOverlay)
		DrawSelection();
}

void MapThread::Draw(juce::Graphics& g, int x0, int y0)
{
	//g.setOpacity(1.f);
	g.drawImageAt(m_Raster, x0, y0);
	g.drawImageAt(m_Vector, x0, y0);
	g.drawImageAt(m_Overlay, x0, y0);
}

//==============================================================================
// Dessin des layers vectoriels
//==============================================================================
void MapThread::DrawLayer(GeoBase::VectorLayer* poLayer)
{
	poLayer->ResetReading();
	OGREnvelope env;
	OGRCoordinateTransformation* poTransfo = OGRCreateCoordinateTransformation(poLayer->SpatialRef(), &m_SpatialRef);
	if (poTransfo == nullptr)
		return;
	do {
		const juce::MessageManagerLock mml(Thread::getCurrentThread());
		if (!mml.lockWasGained())  // if something is trying to kill this job, the lock
			return;
		juce::Graphics g(m_Vector);

		for (int i = 0; i < 100; i++) {
			if (threadShouldExit())
				return;
			OGRFeature* poFeature = poLayer->GetNextFeature();
			if (poFeature == nullptr) {
				delete poTransfo;
				return;
			}

			m_Path.clear();
			m_bFill = false;
			g.setColour(juce::Colour(poLayer->m_Repres.PenColor));
			OGRGeometry* poGeom = poFeature->GetGeometryRef();
			poGeom->transform(poTransfo);
			poGeom->getEnvelope(&env);
			if ((fabs(env.MaxX - env.MinX)/m_dScale < 2) && (fabs(env.MaxX - env.MinX)/m_dScale < 2) && (poGeom->getDimension() > 0)) {
				g.drawLine((env.MinX-m_dX0)/m_dScale, (m_dY0-env.MinY)/m_dScale, (env.MaxX-m_dX0)/m_dScale, (m_dY0-env.MaxY)/m_dScale, 3.);
			}
			else {
				DrawGeometry(poGeom);
				g.strokePath(m_Path, juce::PathStrokeType(poLayer->m_Repres.PenSize));
				if (m_bFill) {
					g.setFillType(juce::FillType(juce::Colour(poLayer->m_Repres.FillColor)));
					g.fillPath(m_Path);
				}
			}

			OGRFeature::DestroyFeature(poFeature);
			m_nNumObjects++;
		}
	} while (!threadShouldExit());
	delete poTransfo;
}

//==============================================================================
// Dessin des geometries OGR
//==============================================================================
void MapThread::DrawGeometry(const OGRGeometry* poGeom)
{
	if (wkbFlatten(poGeom->getGeometryType()) == wkbPoint)
		return DrawPoint(poGeom);
	if (wkbFlatten(poGeom->getGeometryType()) == wkbPolygon)
		DrawPolygon(poGeom);
	if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiPolygon)
		DrawMultiPolygon(poGeom);
	if (wkbFlatten(poGeom->getGeometryType()) == wkbLineString)
		DrawLineString(poGeom);
	if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiLineString)
		DrawMultiLineString(poGeom);
	if (wkbFlatten(poGeom->getGeometryType()) == wkbMultiPoint)
		DrawMultiPoint(poGeom);
}

void MapThread::DrawPoint(const OGRGeometry* poGeom)
{
	if (wkbFlatten(poGeom->getGeometryType()) != wkbPoint)
		return;
	const OGRPoint* poPoint = poGeom->toPoint();
	double X = (poPoint->getX() - m_dX0) / m_dScale, Y = (m_dY0 - poPoint->getY()) / m_dScale;
	double d = 3;
	m_Path.startNewSubPath(X, Y);
	m_Path.addEllipse(X - d, Y - d, 2 * d, 2 * d);
}

void MapThread::DrawPolygon(const OGRGeometry* poGeom)
{
	if (wkbFlatten(poGeom->getGeometryType()) != wkbPolygon)
		return;
	m_bFill = true;
	const OGRPolygon* poPolygon = poGeom->toPolygon();
	const OGRCurve* poCurve = poPolygon->getExteriorRingCurve();
	DrawCurve(poCurve);
	for (int i = 0; i < poPolygon->getNumInteriorRings(); i++) {
		poCurve = poPolygon->getInteriorRingCurve(i);
		DrawCurve(poCurve);
	}
}
void MapThread::DrawMultiPolygon(const OGRGeometry* poGeom)
{
	if (wkbFlatten(poGeom->getGeometryType()) != wkbMultiPolygon)
		return;
	m_bFill = true;
	const OGRMultiPolygon* poMPolygon = poGeom->toMultiPolygon();
	for (int i = 0; i < poMPolygon->getNumGeometries(); i++)
		DrawPolygon(poMPolygon->getGeometryRef(i));
}

void MapThread::DrawLineString(const OGRGeometry* poGeom)
{
	if (wkbFlatten(poGeom->getGeometryType()) != wkbLineString)
		return;
	const OGRLineString* poLine = poGeom->toLineString();
	if (!AllocPoints(poLine->getNumPoints()))
		return;
	poLine->getPoints(m_Pt, 2 * sizeof(double), &m_Pt[1], 2 * sizeof(double));
	m_Path.startNewSubPath((m_Pt[0] - m_dX0)/m_dScale, (m_dY0 - m_Pt[1])/m_dScale);
	for (int i = 1; i < poLine->getNumPoints(); i++)
		m_Path.lineTo((m_Pt[2 * i] - m_dX0)/m_dScale, (m_dY0 - m_Pt[2 * i + 1])/m_dScale);
}

void MapThread::DrawCurve(const OGRCurve* poCurve)
{
	if (wkbFlatten(poCurve->getGeometryType()) == wkbLineString)
		return DrawLineString(poCurve);
	if (strcmp(poCurve->getGeometryName(), "LINEARRING") == 0) {
		const OGRLinearRing* poRing = poCurve->toLinearRing();
		return DrawLinearRing(poRing);
	}
}

void MapThread::DrawLinearRing(const OGRLinearRing* poRing)
{
	for (int i = 0; i < poRing->getNumPoints(); i++);

	if (!AllocPoints(poRing->getNumPoints()))
		return;
	poRing->getPoints(m_Pt, 2 * sizeof(double), &m_Pt[1], 2 * sizeof(double));
	m_Path.startNewSubPath((m_Pt[0] - m_dX0) / m_dScale, (m_dY0 - m_Pt[1]) / m_dScale);
	for (int i = 1; i < poRing->getNumPoints(); i++)
		m_Path.lineTo((m_Pt[2 * i] - m_dX0) / m_dScale, (m_dY0 - m_Pt[2 * i + 1]) / m_dScale);
	m_Path.closeSubPath();
}

void MapThread::DrawMultiLineString(const OGRGeometry* poGeom)
{
	if (wkbFlatten(poGeom->getGeometryType()) != wkbMultiLineString)
		return;
	const OGRMultiLineString* poMLine = poGeom->toMultiLineString();
	for (int i = 0; i < poMLine->getNumGeometries(); i++)
		DrawLineString(poMLine->getGeometryRef(i));
}

void MapThread::DrawMultiPoint(const OGRGeometry* poGeom)
{
	if (wkbFlatten(poGeom->getGeometryType()) != wkbMultiPoint)
		return;
	const OGRMultiPoint* poMPoint = poGeom->toMultiPoint();
	for (int i = 0; i < poMPoint->getNumGeometries(); i++)
		DrawPoint(poMPoint->getGeometryRef(i));
}

//==============================================================================
// Dessin de la selection
//==============================================================================
void MapThread::DrawSelection()
{
	const juce::MessageManagerLock mml(Thread::getCurrentThread());
	if (!mml.lockWasGained())
		return;
	juce::Graphics g(m_Overlay);
	g.setColour(juce::Colours::black);
	OGREnvelope env;
	for (size_t i = 0; i < m_Base->GetSelectionCount(); i++) {
		m_Path.clear();
		GeoBase::Feature feature = m_Base->GetSelection(i);
		OGRLayer* poLayer = m_Base->GetOGRLayer(feature.IdLayer());
		if (poLayer == nullptr)
			continue;
		OGRFeature* poFeature = poLayer->GetFeature(feature.Id());
		if (poFeature == nullptr)
			continue;	
		OGRCoordinateTransformation* poTransfo = OGRCreateCoordinateTransformation(poLayer->GetSpatialRef(), &m_SpatialRef);
		if (poTransfo == nullptr) {
			OGRFeature::DestroyFeature(poFeature);
			continue;
		}
		OGRGeometry* poGeom = poFeature->GetGeometryRef();
		poGeom->transform(poTransfo);
		poGeom->getEnvelope(&env);
		if (m_Env.Intersects(env)) {
			DrawGeometry(poGeom);
			juce::Path::Iterator iter(m_Path);
			int numPoint = 0;
			bool needText = true;
			if ((fabs(env.MaxX - env.MinX) / m_dScale < 100) && (fabs(env.MaxX - env.MinX) / m_dScale < 100) && (poGeom->getDimension() > 0))
				needText = false;
			while (iter.next()) {
				g.drawRect(iter.x1 - 2, iter.y1 - 2, 4.f, 4.f);
				if (needText)
					g.drawSingleLineText(juce::String(numPoint), iter.x1 + 5, iter.y1);
				numPoint++;
			}
		}
		delete poTransfo;
		OGRFeature::DestroyFeature(poFeature);
		if (threadShouldExit())
			return;
	}
}

//==============================================================================
// Dessin des datasets raster
//==============================================================================
void MapThread::DrawLayer(GeoBase::RasterLayer* layer)
{
	if (!m_Env.Intersects(layer->Envelope()))
		return ;
	for (int i = 0; i < layer->GetRasterCount(); i++) {
		if (m_Env.Intersects(layer->GetRasterEnvelope(i)))
			DrawRaster(layer->GetRasterDataset(i), layer->Opacity());
		if (threadShouldExit())
			return;
	}
}

void MapThread::DrawRaster(GDALDataset* poDataset, float opacity)
{
	if (poDataset == nullptr)
		return;
	int nbBand = poDataset->GetRasterCount();
	if (nbBand > 3) nbBand = 3;
	// Pour l'instant, on ne gere pas les rotations et les facteurs d'echelle differents
	int W = poDataset->GetRasterXSize();
	int H = poDataset->GetRasterYSize();
	double transfo[6];
	poDataset->GetGeoTransform(transfo);
	double X0 = transfo[0];
	double Y0 = transfo[3];
	double gsd = transfo[1];
	if (Y0 < 1) Y0 = H;
	// Zone pixel dans l'image
	int U0 = (int)round((m_Env.MinX - X0) / gsd);
	int V0 = (int)round((Y0 - m_Env.MaxY) / gsd);
	int U1 = (int)round((m_Env.MaxX - X0) / gsd);
	int V1 = (int)round((Y0 - m_Env.MinY) / gsd);
	if (U0 < 0) U0 = 0;
	if (V0 < 0) V0 = 0;
	if (U1 > W) U1 = W;
	if (V1 > H) V1 = H;
	// Zone pixel dans le bitmap resultat
	double gsdR = (m_Env.MaxX - m_Env.MinX) / m_Raster.getWidth();
	int R0 = (int)round(((U0 * gsd + X0) - m_Env.MinX) / gsdR);
	int S0 = (int)round((m_Env.MaxY - (Y0 - V0 * gsd)) / gsdR);
	int R1 = (int)round(((U1 * gsd + X0) - m_Env.MinX) / gsdR);
	int S1 = (int)round((m_Env.MaxY - (Y0 - V1 * gsd)) / gsdR);
	//
	juce::Image::PixelFormat format = juce::Image::PixelFormat::RGB;
	if (nbBand == 1)
		format = juce::Image::PixelFormat::SingleChannel;
	juce::Image tmpImage(format, (R1-R0), (S1-S0), true);
	juce::Image::BitmapData bitmap(tmpImage, juce::Image::BitmapData::readWrite);
	for (int i = 0; i < nbBand; ++i) {
		// Fetch the band
		GDALRasterBand* band = poDataset->GetRasterBand(i + 1); // Bandes numerotees de 1 à N
		// Read the data
		CPLErr error = band->RasterIO(GF_Read, U0, V0, (U1-U0), (V1-V0), &bitmap.data[nbBand - 1 - i], (R1 - R0), (S1 - S0), GDT_Byte,
			bitmap.pixelStride, bitmap.lineStride);
		if (error == CE_Failure)
			return ;
	}
	juce::Graphics g(m_Raster);
	g.setOpacity(opacity);
	g.drawImageAt(tmpImage, R0, S0);
	m_nNumObjects++;
}