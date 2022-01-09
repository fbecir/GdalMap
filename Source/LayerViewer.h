//==============================================================================
// LayerViewer.h
//
// Author : F.Becirspahic
// Date : 19/12/2021
//==============================================================================

#pragma once

#include <JuceHeader.h>
#include "GeoBase.h"

//==============================================================================
// LayerViewerComponent : table pour montrer les proprietes des layers
//==============================================================================
class LayerViewerModel :	public juce::TableListBoxModel, 
													public juce::ChangeListener,
													public juce::Slider::Listener,
													public juce::ActionBroadcaster {
public:
	typedef enum { Name = 1, PenWidth = 2, PenColour = 3, FillColour = 4 } Column;
	LayerViewerModel();

	int getNumRows() override;
	void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
	void cellClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	void sliderValueChanged(juce::Slider* slider) override;

	void SetBase(GeoBase* base) { m_Base = base; }

private:
	GeoBase*							m_Base;
	int										m_ActiveRow;
	int										m_ActiveColumn;
};

//==============================================================================
// LayerViewer : fenetre pour contenir le LayerViewerComponent
//==============================================================================
class LayerViewer : public juce::TableListBox, public juce::ActionListener {
public:
	LayerViewer();

	void SetBase(GeoBase* base) { m_Model.SetBase(base); updateContent(); }
	void SetActionListener(juce::ActionListener* listener) { m_Model.addActionListener(listener); }
	void UpdateColumnName();
	// Gestion des actions
	void actionListenerCallback(const juce::String& message) override;

private:
	LayerViewerModel		m_Model;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LayerViewer)
};