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
	bool OpenRasterDataset(const char* filename, const char* name = nullptr, bool visible = true,
												 char** options = nullptr, bool dtm = false);
	bool OpenRasterMultiDataset(const char* filename);
	bool IsOpen(const char* filename);
	size_t SelectFeatures(const OGREnvelope& env, OGRSpatialReference* spatialRef);
	bool SelectFeatureFields(int layerId, GIntBig featureId);

	OGRSpatialReference* SpatialRef() { return &m_SpatialRef; }
	int GetVectorLayerCount() { return (int)m_VLayers.size(); }
	VectorLayer* GetVectorLayer(int i) { if (i < m_VLayers.size()) return m_VLayers[i]; return nullptr; }
	VectorLayer* GetVectorLayerId(int id) { for (int i = 0; i < m_VLayers.size(); i++) if (m_VLayers[i]->Id() == id) return m_VLayers[i]; return nullptr; }

	// Change l'ordre des layers
	bool ReorderVectorLayer(int oldPosition, int newPosition);
	bool ReorderRasterLayer(int oldPosition, int newPosition);
	bool ReorderDtmLayer(int oldPosition, int newPosition);

	OGRLayer* GetOGRLayer(int id) { VectorLayer* layer = GetVectorLayerId(id); if (layer != nullptr) return layer->GetOGRLayer(); return nullptr; }
	int GetRasterLayerCount() { return (int)m_RLayers.size(); }
	RasterLayer* GetRasterLayer(int i) { if (i < m_RLayers.size()) return m_RLayers[i]; return nullptr; }
	int GetDtmLayerCount() { return (int)m_ZLayers.size(); }
	RasterLayer* GetDtmLayer(int i) { if (i < m_ZLayers.size()) return m_ZLayers[i]; return nullptr; }

	OGREnvelope GetEnvelope() { return m_Env; }
	size_t GetSelectionCount() { return m_Selection.size(); }
	Feature GetSelection(size_t index) { return m_Selection[index]; }
	int GetFieldCount() { return (int)m_Field.size() / 2; }
	std::string GetFieldName(int i) { if (i < GetFieldCount()) return m_Field[2 * i]; return ""; }
	std::string GetFieldValue(int i) { if (i < GetFieldCount()) return m_Field[2 * i + 1]; return ""; }

	static OGREnvelope ConvertEnvelop(const OGREnvelope& env, OGRSpatialReference* fromRef, OGRSpatialReference* toRef);

	typedef struct {
		GUInt32			PenColor;
		GUInt32			FillColor;
		float				PenSize;
		bool				Visible;
	} Repres;

	class Feature {
	protected:
		OGREnvelope		m_Env;
		GIntBig				m_Id;
		int						m_IdLayer;
	public:
		Feature() { m_Id = -1; m_IdLayer = -1; }
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
		VectorLayer(int id);
		inline int Id() { return m_Id; }
		bool SetDataset(GDALDataset* poDataset, int id);
		OGREnvelope Envelope() { return m_Env; }
		OGRSpatialReference* SpatialRef() { if (m_OGRLayer != nullptr) return m_OGRLayer->GetSpatialRef(); return nullptr; }
		GIntBig GetFeatureCount() { if (m_OGRLayer != nullptr) return m_OGRLayer->GetFeatureCount(); return 0; }
		void SetSpatialFilterRect(const OGREnvelope& env, OGRSpatialReference* spatialRef);
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
		double					m_GSD;
	public:
		Raster() { m_Dataset = nullptr; m_GSD = 0.; }
		bool AddDataset(GDALDataset* poDataset);
		OGREnvelope Envelope() { return m_Env; }
		GDALDataset* Dataset() { return m_Dataset; }
		double GSD() { return m_GSD; }
	};

	class RasterLayer {
	protected:
		std::vector<Raster>			m_Raster;
		OGREnvelope							m_TotalEnv;
		std::string							m_Name;
		float										m_Opacity;
	public:
		RasterLayer() { m_Name = "RASTER"; m_Opacity = 1.f; Visible = true; }
		OGREnvelope Envelope() { return m_TotalEnv; }
		std::string Name() { return m_Name; }
		void Name(const char* name) { m_Name = name; }
		float Opacity() { return m_Opacity; }
		void Opacity(float opa) { m_Opacity = opa;  if (opa < 0.) m_Opacity = 0; if (opa > 1.) m_Opacity = 1.;}
		bool AddDataset(GDALDataset* poDataset);
		int GetRasterCount() { return (int)m_Raster.size(); }
		GDALDataset* GetRasterDataset(int i) { if (i < m_Raster.size()) return m_Raster[i].Dataset(); return nullptr; }
		OGREnvelope GetRasterEnvelope(int i) { if (i < m_Raster.size()) return m_Raster[i].Envelope(); return OGREnvelope(); }
		double GSD();

		bool				Visible;
	};

protected:
	OGRSpatialReference				m_SpatialRef;
	OGREnvelope								m_Env;
	std::vector<GDALDataset*> m_Dataset;		// All the datasets
	std::vector<VectorLayer*>	m_VLayers;		// Vector layers
	std::vector<RasterLayer*>	m_RLayers;		// Raster layers
	std::vector<RasterLayer*>	m_ZLayers;		// DTM layers
	std::vector<Feature>			m_Selection;	// Selected features
	std::vector<std::string>	m_Field;			// Fields of the selected feature

	template<typename T> static bool ReorderLayer(std::vector<T*>* V, int oldPosition, int newPosition);
};