//==============================================================================
// GeoBase.cpp
//
// Author : F.Becirspahic
// Date : 14/12/2021
//==============================================================================

#include "GeoBase.h"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "cpl_string.h"

//==============================================================================
// Constructeur GeoBase
//==============================================================================
GeoBase::GeoBase()
{
	m_SpatialRef.importFromEPSG(3857);
}

//==============================================================================
// Retire les jeux de donnees charges
//==============================================================================
void GeoBase::Clear()
{
	m_Env = OGREnvelope();
	m_Selection.clear();
	m_Field.clear();
	for (int i = 0; i < m_VLayers.size(); i++)
		delete m_VLayers[i];
	m_VLayers.clear();
	for (int i = 0; i < m_RLayers.size(); i++)
		delete m_RLayers[i];
	m_RLayers.clear();
	for (int i = 0; i < m_ZLayers.size(); i++)
		delete m_ZLayers[i];
	m_ZLayers.clear();
	for (int i = 0; i < m_Dataset.size(); i++)
		m_Dataset[i]->Release();
	m_Dataset.clear();
}

//==============================================================================
// Ouverture d'un dataset vectoriel
//==============================================================================
bool GeoBase::OpenVectorDataset(const char* filename, char** options)
{
	GDALDataset* poDataset = GDALDataset::Open(filename, GDAL_OF_VECTOR | GDAL_OF_READONLY, nullptr, options);
	if (poDataset == NULL) 
		return false;

	// Analyse des layers
	OGREnvelope total;
	for (int i = 0; i < poDataset->GetLayerCount(); i++) {
		if (poDataset->IsLayerPrivate(i))
			continue;
		VectorLayer* layer = new VectorLayer(GetVectorLayerCount()+1);
		if (layer == nullptr)
			continue;
		if (!layer->SetDataset(poDataset, i)) {
			delete layer;
			continue;
		}
		m_VLayers.push_back(layer);
		total = layer->Envelope();
		m_Env.Merge(ConvertEnvelop(total, layer->SpatialRef(), &m_SpatialRef));
	}
	m_Dataset.push_back(poDataset);

	return true;
}

//==============================================================================
// Ouverture d'un dataset raster
//==============================================================================
bool GeoBase::OpenRasterDataset(const char* filename, const char* name, bool visible, char** options, bool dtm)
{
	GDALDataset* poDataset = GDALDataset::Open(filename, GDAL_OF_RASTER | GDAL_OF_READONLY, nullptr, options);
	if (poDataset == NULL)
		return false;

	RasterLayer* layer = new RasterLayer;
	if (layer->AddDataset(poDataset)) {
		if (name != nullptr)
			layer->Name(name);
		layer->Visible = visible;
		if (dtm)
			m_ZLayers.push_back(layer);
		else
			m_RLayers.push_back(layer);
		m_Dataset.push_back(poDataset);
		m_Env.Merge(layer->Envelope());
		return true;
	}
	delete layer;
	poDataset->Release();
	return false;
}

//==============================================================================
// Ouverture d'un multi-dataset raster (WMTS par exemple)
//==============================================================================
bool GeoBase::OpenRasterMultiDataset(const char* filename)
{
	GDALDataset* poDataset = GDALDataset::Open(filename, GDAL_OF_RASTER | GDAL_OF_READONLY);
	if (poDataset == NULL)
		return false;

	char** papszSubdatasets = poDataset->GetMetadata("SUBDATASETS");
	// The returned string list is owned by the object, and may change at any time

	CPLStringList options;
	//options.AddNameValue("EXTENDBEYONDDATELINE", "YES");
	//options.AddNameValue("EXTENT_METHOD", "MOST_PRECISE_TILE_MATRIX");
	options.AddNameValue("CLIP_EXTENT_WITH_MOST_PRECISE_TILE_MATRIX ", "NO");
	options.AddNameValue("CLIP_EXTENT_WITH_MOST_PRECISE_TILE_MATRIX_LIMITS", "NO");
	//options.AddNameValue("ZOOM_LEVEL", "19");

	for (int i = 0; i < CSLCount(papszSubdatasets); i++) {
		char szKeyName[1024];
		char* pszSubdatasetName;
		char* pszSubdatasetDesc;
		snprintf(szKeyName, sizeof(szKeyName),"SUBDATASET_%d_NAME", i+1);
		szKeyName[sizeof(szKeyName) - 1] = '\0';
		pszSubdatasetName = CPLStrdup(CSLFetchNameValue(papszSubdatasets, szKeyName));
		snprintf(szKeyName, sizeof(szKeyName), "SUBDATASET_%d_DESC", i+1);
		szKeyName[sizeof(szKeyName) - 1] = '\0';
		pszSubdatasetDesc = CPLStrdup(CSLFetchNameValue(papszSubdatasets, szKeyName));
		if (strlen(pszSubdatasetName) > 0)
			OpenRasterDataset(pszSubdatasetName, pszSubdatasetDesc, false, options.List());
		CPLFree(pszSubdatasetName);
		CPLFree(pszSubdatasetDesc);
	}
	poDataset->Release();
	return true;
}

