//==============================================================================
// SelectionViewer.h
//
// Author : F.Becirspahic
// Date : 03/01/2022
//==============================================================================

#pragma once

#include <JuceHeader.h>
#include "GeoBase.h"

//==============================================================================
// SelectionViewerModel : table pour montrer les proprietes des layers
//==============================================================================
class SelectionViewerModel :	public juce::TableListBoxModel,
															public juce::ActionBroadcaster {
public:
	typedef enum { Name = 1, Index = 2, MinX = 3, MaxX = 4, MinY = 5, MaxY = 6 } Column;
	SelectionViewerModel();

	int getNumRows() override;
	void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
	void cellDoubleClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;

	void SetBase(GeoBase* base) { m_Base = base; }

private:
	GeoBase*							m_Base;
	int										m_ActiveRow;
	int										m_ActiveColumn;
};

//==============================================================================
// SelectionViewer : fenetre pour contenir le LayerViewerComponent
//==============================================================================
class SelectionViewer : public juce::TableListBox, public juce::ActionListener {
public:
	SelectionViewer();

	void SetBase(GeoBase* base) { m_Model.SetBase(base); updateContent();}
	void SetActionListener(juce::ActionListener* listener) { m_Model.addActionListener(listener); }
	void UpdateColumnName();
	// Gestion des actions
	void actionListenerCallback(const juce::String& message) override;

private:
	SelectionViewerModel	m_Model;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectionViewer)
};