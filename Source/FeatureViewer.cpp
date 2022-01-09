//==============================================================================
// FeatureViewer.h
//
// Author : F.Becirspahic
// Date : 04/01/2022
//==============================================================================

#include "FeatureViewer.h"

//==============================================================================
// Nombre de lignes de la table
//==============================================================================
int FeatureViewerModel::getNumRows()
{
	if (m_Base == nullptr)
		return 0;
	return m_Base->GetFieldCount();
}

// This is overloaded from TableListBoxModel, and should fill in the background of the whole row
void FeatureViewerModel::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	g.setColour(juce::Colours::lightblue);
	if (rowIsSelected)
		g.drawRect(g.getClipBounds());
	g.setColour(juce::Colours::black);
}

void FeatureViewerModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (m_Base == nullptr)
		return;
	if (rowNumber >= m_Base->GetFieldCount())
		return;
	switch(columnId) {
	case Column::Name :
		g.drawText(m_Base->GetFieldName(rowNumber), 0, 0, width, height, juce::Justification::centredLeft);
		break;
	case Column::Value :
		g.drawText(m_Base->GetFieldValue(rowNumber), 0, 0, width, height, juce::Justification::centredLeft);
		break;
	}
}

//==============================================================================
// Double-clic dans une cellule
//==============================================================================
void FeatureViewerModel::cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent& event)
{
	
}

//==============================================================================
// SelectionViewer : constructeur
//==============================================================================
FeatureViewer::FeatureViewer(const juce::String& name, juce::Colour backgroundColour, int requiredButtons)
	: juce::DocumentWindow(name, backgroundColour, requiredButtons)
{
	// Ajout d'une bordure
	m_Table.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
	m_Table.setOutlineThickness(1);
	// Ajout des colonnes
	m_Table.getHeader().addColumn(juce::translate("Name"), FeatureViewerModel::Column::Name, 150);
	m_Table.getHeader().addColumn(juce::translate("Value"), FeatureViewerModel::Column::Value, 350);
	m_Table.setSize(600, 200);

	m_Table.setModel(&m_Model);

	setContentOwned(&m_Table, true);
	setResizable(true, true);
	setAlwaysOnTop(true);
}

//==============================================================================
// FeatureViewer : mise a jour du nom des colonnes (pour la traduction)
//==============================================================================
void FeatureViewer::UpdateColumnName()
{
	m_Table.getHeader().setColumnName(FeatureViewerModel::Column::Name, juce::translate("Name"));
	m_Table.getHeader().setColumnName(FeatureViewerModel::Column::Value, juce::translate("Value"));
}