//==============================================================================
// Indique si un fichier est deja ouvert dans les datasets
//==============================================================================
bool GeoBase::IsOpen(const char* filename)
{
	for (size_t i = 0; i < m_Dataset.size(); i++) {
		GDALDataset* poDataset = m_Dataset[i];
		char** fileList = poDataset->GetFileList();
		if (fileList == nullptr)
			continue;
		if (CSLFindString(fileList, filename) != -1) {
			CSLDestroy(fileList);
			return true;
		}
		CSLDestroy(fileList);
	}
	return false;
}

//==============================================================================
// Selection des features dans une enveloppe
//==============================================================================
size_t GeoBase::SelectFeatures(const OGREnvelope& env, OGRSpatialReference* spatialRef)
{
	m_Selection.clear();
	if (!OGRGeometryFactory::haveGEOS())
		return 0;
	for (int layerId = 0; layerId < GetVectorLayerCount(); layerId++) {
		VectorLayer* poLayer = GetVectorLayer(layerId);
		if (poLayer == nullptr)
			continue;
		if (!poLayer->m_Repres.Visible)
			continue;
		poLayer->SetSpatialFilterRect(env, spatialRef);
		poLayer->ResetReading();
		OGRLinearRing ring;
		OGRPolygon poly;
		ring.addPoint(env.MinX, env.MinY);
		ring.addPoint(env.MaxX, env.MinY);
		ring.addPoint(env.MaxX, env.MaxY);
		ring.addPoint(env.MinX, env.MaxY);
		ring.addPoint(env.MinX, env.MinY);
		ring.closeRings();
		poly.addRing(&ring);
		if (!poly.IsValid())
			continue;
		OGRCoordinateTransformation* poTransfo = OGRCreateCoordinateTransformation(spatialRef, poLayer->SpatialRef());
		if (poTransfo == nullptr)
			continue;
		poly.transform(poTransfo);

		OGREnvelope featureEnv;
		GIntBig featureId;
		do {
			OGRFeature* poFeature = poLayer->GetNextFeature();
			if (poFeature == nullptr)
				break;
			OGRGeometry* poGeom = poFeature->GetGeometryRef();
			if (poGeom->Intersects(&poly) == TRUE) {
				poGeom->getEnvelope(&featureEnv);
				featureId = poFeature->GetFID();
				if (featureId != OGRNullFID)
					m_Selection.push_back(Feature(poFeature->GetFID(), featureEnv, poLayer->Id()));
			}
			OGRFeature::DestroyFeature(poFeature);
		} while (true);
		delete poTransfo;
	}
	return m_Selection.size();
}

//==============================================================================
// Selection des champs d'un feature
//==============================================================================
bool GeoBase::SelectFeatureFields(int layerId, GIntBig featureId)
{
	m_Field.clear();
	OGRLayer* poLayer = GetOGRLayer(layerId);
	if (poLayer == nullptr)
		return false;
	OGRFeature* poFeature = poLayer->GetFeature(featureId);
	if (poFeature == nullptr)
		return false;
	OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
	for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++) {
		OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn(iField);
		m_Field.push_back(poFieldDefn->GetNameRef());
		m_Field.push_back(poFeature->GetFieldAsString(iField));
	}
	OGRFeature::DestroyFeature(poFeature);
	return true;
}

