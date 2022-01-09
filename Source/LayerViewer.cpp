//==============================================================================
// LayerViewer.cpp
//
// Author : F.Becirspahic
// Date : 19/12/2021
//==============================================================================

#include "LayerViewer.h"

//==============================================================================
// LayerViewerComponent : constructeur
//==============================================================================
LayerViewerModel::LayerViewerModel()
{
	m_Base = nullptr;
	m_ActiveRow = m_ActiveColumn = -1;
}

//==============================================================================
// Nombre de lignes de la table
//==============================================================================
int LayerViewerModel::getNumRows()
{
	if (m_Base == nullptr)
		return 0;
	return m_Base->GetVectorLayerCount();
}

// This is overloaded from TableListBoxModel, and should fill in the background of the whole row
void LayerViewerModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	g.setColour(juce::Colours::lightblue);
	if (rowIsSelected)
		g.drawRect(g.getClipBounds());
	g.setColour(juce::Colours::black);
}

void LayerViewerModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetVectorLayerCount())
		return;
	GeoBase::VectorLayer* geoLayer = m_Base->GetVectorLayer(rowNumber);
	if (geoLayer == nullptr)
		return;
	OGRLayer* ogrLayer = geoLayer->GetOGRLayer();
	if (ogrLayer == nullptr)
		return;
	switch(columnId) {
	case Column::Name :// Name
		g.drawText(juce::String(ogrLayer->GetName()), 0, 0, width, height, juce::Justification::centredLeft);
		break;
	case Column::PenWidth:// Width
		g.drawText(juce::String(geoLayer->m_Repres.PenSize), 0, 0, width, height, juce::Justification::centred);
		break;
	case Column::PenColour:// Pen
		g.setColour(juce::Colour(geoLayer->m_Repres.PenColor));
		g.fillRect(0, 0, width, height);
		break;
	case Column::FillColour:// brush
		g.setColour(juce::Colour(geoLayer->m_Repres.FillColor));
		g.fillRect(0, 0, width, height);
		break;
	}
}

//==============================================================================
// Double-clic dans une cellule
//==============================================================================
void LayerViewerModel::cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetVectorLayerCount())
		return;
	GeoBase::VectorLayer* geoLayer = m_Base->GetVectorLayer(rowNumber);
	m_ActiveRow = rowNumber;
	m_ActiveColumn = columnId;
	juce::Rectangle<int> bounds;
	bounds.setCentre(event.getMouseDownScreenPosition());
	bounds.setWidth(1); bounds.setHeight(1);

	// Choix d'une couleur
	if ((columnId == Column::PenColour) || (columnId == Column::FillColour)) {
		auto colourSelector = std::make_unique<juce::ColourSelector>(juce::ColourSelector::showAlphaChannel
			| juce::ColourSelector::showColourAtTop
			| juce::ColourSelector::editableColour
			| juce::ColourSelector::showSliders
			| juce::ColourSelector::showColourspace);

		colourSelector->setName("background");
		colourSelector->setCurrentColour(juce::Colour(geoLayer->m_Repres.PenColor));
		colourSelector->addChangeListener(this);
		colourSelector->setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
		colourSelector->setSize(400, 300);

		juce::CallOutBox::launchAsynchronously(std::move(colourSelector), bounds, nullptr);
		return;
	}
	// Choix d'une epaisseur
	if (columnId == Column::PenWidth) {
		auto widthSelector = std::make_unique<juce::Slider>();
		widthSelector->setRange(0., 20., 1.);
		widthSelector->setValue(geoLayer->m_Repres.PenSize);
		widthSelector->setSliderStyle(juce::Slider::LinearHorizontal);
		widthSelector->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);
		widthSelector->setSize(200, 50);
		widthSelector->addListener(this);
		juce::CallOutBox::launchAsynchronously(std::move(widthSelector), bounds, nullptr);
		return;
	}
}

//==============================================================================
// LayerViewer : constructeur
//==============================================================================
void LayerViewerModel::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (m_Base == nullptr)
		return;
	if (m_ActiveRow >= m_Base->GetVectorLayerCount())
		return;
	GeoBase::VectorLayer* geoLayer = m_Base->GetVectorLayer(m_ActiveRow);

	// Choix d'une couleur
	if ((m_ActiveColumn == Column::PenColour) || (m_ActiveColumn == Column::FillColour)) {
		if (auto* cs = dynamic_cast<juce::ColourSelector*> (source)) {
			GUInt32 color = cs->getCurrentColour().getARGB();
			if (m_ActiveColumn == Column::PenColour)
				geoLayer->m_Repres.PenColor = color;
			if (m_ActiveColumn == Column::FillColour)
				geoLayer->m_Repres.FillColor = color;
			sendActionMessage("UpdateRepres");
		}
	}
}

//==============================================================================
// Changement de valeur des sliders
//==============================================================================
void LayerViewerModel::sliderValueChanged(juce::Slider* slider)
{
	if (m_Base == nullptr)
		return;
	if (m_ActiveRow >= m_Base->GetVectorLayerCount())
		return;
	GeoBase::VectorLayer* geoLayer = m_Base->GetVectorLayer(m_ActiveRow);

	// Choix d'une epaisseur
	if (m_ActiveColumn == Column::PenWidth) {
		geoLayer->m_Repres.PenSize = (int)slider->getValue();
		sendActionMessage("UpdateRepres");
	}
}

//==============================================================================
// LayerViewer : constructeur
//==============================================================================
LayerViewer::LayerViewer()
{
	setName("Layers");
	m_Model.addActionListener(this);
	// Bordure
	setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
	setOutlineThickness(1);
	// Ajout des colonnes
	getHeader().addColumn(juce::translate("Name"), LayerViewerModel::Column::Name, 200);
	getHeader().addColumn(juce::translate("Width"), LayerViewerModel::Column::PenWidth, 50);
	getHeader().addColumn(juce::translate("Pen"), LayerViewerModel::Column::PenColour, 50);
	getHeader().addColumn(juce::translate("Brush"), LayerViewerModel::Column::FillColour, 50);

	setSize(352, 200);

	setModel(&m_Model);
}

//==============================================================================
// LayerViewer : mise a jour du nom des colonnes (pour la traduction)
//==============================================================================
void LayerViewer::UpdateColumnName()
{
	getHeader().setColumnName(LayerViewerModel::Column::Name, juce::translate("Name"));
	getHeader().setColumnName(LayerViewerModel::Column::PenWidth, juce::translate("Width"));
	getHeader().setColumnName(LayerViewerModel::Column::PenColour, juce::translate("Pen"));
	getHeader().setColumnName(LayerViewerModel::Column::FillColour, juce::translate("Brush"));
}

//==============================================================================
// Gestion des actions
//==============================================================================
void LayerViewer::actionListenerCallback(const juce::String& message)
{
	if (message == "UpdateRepres") {
		repaint();
	}
}