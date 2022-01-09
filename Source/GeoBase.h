//==============================================================================
// GeoBase.h
//
// Author : F.Becirspahic
// Date : 14/12/2021
//==============================================================================

#pragma once
#include "ogrsf_frmts.h"

class GDALDataset;

class GeoBase {
public:
	class VectorLayer;
	class Feature;
	class RasterLayer;
	class Raster;

	GeoBase();
	virtual ~GeoBase() { Clear(); }
	void Clear();

	bool OpenVectorDataset(const char* filename);
	bool OpenRasterDataset(const char* filename);
	size_t SelectFeatures(const OGREnvelope& env);
	bool SelectFeatureFields(int layerId, GIntBig featureId);

	int GetVectorLayerCount() { return m_VLayers.size(); }
	VectorLayer* GetVectorLayer(int i) { if (i < m_VLayers.size()) return m_VLayers[i]; return nullptr; }
	bool ReorderVectorLayer(int oldPosition, int newPosition);
	OGRLayer* GetOGRLayer(int i) { if (i < m_VLayers.size()) return m_VLayers[i]->GetOGRLayer(); return nullptr; }
	int GetRasterLayerCount() { return m_RLayers.size(); }
	RasterLayer* GetRasterLayer(int i) { if (i < m_RLayers.size()) return m_RLayers[i]; return nullptr; }

	OGREnvelope GetEnvelope() { return m_Env; }
	size_t GetSelectionCount() { return m_Selection.size(); }
	Feature GetSelection(size_t index) { return m_Selection[index]; }
	int GetFieldCount() { return m_Field.size() / 2; }
	std::string GetFieldName(int i) { if (i < GetFieldCount()) return m_Field[2 * i]; return ""; }
	std::string GetFieldValue(int i) { if (i < GetFieldCount()) return m_Field[2 * i + 1]; return ""; }

	typedef struct {
		GUInt32			PenColor;
		GUInt32			FillColor;
		float				PenSize;
	} Repres;

	class Feature {
	protected:
		OGREnvelope		m_Env;
		GIntBig				m_Id;
		int						m_IdLayer;
	public:
		Feature(GIntBig id, OGREnvelope env, int idLayer) { m_Id = id; m_Env = env; m_IdLayer = idLayer; }
		bool Intersect(OGREnvelope env) { return m_Env.Intersects(env); }
		GIntBig Id() { return m_Id; }
		OGREnvelope Envelope() { return m_Env; }
		int IdLayer() { return m_IdLayer; }
	};

	class VectorLayer {
	protected:
		GDALDataset*	m_Dataset;
		OGRLayer*			m_OGRLayer;
		int						m_Id;
		OGREnvelope		m_Env;
		bool					m_bTransactions;
		OGREnvelope		m_FilterRect;
		size_t				m_nIndex;
		std::vector<Feature> m_T;
	public:
		VectorLayer();
		bool SetDataset(GDALDataset* poDataset, int id);
		OGREnvelope Envelope() { return m_Env; }
		GIntBig GetFeatureCount() { if (m_OGRLayer != nullptr) return m_OGRLayer->GetFeatureCount(); return 0; }
		void SetSpatialFilterRect(const OGREnvelope& env);
		void ResetReading() { if (m_OGRLayer != nullptr) m_OGRLayer->ResetReading(); m_nIndex = 0; }
		OGRFeature* GetNextFeature();
		bool GetNextFeatureId(GIntBig& id, OGREnvelope& env);
		OGRLayer* GetOGRLayer() { return m_OGRLayer; }

		Repres				m_Repres;
	};

	class Raster {
	protected:
		GDALDataset*		m_Dataset;
		OGREnvelope			m_Env;
	public:
		Raster() { m_Dataset = nullptr; }
		bool AddDataset(GDALDataset* poDataset);
		OGREnvelope Envelope() { return m_Env; }
		GDALDataset* Dataset() { return m_Dataset; }
	};

	class RasterLayer {
	protected:
		std::vector<Raster>			m_Raster;
		OGREnvelope							m_TotalEnv;
	public:
		RasterLayer() { ; }
		OGREnvelope Envelope() { return m_TotalEnv; }
		bool AddDataset(GDALDataset* poDataset);
		int GetRasterCount() { return m_Raster.size(); }
		GDALDataset* GetRasterDataset(int i) { if (i < m_Raster.size()) return m_Raster[i].Dataset(); return nullptr; }
		OGREnvelope GetRasterEnvelope(int i) { if (i < m_Raster.size()) return m_Raster[i].Envelope(); return OGREnvelope(); }
	};

protected:
	OGREnvelope								m_Env;
	std::vector<GDALDataset*> m_Dataset;
	std::vector<VectorLayer*>	m_VLayers;
	std::vector<RasterLayer*>	m_RLayers;
	std::vector<Feature>			m_Selection;
	std::vector<std::string>	m_Field;

};