//==============================================================================
// Change l'ordre des layers
//==============================================================================
bool GeoBase::ReorderVectorLayer(int oldPosition, int newPosition)
{ 
	return ReorderLayer<VectorLayer>(&m_VLayers, oldPosition, newPosition); 
}
bool GeoBase::ReorderRasterLayer(int oldPosition, int newPosition)
{ 
	return ReorderLayer<RasterLayer>(&m_RLayers, oldPosition, newPosition);
}
bool GeoBase::ReorderDtmLayer(int oldPosition, int newPosition)
{ 
	return ReorderLayer<RasterLayer>(&m_ZLayers, oldPosition, newPosition);
}

//==============================================================================
// Change l'ordre dans une liste de layers
//==============================================================================
template<typename T> static bool GeoBase::ReorderLayer(std::vector<T*>* V, int oldPosition, int newPosition)
{
	if (oldPosition >= V->size())
		return false;
	T* layer = (*V)[oldPosition];
	V->erase(V->begin() + oldPosition);
	if (newPosition < 0) {
		V->push_back(layer);
		return true;
	}
	if (newPosition < oldPosition) {
		V->insert(V->begin() + newPosition, layer);
		return true;
	}
	if (newPosition <= V->size()) {
		V->insert(V->begin() + newPosition - 1, layer);
		return true;
	}
	V->push_back(layer);
	return true;
}

//==============================================================================
// Changement de systeme de reference d'une envelope
//==============================================================================
OGREnvelope GeoBase::ConvertEnvelop(const OGREnvelope& env, OGRSpatialReference* fromRef, OGRSpatialReference* toRef)
{
	if ((fromRef == nullptr) || (toRef == nullptr))
		return env;
	OGRCoordinateTransformation* poTransfo = OGRCreateCoordinateTransformation(fromRef, toRef);
	if (poTransfo == nullptr)
		return env;
	OGREnvelope result;
	double X = env.MinX, Y = env.MinY;	// Bottom Left
	poTransfo->Transform(1, &X, &Y);		
	result.Merge(X, Y);									
	X = env.MinX; Y = env.MaxY;					// Top Left
	poTransfo->Transform(1, &X, &Y);		
	result.Merge(X, Y);
	X = env.MaxX; Y = env.MaxY;					// Top Right
	poTransfo->Transform(1, &X, &Y);		
	result.Merge(X, Y);
	X = env.MaxX; Y = env.MinY;					// Bottom Right
	poTransfo->Transform(1, &X, &Y);		
	result.Merge(X, Y);
	delete poTransfo;
	return result;
}

//==============================================================================
// Constructeur GeoLayer
//==============================================================================
GeoBase::VectorLayer::VectorLayer(int id)
{
	m_Dataset = nullptr;
	m_OGRLayer = nullptr; 
	m_Id = id;
	m_bFastSpatialFilter = false;
	m_nIndex = 0;
	m_Repres.PenColor = 0xFF008800;
	m_Repres.FillColor = 0x55770000;
	m_Repres.PenSize = 2.;
	m_Repres.Visible = true;
}

//==============================================================================
// Ouverture d'un dataset vectoriel
//==============================================================================
bool GeoBase::VectorLayer::SetDataset(GDALDataset* poDataset, int id)
{
	m_OGRLayer = poDataset->GetLayer(id);
	if (m_OGRLayer == nullptr)
		return false;
	if ( (m_OGRLayer->TestCapability(OLCFastSpatialFilter)) || 
		(m_OGRLayer->TestCapability(OLCTransactions)) || (m_OGRLayer->TestCapability(OLCFastGetExtent)) ) {
		if (m_OGRLayer->TestCapability(OLCTransactions)) {
			std::string geom = m_OGRLayer->GetGeometryColumn();
			if (geom.size() < 1) // Layer non geometrique
				return false;
		}
		m_bFastSpatialFilter = true;
		m_OGRLayer->GetExtent(&m_Env);
		return true;
	}
	
	OGREnvelope env;
	m_OGRLayer->ResetReading();
	do {
		OGRFeature* poFeature = m_OGRLayer->GetNextFeature();
		if (poFeature == nullptr)
			break;
		const OGRGeometry* poGeom = poFeature->GetGeometryRef();
		poGeom->getEnvelope(&env);
		m_T.push_back(Feature(poFeature->GetFID(), env, id));
		OGRFeature::DestroyFeature(poFeature);
		m_Env.Merge(env);
	} while (true);
	if (m_T.size() < 1)
		return false;
	m_FilterRect = m_Env;
	return true;
}

