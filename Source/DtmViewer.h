//==============================================================================
// DtmViewer.h
// Created: 21 Jan 2022 10:09:55am
// Author:  FBecirspahic
//==============================================================================

#pragma once

#include <JuceHeader.h>
#include "GeoBase.h"

//==============================================================================
// DtmViewerModel : modele pour montrer les proprietes des MNT
//==============================================================================
class DtmViewerModel : public juce::TableListBoxModel,
	public juce::ChangeListener,
	public juce::Slider::Listener,
	public juce::ActionBroadcaster {
public:
	typedef enum { Visibility = 1, Name = 2, Opacity = 3 } Column;
	DtmViewerModel();

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
// DtmRangeModel : modele pour montrer les plages d'altitudes
//==============================================================================
class DtmRangeModel : public juce::TableListBoxModel,
	public juce::ChangeListener,
	public juce::Slider::Listener,
	public juce::ActionBroadcaster {
public:
	typedef enum { Altitude = 1, Colour = 2 } Column;
	DtmRangeModel() { m_ActiveRow = m_ActiveColumn = -1; }

	int getNumRows() override;
	void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
	void cellClicked(int rowNumber, int columnId, const juce::MouseEvent&) override;
	
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;
	void sliderValueChanged(juce::Slider* slider) override;

private:
	int										m_ActiveRow;
	int										m_ActiveColumn;
};

//==============================================================================
// DtmViewer : fenetre pour voir les caracteristiques des MNT
//==============================================================================
class DtmViewer : public juce::Component,
	public juce::ActionListener,
	public juce::ComboBox::Listener,
	public juce::Slider::Listener,
	public juce::DragAndDropTarget,
	public juce::DragAndDropContainer {
public:
	DtmViewer();

	void SetBase(GeoBase* base) { m_Base = base;  m_ModelDtm.SetBase(base); m_TableDtm.updateContent(); }
	void SetActionListener(juce::ActionListener* listener) 
		{ m_ModelDtm.addActionListener(listener); m_ModelRange.addActionListener(listener); }
	void UpdateColumnName();
	void resized() override;
	// Gestion des actions
	void actionListenerCallback(const juce::String& message) override;
	void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
	void sliderValueChanged(juce::Slider* slider) override;

	// Drag&Drop
	void itemDropped(const SourceDetails& details) override;
	bool isInterestedInDragSource(const SourceDetails& details) override;
	void itemDragEnter(const SourceDetails&) override { ; }
	void itemDragMove(const SourceDetails&) override { ; }
	void itemDragExit(const SourceDetails&) override { ; }

private:
	GeoBase* m_Base;
	juce::TableListBox	m_TableDtm;
	DtmViewerModel			m_ModelDtm;
	juce::TableListBox	m_TableRange;
	DtmRangeModel				m_ModelRange;
	juce::ComboBox			m_Mode;
	juce::Slider				m_IsoStep;
	juce::Slider				m_Azimuth;
	juce::Slider				m_Zenith;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DtmViewer)
};