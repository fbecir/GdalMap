//==============================================================================
// SelectionViewer.cpp
//
// Author : F.Becirspahic
// Date : 03/01/2022
//==============================================================================

#include "SelectionViewer.h"

//==============================================================================
// SelectionViewerModel : constructeur
//==============================================================================
SelectionViewerModel::SelectionViewerModel()
{
	m_Base = nullptr;
	m_ActiveRow = m_ActiveColumn = -1;
}

//==============================================================================
// Nombre de lignes de la table
//==============================================================================
int SelectionViewerModel::getNumRows()
{
	if (m_Base == nullptr)
		return 0;
	return m_Base->GetSelectionCount();
}

// This is overloaded from TableListBoxModel, and should fill in the background of the whole row
void SelectionViewerModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	g.setColour(juce::Colours::lightblue);
	if (rowIsSelected)
		g.drawRect(g.getClipBounds());
	g.setColour(juce::Colours::white);
}

void SelectionViewerModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetSelectionCount())
		return;
	GeoBase::Feature geoFeature = m_Base->GetSelection(rowNumber);
	OGRLayer* ogrLayer = m_Base->GetOGRLayer(geoFeature.IdLayer());
	if (ogrLayer == nullptr)
		return;
	OGREnvelope env = geoFeature.Envelope();
	switch(columnId) {
	case Column::Name :
		g.drawText(juce::String(ogrLayer->GetName()), 0, 0, width, height, juce::Justification::centredLeft);
		break;
	case Column::Index :
		g.drawText(juce::String(geoFeature.Id()), 0, 0, width, height, juce::Justification::centred);
		break;
	case Column::MinX:
		g.drawText(juce::String(env.MinX, 2), 0, 0, width, height, juce::Justification::centred);
		break;
	case Column::MaxX:
		g.drawText(juce::String(env.MaxX, 2), 0, 0, width, height, juce::Justification::centred);
		break;
	case Column::MinY:
		g.drawText(juce::String(env.MinY, 2), 0, 0, width, height, juce::Justification::centred);
		break;
	case Column::MaxY:
		g.drawText(juce::String(env.MaxY, 2), 0, 0, width, height, juce::Justification::centred);
		break;
	}
}

//==============================================================================
// Double-clic dans une cellule
//==============================================================================
void SelectionViewerModel::cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetSelectionCount())
		return;
	GeoBase::Feature geoFeature = m_Base->GetSelection(rowNumber);
	OGREnvelope env = geoFeature.Envelope();
	OGRLayer* layer = m_Base->GetOGRLayer(geoFeature.IdLayer());
	if (layer == nullptr)
		return;
	env = GeoBase::ConvertEnvelop(env, layer->GetSpatialRef(), m_Base->SpatialRef());
	sendActionMessage("ZoomEnvelope:" + juce::String(env.MinX,2) + ":" + juce::String(env.MaxX,2) + ":" +
																			juce::String(env.MinY,2) + ":" + juce::String(env.MaxY,2));
	sendActionMessage("SelectFeature:"+ juce::String(geoFeature.Id())+":"+juce::String(geoFeature.IdLayer()));
}

//==============================================================================
// SelectionViewer : constructeur
//==============================================================================
SelectionViewer::SelectionViewer()
{
	m_Model.addActionListener(this);
	// give it a border
	setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
	setOutlineThickness(1);
	// Ajout des colonnes
	getHeader().addColumn("Name", SelectionViewerModel::Column::Name, 150);
	getHeader().addColumn("Index", SelectionViewerModel::Column::Index, 50);
	getHeader().addColumn("MinX", SelectionViewerModel::Column::MinX, 100);
	getHeader().addColumn("MaxX", SelectionViewerModel::Column::MaxX, 100);
	getHeader().addColumn("MinY", SelectionViewerModel::Column::MinY, 100);
	getHeader().addColumn("MaxY", SelectionViewerModel::Column::MaxY, 100);

	setSize(602, 400);
	setModel(&m_Model);
}

//==============================================================================
// SelectionViewer : mise a jour du nom des colonnes (pour la traduction)
//==============================================================================
void SelectionViewer::UpdateColumnName()
{
	getHeader().setColumnName(SelectionViewerModel::Column::Name, juce::translate("Name"));
	getHeader().setColumnName(SelectionViewerModel::Column::Index, juce::translate("Index"));
	getHeader().setColumnName(SelectionViewerModel::Column::MinX, juce::translate("MinX"));
	getHeader().setColumnName(SelectionViewerModel::Column::MaxX, juce::translate("MaxX"));
	getHeader().setColumnName(SelectionViewerModel::Column::MinY, juce::translate("MinY"));
	getHeader().setColumnName(SelectionViewerModel::Column::MaxY, juce::translate("MaxY"));
}

//==============================================================================
// Gestion des actions
//==============================================================================
void SelectionViewer::actionListenerCallback(const juce::String& message)
{
	if (message == "UpdateVector") {
		repaint();
	}
}