//==============================================================================
// Fixe l'enveloppe pour la recherche de feature
//==============================================================================
void GeoBase::VectorLayer::SetSpatialFilterRect(const OGREnvelope& env, OGRSpatialReference* spatialRef)
{
	if (m_OGRLayer == nullptr)
		return;
	m_FilterRect = GeoBase::ConvertEnvelop(env, spatialRef, m_OGRLayer->GetSpatialRef());
	m_OGRLayer->SetSpatialFilterRect(m_FilterRect.MinX, m_FilterRect.MinY, m_FilterRect.MaxX, m_FilterRect.MaxY);
	m_nIndex = 0;
}

//==============================================================================
// Renvoie le prochain feature
//==============================================================================
OGRFeature* GeoBase::VectorLayer::GetNextFeature()
{
	if (m_OGRLayer == nullptr)
		return nullptr;
	if (m_bFastSpatialFilter)
		return m_OGRLayer->GetNextFeature();
	for (size_t i = m_nIndex; i < m_T.size(); i++) {
		if (!m_T[i].Intersect(m_FilterRect))
			continue;
		m_nIndex = i + 1;
		return m_OGRLayer->GetFeature(m_T[i].Id());
	}
	return nullptr;
}

//==============================================================================
// Recherche de l'ID du prochain feature
//==============================================================================
bool GeoBase::VectorLayer::GetNextFeatureId(GIntBig& id, OGREnvelope& env)
{
	if (m_OGRLayer == nullptr)
		return false;
	if (m_bFastSpatialFilter) {
		OGRFeature* poFeature = m_OGRLayer->GetNextFeature();
		if (poFeature == nullptr)
			return false;
		id = poFeature->GetFID();
		const OGRGeometry* poGeom = poFeature->GetGeometryRef();
		poGeom->getEnvelope(&env);
		OGRFeature::DestroyFeature(poFeature);
		return true;
	}
	for (size_t i = m_nIndex; i < m_T.size(); i++) {
		if (!m_T[i].Intersect(m_FilterRect))
			continue;
		m_nIndex = i + 1;
		id = m_T[i].Id();
		env = m_T[i].Envelope();
		return true;
	}
	return false;
}

//==============================================================================
// Ajout d'un dataset raster
//==============================================================================
bool GeoBase::RasterLayer::AddDataset(GDALDataset* poDataset)
{
	Raster raster;
	if (!raster.AddDataset(poDataset))
		return false;
	m_Raster.push_back(raster);
	m_TotalEnv.Merge(raster.Envelope());
	return true;
}

double GeoBase::RasterLayer::GSD()
{
	double gsd = std::numeric_limits<double>::max();
	for (size_t i = 0; i < m_Raster.size(); i++)
		if (m_Raster[i].GSD() < gsd)
			gsd = m_Raster[i].GSD();
	return gsd;
}

//==============================================================================
// Ajout d'un dataset raster
//==============================================================================
bool GeoBase::Raster::AddDataset(GDALDataset* poDataset)
{
	double transfo[6];
	poDataset->GetGeoTransform(transfo);
	if (fabs(transfo[1] * transfo[5] - transfo[2] * transfo[4]) < 0.000001)
		return false;	// Transformation non affine
	m_GSD = transfo[1];
	int W = poDataset->GetRasterXSize();
	int H = poDataset->GetRasterYSize();
	double X0 = transfo[0];
	double Y0 = transfo[3];
	double X1 = transfo[0] + W * transfo[1];
	double Y1 = transfo[3] + W * transfo[4];
	double X2 = transfo[0] + W * transfo[1] + H * transfo[2];
	double Y2 = transfo[3] + W * transfo[4] + H * transfo[5];
	double X3 = transfo[0] + H * transfo[2];
	double Y3 = transfo[3] + H * transfo[5];
	m_Env = OGREnvelope();
	m_Env.Merge(X0, Y0);
	m_Env.Merge(X1, Y1);
	m_Env.Merge(X2, Y2);
	m_Env.Merge(X3, Y3);
	m_Dataset = poDataset;

	return true;
}