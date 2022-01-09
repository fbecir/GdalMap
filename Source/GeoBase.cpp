//==============================================================================
// GeoBase.cpp
//
// Author : F.Becirspahic
// Date : 14/12/2021
//==============================================================================

#include "GeoBase.h"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()


//==============================================================================
// Constructeur GeoBase
//==============================================================================
GeoBase::GeoBase()
{

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
	for (int i = 0; i < m_Dataset.size(); i++)
		m_Dataset[i]->Release();
	m_Dataset.clear();
}

//==============================================================================
// Ouverture d'un dataset vectoriel
//==============================================================================
bool GeoBase::OpenVectorDataset(const char* filename)
{
	GDALDataset* poDataset = GDALDataset::Open(filename, GDAL_OF_VECTOR | GDAL_OF_READONLY);
	if (poDataset == NULL) 
		return false;

	// Analyse des layers
	OGREnvelope total;
	for (int i = 0; i < poDataset->GetLayerCount(); i++) {
		VectorLayer* layer = new VectorLayer;
		if (layer == nullptr)
			continue;
		if (!layer->SetDataset(poDataset, i)) {
			delete layer;
			continue;
		}
		m_VLayers.push_back(layer);
		m_Env.Merge(layer->Envelope());
	}
	m_Dataset.push_back(poDataset);

	return true;
}

//==============================================================================
// Ouverture d'un dataset raster
//==============================================================================
bool GeoBase::OpenRasterDataset(const char* filename)
{
	GDALDataset* poDataset = GDALDataset::Open(filename, GDAL_OF_RASTER | GDAL_OF_READONLY);
	if (poDataset == NULL)
		return false;

	RasterLayer* layer = new RasterLayer;
	if (layer->AddDataset(poDataset)) {
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
// Selection des features dans une enveloppe
//==============================================================================
size_t GeoBase::SelectFeatures(const OGREnvelope& env)
{
	m_Selection.clear();
	for (int layerId = 0; layerId < GetVectorLayerCount(); layerId++) {
		VectorLayer* poLayer = GetVectorLayer(layerId);
		if (poLayer == nullptr)
			continue;
		poLayer->SetSpatialFilterRect(env);
		poLayer->ResetReading();
		GIntBig featureId;
		OGREnvelope featureEnv;
		do {
			if (!poLayer->GetNextFeatureId(featureId, featureEnv))
				break;
			m_Selection.push_back(Feature(featureId, featureEnv, layerId));
		} while (true);
	}
	return m_Selection.size();
}

//==============================================================================
// Selection des champs d'un feature
//==============================================================================
bool GeoBase::SelectFeatureFields(int layerId, GIntBig featureId)
{
	m_Field.clear();
	if (layerId >= GetVectorLayerCount())
		return false;
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
// Constructeur GeoLayer
//==============================================================================
GeoBase::VectorLayer::VectorLayer()
{
	m_Dataset = nullptr;
	m_OGRLayer = nullptr; 
	m_Id = 0;
	m_bTransactions = false;
	m_nIndex = 0;
	m_Repres.PenColor = 0xCC008800;
	m_Repres.FillColor = 0x55770000;
	m_Repres.PenSize = 4.;
}

//==============================================================================
// Ouverture d'un dataset vectoriel
//==============================================================================
bool GeoBase::VectorLayer::SetDataset(GDALDataset* poDataset, int id)
{
	if (id > poDataset->GetLayerCount())
		return false;
	m_OGRLayer = poDataset->GetLayer(id);
	if (m_OGRLayer == nullptr)
		return false;
	if (poDataset->TestCapability(ODsCTransactions)) {
		std::string geom = m_OGRLayer->GetGeometryColumn();
		if (geom.size() < 1) // Layer non geometrique
			return false;
		m_bTransactions = true;
		m_OGRLayer->GetExtent(&m_Env);
		return true;
	}
	OGREnvelope env;
	for (GIntBig i = 0; i < m_OGRLayer->GetFeatureCount(); i++) {
		OGRFeature* poFeature = m_OGRLayer->GetFeature(i);
		if (poFeature == nullptr)
			continue;
		const OGRGeometry* poGeom = poFeature->GetGeometryRef();
		poGeom->getEnvelope(&env);
		m_T.push_back(Feature(i, env, id));
		OGRFeature::DestroyFeature(poFeature);
		m_Env.Merge(env);
	}
	m_FilterRect = m_Env;
	return true;
}

//==============================================================================
// Fixe l'enveloppe pour la recherche de feature
//==============================================================================
void GeoBase::VectorLayer::SetSpatialFilterRect(const OGREnvelope& env)
{
	if (m_OGRLayer == nullptr)
		return;
	m_FilterRect = env;
	m_OGRLayer->SetSpatialFilterRect(env.MinX, env.MinY, env.MaxX, env.MaxY);
	m_nIndex = 0;
}

//==============================================================================
// Renvoie le prochain feature
//==============================================================================
OGRFeature* GeoBase::VectorLayer::GetNextFeature()
{
	if (m_OGRLayer == nullptr)
		return nullptr;
	if (m_bTransactions)
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
	if (m_bTransactions) {
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

//==============================================================================
// Ajout d'un dataset raster
//==============================================================================
bool GeoBase::Raster::AddDataset(GDALDataset* poDataset)
{
	double transfo[6];
	poDataset->GetGeoTransform(transfo);
	if (fabs(transfo[1] * transfo[5] - transfo[2] * transfo[4]) < 0.000001)
		return false;	// Transformation non affine
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