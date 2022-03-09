//==============================================================================
// RasterLayerViewer.cpp
//
// Created: 10 Jan 2022 9:58:48pm
// Author:  FBecirspahic
//==============================================================================

#include "RasterLayerViewer.h"
#include "Utilities.h"

//==============================================================================
// RasterLayerViewerModel : constructeur
//==============================================================================
RasterLayerViewerModel::RasterLayerViewerModel()
{
	m_Base = nullptr;
	m_ActiveRow = m_ActiveColumn = -1;
}

//==============================================================================
// Nombre de lignes de la table
//==============================================================================
int RasterLayerViewerModel::getNumRows()
{
	if (m_Base == nullptr)
		return 0;
	return m_Base->GetRasterLayerCount();
}

// This is overloaded from TableListBoxModel, and should fill in the background of the whole row
void RasterLayerViewerModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	g.setColour(juce::Colours::lightblue);
	if (rowIsSelected)
		g.drawRect(g.getClipBounds());
	g.setColour(juce::Colours::white);
}

void RasterLayerViewerModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetRasterLayerCount())
		return;
	GeoBase::RasterLayer* geoLayer = m_Base->GetRasterLayer(rowNumber);
	if (geoLayer == nullptr)
		return;

	juce::Image icone;
	switch (columnId) {
	case Column::Visibility:
		if (geoLayer->Visible)
			icone = getImageFromAssets("View.png");
		else
			icone = getImageFromAssets("NoView.png");
		g.drawImageAt(icone, (width - icone.getWidth()) / 2, (height - icone.getHeight()) / 2);
		break;
	case Column::Name:
		g.drawText(juce::String(geoLayer->Name()), 0, 0, width, height, juce::Justification::centredLeft);
		break;
	case Column::Opacity:
		g.drawText(juce::String(geoLayer->Opacity()*100), 0, 0, width, height, juce::Justification::centred);
		break;
	case Column::GSD:
		g.drawText(juce::String(geoLayer->GSD()), 0, 0, width, height, juce::Justification::centredLeft);
		break;
	}
}

//==============================================================================
// Clic dans une cellule
//==============================================================================
void RasterLayerViewerModel::cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetRasterLayerCount())
		return;
	GeoBase::RasterLayer* layer = m_Base->GetRasterLayer(rowNumber);
	m_ActiveRow = rowNumber;
	m_ActiveColumn = columnId;
	juce::Rectangle<int> bounds;
	bounds.setCentre(event.getMouseDownScreenPosition());
	bounds.setWidth(1); bounds.setHeight(1);

	// Visibilite
	if (columnId == Column::Visibility) {
		layer->Visible = !layer->Visible;
		sendActionMessage("UpdateRaster");
		return;
	}

	// Choix d'une opacite
	if (columnId == Column::Opacity) {
		auto opacitySelector = std::make_unique<juce::Slider>();
		opacitySelector->setRange(0., 100., 1.);
		opacitySelector->setValue(layer->Opacity()*100.);
		opacitySelector->setSliderStyle(juce::Slider::LinearHorizontal);
		opacitySelector->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);
		opacitySelector->setSize(200, 50);
		opacitySelector->addListener(this);
		juce::CallOutBox::launchAsynchronously(std::move(opacitySelector), bounds, nullptr);
		return;
	}
}

//==============================================================================
// Drag&Drop des lignes pour changer l'ordre des layers
//==============================================================================
juce::var RasterLayerViewerModel::getDragSourceDescription(const juce::SparseSet<int>& selectedRows)
{
	juce::StringArray rows;
	for (int i = 0; i < selectedRows.size(); i++)
		rows.add(juce::String(selectedRows[i]));
	return rows.joinIntoString(":");
}

