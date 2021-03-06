//==============================================================================
// DtmViewer.cpp
// Created: 21 Jan 2022 10:09:55am
// Author:  FBecirspahic
//==============================================================================


#include "DtmViewer.h"
#include "Utilities.h"
#include "DtmShader.h"

//==============================================================================
// DtmViewerModel : constructeur
//==============================================================================
DtmViewerModel::DtmViewerModel()
{
	m_Base = nullptr;
	m_ActiveRow = m_ActiveColumn = -1;
}

//==============================================================================
// Nombre de lignes de la table
//==============================================================================
int DtmViewerModel::getNumRows()
{
	if (m_Base == nullptr)
		return 0;
	return m_Base->GetDtmLayerCount();
}

// This is overloaded from TableListBoxModel, and should fill in the background of the whole row
void DtmViewerModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	g.setColour(juce::Colours::lightblue);
	if (rowIsSelected)
		g.drawRect(g.getClipBounds());
	g.setColour(juce::Colours::white);
}

void DtmViewerModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetDtmLayerCount())
		return;
	GeoBase::RasterLayer* geoLayer = m_Base->GetDtmLayer(rowNumber);
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
		g.drawText(juce::String(geoLayer->Opacity() * 100), 0, 0, width, height, juce::Justification::centred);
		break;
	}
}

//==============================================================================
// Clic dans une cellule
//==============================================================================
void DtmViewerModel::cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetDtmLayerCount())
		return;
	GeoBase::RasterLayer* layer = m_Base->GetDtmLayer(rowNumber);
	m_ActiveRow = rowNumber;
	m_ActiveColumn = columnId;
	juce::Rectangle<int> bounds;
	bounds.setCentre(event.getMouseDownScreenPosition());
	bounds.setWidth(1); bounds.setHeight(1);

	// Visibilite
	if (columnId == Column::Visibility) {
		layer->Visible = !layer->Visible;
		sendActionMessage("UpdateDtm");
		return;
	}

	// Choix d'une opacite
	if (columnId == Column::Opacity) {
		auto opacitySelector = std::make_unique<juce::Slider>();
		opacitySelector->setRange(0., 100., 1.);
		opacitySelector->setValue(layer->Opacity() * 100.);
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
juce::var DtmViewerModel::getDragSourceDescription(const juce::SparseSet<int>& selectedRows)
{
	juce::StringArray rows;
	for (int i = 0; i < selectedRows.size(); i++)
		rows.add(juce::String(selectedRows[i]));
	return rows.joinIntoString(":");
}

//==============================================================================
// LayerViewer : constructeur
//==============================================================================
void DtmViewerModel::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	if (m_Base == nullptr)
		return;
	if (m_ActiveRow >= m_Base->GetDtmLayerCount())
		return;
	GeoBase::RasterLayer* geoLayer = m_Base->GetDtmLayer(m_ActiveRow);
}

//==============================================================================
// Changement de valeur des sliders
//==============================================================================
void DtmViewerModel::sliderValueChanged(juce::Slider* slider)
{
	if (m_Base == nullptr)
		return;
	if (m_ActiveRow >= m_Base->GetDtmLayerCount())
		return;
	GeoBase::RasterLayer* geoLayer = m_Base->GetDtmLayer(m_ActiveRow);

	// Choix d'une opacite
	if (m_ActiveColumn == Column::Opacity) {
		geoLayer->Opacity(slider->getValue() * 0.01);
		sendActionMessage("UpdateDtm");
	}
}

//==============================================================================
// Nombre de lignes de la table
//==============================================================================
int DtmRangeModel::getNumRows()
{
	return DtmShader::m_Z.size() + 1;
}

// This is overloaded from TableListBoxModel, and should fill in the background of the whole row
void DtmRangeModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	g.setColour(juce::Colours::lightblue);
	if (rowIsSelected)
		g.drawRect(g.getClipBounds());
	g.setColour(juce::Colours::white);
}

void DtmRangeModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber > DtmShader::m_Z.size())
		return;
	switch (columnId) {
	case Column::Altitude:
		if (rowNumber == 0) g.drawText(juce::String(DtmShader::m_Z[rowNumber]), 0, 0, width, height, juce::Justification::centred);
		if (rowNumber == 1) g.drawText(" < " + juce::String(DtmShader::m_Z[rowNumber]), 0, 0, width, height, juce::Justification::centred);
		if ((rowNumber >= 2)&&(rowNumber < DtmShader::m_Z.size()))
			g.drawText(juce::String(DtmShader::m_Z[rowNumber]), 0, 0, width, height, juce::Justification::centred);
		if (rowNumber == DtmShader::m_Z.size())
			g.drawText(" > " + juce::String(DtmShader::m_Z[rowNumber-1]), 0, 0, width, height, juce::Justification::centred);
		break;
	case Column::Colour:
		g.setColour(DtmShader::m_Colour[rowNumber]);
		g.fillRect(0, 0, width, height);
		break;
	}
}

//==============================================================================
// Clic dans une cellule
//==============================================================================
void DtmRangeModel::cellClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
	if (rowNumber > DtmShader::m_Z.size())
		return;
	if (event.mods.isRightButtonDown()) {	// Clic bouton droit
		double z = DtmShader::m_Z[DtmShader::m_Z.size() - 1] + 100;
		if (rowNumber < DtmShader::m_Z.size() - 1)
			z = (DtmShader::m_Z[rowNumber] + DtmShader::m_Z[rowNumber + 1]) * 0.5;
		DtmShader::AddAltitude(z);
		sendActionMessage("UpdateRange");
		return;
	}

	m_ActiveRow = rowNumber;
	m_ActiveColumn = columnId;
	juce::Rectangle<int> bounds;
	bounds.setCentre(event.getMouseDownScreenPosition());
	bounds.setWidth(1); bounds.setHeight(1);

	if (columnId == Column::Altitude) {
		if (rowNumber >= DtmShader::m_Z.size())
			return;
		auto altitudeSelector = std::make_unique<juce::Slider>();
		double min = 0., max = 0.;
		if (rowNumber == 0)
			min = -9999.;
		else
			min = DtmShader::m_Z[rowNumber - 1];
		if (rowNumber == (DtmShader::m_Z.size() - 1))
			max = 10000.;
		else
			max = DtmShader::m_Z[rowNumber + 1];
		
		altitudeSelector->setRange(min, max, 1.);
		altitudeSelector->setValue(DtmShader::m_Z[rowNumber]);
		altitudeSelector->setSliderStyle(juce::Slider::LinearHorizontal);
		altitudeSelector->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);
		altitudeSelector->setSize(200, 50);
		altitudeSelector->addListener(this);
		juce::CallOutBox::launchAsynchronously(std::move(altitudeSelector), bounds, nullptr);
		return;
	}

	// Choix d'une couleur
	if (columnId == Column::Colour) {
		auto colourSelector = std::make_unique<juce::ColourSelector>(juce::ColourSelector::showAlphaChannel
			| juce::ColourSelector::showColourAtTop
			| juce::ColourSelector::editableColour
			| juce::ColourSelector::showSliders
			| juce::ColourSelector::showColourspace);

		colourSelector->setName("background");
		colourSelector->setCurrentColour(DtmShader::m_Colour[rowNumber]);
		colourSelector->addChangeListener(this);
		colourSelector->setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
		colourSelector->setSize(400, 300);

		juce::CallOutBox::launchAsynchronously(std::move(colourSelector), bounds, nullptr);
		return;
	}
}

//==============================================================================
// LayerViewer : constructeur
//==============================================================================
void DtmRangeModel::changeListenerCallback(juce::ChangeBroadcaster* source)
{
	// Choix d'une couleur
	if ((m_ActiveColumn == Column::Colour)) {
		if (auto* cs = dynamic_cast<juce::ColourSelector*> (source)) {
			DtmShader::m_Colour[m_ActiveRow] = cs->getCurrentColour();
			sendActionMessage("UpdateDtm");
		}
	}

}

