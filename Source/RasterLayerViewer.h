//==============================================================================
// RasterLayerViewer.h
//
// Created: 10 Jan 2022 9:58:48pm
// Author:  FBecirspahic
//==============================================================================

#pragma once
#include <JuceHeader.h>
#include "GeoBase.h"

//==============================================================================
// RasterLayerViewerModel : table pour montrer les proprietes des layers
//==============================================================================
class RasterLayerViewerModel : public juce::TableListBoxModel,
	public juce::ChangeListener,
	public juce::Slider::Listener,
	public juce::ActionBroadcaster {
public:
	typedef enum { Visibility = 1, Name = 2, Opacity = 3, PenColour = 4, FillColour = 5 } Column;
	RasterLayerViewerModel();

	int getNumRows() override;
	void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
	void cellClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;
	juce::var getDragSourceDescription(const juce::SparseSet<int>& selectedRows) override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	void sliderValueChanged(juce::Slider* slider) override;

	void SetBase(GeoBase* base) { m_Base = base; }

private:
	GeoBase* m_Base;
	int										m_ActiveRow;
	int										m_ActiveColumn;
};

//==============================================================================
// RasterLayerViewer : fenetre pour contenir
//==============================================================================
class RasterLayerViewer : public juce::Component,
	public juce::ActionListener,
	public juce::DragAndDropTarget,
	public juce::DragAndDropContainer {
public:
	RasterLayerViewer();

	void SetBase(GeoBase* base) { m_Base = base;  m_Model.SetBase(base); m_Table.updateContent(); }
	void SetActionListener(juce::ActionListener* listener) { m_Model.addActionListener(listener); }
	void UpdateColumnName();
	void resized() override { auto b = getLocalBounds(); m_Table.setSize(b.getWidth(), b.getHeight()); }
	// Gestion des actions
	void actionListenerCallback(const juce::String& message) override;

	// Drag&Drop
	void itemDropped(const SourceDetails& details) override;
	bool isInterestedInDragSource(const SourceDetails& details) override;
	void itemDragEnter(const SourceDetails& ) override { ; }
	void itemDragMove(const SourceDetails& ) override { ; }
	void itemDragExit(const SourceDetails&) override { ; }

private:
	GeoBase* m_Base;
	juce::TableListBox				m_Table;
	RasterLayerViewerModel		m_Model;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RasterLayerViewer)
};