//==============================================================================
// LayerViewer : constructeur
//==============================================================================
void RasterLayerViewerModel::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (m_Base == nullptr)
		return;
	if (m_ActiveRow >= m_Base->GetRasterLayerCount())
		return;
	GeoBase::RasterLayer* geoLayer = m_Base->GetRasterLayer(m_ActiveRow);
	/*
	// Choix d'une couleur
	if ((m_ActiveColumn == Column::PenColour) || (m_ActiveColumn == Column::FillColour)) {
		if (auto* cs = dynamic_cast<juce::ColourSelector*> (source)) {
			GUInt32 color = cs->getCurrentColour().getARGB();
			if (m_ActiveColumn == Column::PenColour)
				geoLayer->m_Repres.PenColor = color;
			if (m_ActiveColumn == Column::FillColour)
				geoLayer->m_Repres.FillColor = color;
			sendActionMessage("UpdateRaster");
		}
	}
	*/
}

//==============================================================================
// Changement de valeur des sliders
//==============================================================================
void RasterLayerViewerModel::sliderValueChanged(juce::Slider* slider)
{
	if (m_Base == nullptr)
		return;
	if (m_ActiveRow >= m_Base->GetRasterLayerCount())
		return;
	GeoBase::RasterLayer* geoLayer = m_Base->GetRasterLayer(m_ActiveRow);

	// Choix d'une opacite
	if (m_ActiveColumn == Column::Opacity) {
		geoLayer->Opacity(slider->getValue() * 0.01);
		sendActionMessage("UpdateRaster");
	}
}

//==============================================================================
// RasterLayerViewer : constructeur
//==============================================================================
RasterLayerViewer::RasterLayerViewer()
{
	m_Base = nullptr;
	setName("Raster Layers");
	m_Model.addActionListener(this);
	// Bordure
	m_Table.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
	m_Table.setOutlineThickness(1);
	// Ajout des colonnes
	m_Table.getHeader().addColumn(juce::translate(" "), RasterLayerViewerModel::Column::Visibility, 25);
	m_Table.getHeader().addColumn(juce::translate("Name"), RasterLayerViewerModel::Column::Name, 200);
	m_Table.getHeader().addColumn(juce::translate("Opacity"), RasterLayerViewerModel::Column::Opacity, 50);
	m_Table.getHeader().addColumn(juce::translate("GSD"), RasterLayerViewerModel::Column::GSD, 50);
	m_Table.setSize(352, 200);
	m_Table.setModel(&m_Model);
	addAndMakeVisible(m_Table);
}

//==============================================================================
// LayerViewer : mise a jour du nom des colonnes (pour la traduction)
//==============================================================================
void RasterLayerViewer::UpdateColumnName()
{
	m_Table.getHeader().setColumnName(RasterLayerViewerModel::Column::Visibility, juce::translate(" "));
	m_Table.getHeader().setColumnName(RasterLayerViewerModel::Column::Name, juce::translate("Name"));
	m_Table.getHeader().setColumnName(RasterLayerViewerModel::Column::Opacity, juce::translate("Opacity"));
	m_Table.getHeader().setColumnName(RasterLayerViewerModel::Column::GSD, juce::translate("GSD"));
}

//==============================================================================
// Gestion des actions
//==============================================================================
void RasterLayerViewer::actionListenerCallback(const juce::String& message)
{
	if (message == "UpdateRaster") {
		repaint();
	}
}

//==============================================================================
// Drag&Drop
//==============================================================================
void RasterLayerViewer::itemDropped(const SourceDetails& details)
{
	juce::String message = details.description.toString();
	juce::StringArray T;
	T.addTokens(message, ":", "");
	if (T.size() < 1)
		return;
	int i;
	i = T[0].getIntValue();
	int row = m_Table.getRowContainingPosition(details.localPosition.x, details.localPosition.y);
	m_Base->ReorderRasterLayer(i, row);
	//m_Table.updateContent();
	//m_Table.repaint();
	m_Model.sendActionMessage("UpdateRaster");
}

bool RasterLayerViewer::isInterestedInDragSource(const SourceDetails& details)
{
	if (details.sourceComponent.get() != &m_Table)
		return false;
	return true;
}