//==============================================================================
// Changement de valeur des sliders
//==============================================================================
void DtmRangeModel::sliderValueChanged(juce::Slider* slider)
{
	if (m_ActiveRow >= DtmShader::m_Z.size())
		return;
	if (m_ActiveColumn == Column::Altitude) {	// Changement de l'altitude
		DtmShader::m_Z[m_ActiveRow] = slider->getValue();
		sendActionMessage("UpdateDtm");
	}
}

//==============================================================================
// DtmViewer : constructeur
//==============================================================================
DtmViewer::DtmViewer()
{
	m_Base = nullptr;
	DtmShader shader;	// Necessaire pour initialiser les plages et les couleurs
	setName("DTM Layers");
	m_ModelDtm.addActionListener(this);
	// Bordure
	m_TableDtm.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
	m_TableDtm.setOutlineThickness(1);
	// Ajout des colonnes
	m_TableDtm.getHeader().addColumn(juce::translate(" "), DtmViewerModel::Column::Visibility, 25);
	m_TableDtm.getHeader().addColumn(juce::translate("Name"), DtmViewerModel::Column::Name, 200);
	m_TableDtm.getHeader().addColumn(juce::translate("Opacity"), DtmViewerModel::Column::Opacity, 50);
	m_TableDtm.setSize(352, 200);
	m_TableDtm.setModel(&m_ModelDtm);
	addAndMakeVisible(m_TableDtm);

	m_ModelRange.addActionListener(this);
	// Bordure
	m_TableRange.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
	m_TableRange.setOutlineThickness(1);
	// Ajout des colonnes
	m_TableRange.getHeader().addColumn(juce::translate("Altitude"), DtmRangeModel::Column::Altitude, 50);
	m_TableRange.getHeader().addColumn(juce::translate("Colour"), DtmRangeModel::Column::Colour, 50);
	m_TableRange.setModel(&m_ModelRange);
	addAndMakeVisible(m_TableRange);

	m_Mode.addItem(juce::translate("Altitude"), 1);
	m_Mode.addItem(juce::translate("Standard shading"), 2);
	m_Mode.addItem(juce::translate("Light shading"), 3);
	m_Mode.addItem(juce::translate("Free shading"), 4); 
	m_Mode.addItem(juce::translate("Slope"), 5);
	m_Mode.addItem(juce::translate("Colours"), 6);
	m_Mode.addItem(juce::translate("Colours + Shading"), 7);
	m_Mode.addItem(juce::translate("Contour lines"), 8);
	
	m_Mode.addListener(this);
	m_Mode.setSelectedId(static_cast<int>(DtmShader::m_Mode) + 1);
	addAndMakeVisible(m_Mode);
	
	m_IsoStep.setRange(1., 250., 1.);
	m_IsoStep.setValue(DtmShader::m_dIsoStep);
	m_IsoStep.setSliderStyle(juce::Slider::LinearHorizontal);
	m_IsoStep.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);
	m_IsoStep.addListener(this);
	m_IsoStep.setChangeNotificationOnlyOnRelease(true);
	addAndMakeVisible(m_IsoStep);

	m_Azimuth.setRange(0., 360., 1.);
	m_Azimuth.setValue(DtmShader::m_dSolarAzimuth);
	m_Azimuth.setSliderStyle(juce::Slider::Rotary);
	m_Azimuth.setRotaryParameters((float)juce::MathConstants<double>::pi, 3.f*juce::MathConstants<double>::pi, false);
	m_Azimuth.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
	m_Azimuth.addListener(this);
	m_Azimuth.setChangeNotificationOnlyOnRelease(true);
	addAndMakeVisible(m_Azimuth);

	m_Zenith.setRange(0., 90., 1.);
	m_Zenith.setValue(DtmShader::m_dSolarZenith);
	m_Zenith.setSliderStyle(juce::Slider::LinearBarVertical);
	//m_Zenith.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
	m_Zenith.setChangeNotificationOnlyOnRelease(true);
	m_Zenith.addListener(this);
	addAndMakeVisible(m_Zenith);
}

