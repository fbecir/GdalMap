//==============================================================================
// FeatureViewer.h
//
// Author : F.Becirspahic
// Date : 04/01/2022
//==============================================================================

#pragma once

#include <JuceHeader.h>
#include "GeoBase.h"

//==============================================================================
// FeatureViewerModel : table pour montrer les proprietes des layers
//==============================================================================
class FeatureViewerModel :	public juce::TableListBoxModel {
public:
	typedef enum { Name = 1, Value = 2 } Column;
	FeatureViewerModel() { m_Base = nullptr; }

	int getNumRows() override;
	void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
	void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;

	void SetBase(GeoBase* base) { m_Base = base; }

private:
	GeoBase* m_Base;
};

//==============================================================================
// FeatureViewer : fenetre pour contenir le LayerViewerComponent
//==============================================================================
class FeatureViewer : public juce::DocumentWindow {
public:
	FeatureViewer(const juce::String& name, juce::Colour backgroundColour, int requiredButtons);

	void closeButtonPressed() { setVisible(false); }
	void SetBase(GeoBase* base) { m_Model.SetBase(base); repaint();  m_Table.updateContent(); }
	void UpdateColumnName();

private:
	juce::TableListBox m_Table;
	FeatureViewerModel m_Model;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeatureViewer)
};