//==============================================================================
// LayerViewer : mise a jour du nom des colonnes (pour la traduction)
//==============================================================================
void DtmViewer::UpdateColumnName()
{
	m_TableDtm.getHeader().setColumnName(DtmViewerModel::Column::Visibility, juce::translate(" "));
	m_TableDtm.getHeader().setColumnName(DtmViewerModel::Column::Name, juce::translate("Name"));
	m_TableDtm.getHeader().setColumnName(DtmViewerModel::Column::Opacity, juce::translate("Opacity"));
	m_TableRange.getHeader().setColumnName(DtmRangeModel::Column::Altitude, juce::translate("Altitude"));
	m_TableRange.getHeader().setColumnName(DtmRangeModel::Column::Colour, juce::translate("Colour"));
}

//==============================================================================
// Redimensionnement
//==============================================================================
void DtmViewer::resized()
{ 
	auto b = getLocalBounds();
	m_TableDtm.setTopLeftPosition(0, 0);
	m_TableDtm.setSize(b.getWidth(), b.getHeight() / 2);
	m_TableRange.setTopLeftPosition(0, b.getHeight() / 2);
	m_TableRange.setSize(b.getWidth()/2, b.getHeight() / 2);
	m_Mode.setTopLeftPosition(b.getWidth() / 2, b.getHeight() / 2);
	m_Mode.setSize(b.getWidth() / 2, 25);
	m_IsoStep.setTopLeftPosition(b.getWidth() / 2, b.getHeight() / 2 + 30);
	m_IsoStep.setSize(b.getWidth() / 2, 30);
	m_Azimuth.setTopLeftPosition(b.getWidth() / 2, b.getHeight() / 2 + 30);
	m_Azimuth.setSize(b.getWidth() / 4, 100);
	m_Zenith.setTopLeftPosition(3 * b.getWidth() / 4, b.getHeight() / 2 + 30);
	m_Zenith.setSize(b.getWidth() / 4, 100);
}

//==============================================================================
// Gestion des actions
//==============================================================================
void DtmViewer::actionListenerCallback(const juce::String& message)
{
	if (message == "UpdateDtm") {
		repaint();
	}
	if (message == "UpdateRange") {
		m_TableRange.updateContent();
	}
}

//==============================================================================
// Changement de valeur des ComboBox
//==============================================================================
void DtmViewer::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
	if (comboBoxThatHasChanged == &m_Mode) {
		DtmShader::m_Mode = (DtmShader::ShaderMode)(m_Mode.getSelectedId() - 1);
		m_ModelRange.sendActionMessage("UpdateDtm");
		m_IsoStep.setVisible(false);
		m_Azimuth.setVisible(false);
		m_Zenith.setVisible(false);
		if (DtmShader::m_Mode == DtmShader::ShaderMode::Contour)
			m_IsoStep.setVisible(true);
		if (DtmShader::m_Mode == DtmShader::ShaderMode::Free_Shading) {
			m_Azimuth.setVisible(true);
			m_Zenith.setVisible(true);
		}
	}
}

//==============================================================================
// Changement de valeur des sliders
//==============================================================================
void DtmViewer::sliderValueChanged(juce::Slider* slider)
{
	if (slider == &m_IsoStep)	// Changement de l'equidistance des isohypses
		DtmShader::m_dIsoStep = slider->getValue();
	if (slider == &m_Azimuth)
		DtmShader::m_dSolarAzimuth = slider->getValue();
	if (slider == &m_Zenith)
		DtmShader::m_dSolarZenith = slider->getValue();
	m_ModelRange.sendActionMessage("UpdateDtm");
}

//==============================================================================
// Drag&Drop
//==============================================================================
void DtmViewer::itemDropped(const SourceDetails& details)
{
	juce::String message = details.description.toString();
	juce::StringArray T;
	T.addTokens(message, ":", "");
	if (T.size() < 1)
		return;
	int i;
	i = T[0].getIntValue();
	int row = m_TableDtm.getRowContainingPosition(details.localPosition.x, details.localPosition.y);
	m_Base->ReorderDtmLayer(i, row);
	//m_Table.updateContent();
	//m_Table.repaint();
	m_ModelDtm.sendActionMessage("UpdateDtm");
}

bool DtmViewer::isInterestedInDragSource(const SourceDetails& details)
{
	if (details.sourceComponent.get() != &m_TableDtm)
		return false;
	return true;
}