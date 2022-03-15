//==============================================================================
//    SelTreeViewer.h
//
//    Created: 14 Mar 2022 3:50:54pm
//    Author:  FBecirspahic
//==============================================================================

#pragma once

#include <JuceHeader.h>
#include "GeoBase.h"

class SelTreeItem : public juce::TreeViewItem {
public:
  SelTreeItem() { m_Base = nullptr; }
  SelTreeItem(GeoBase* base, GeoBase::Feature feature) { m_Base = base; m_Feature = feature; }
  SelTreeItem(std::string name, std::string value) { m_Base = nullptr; m_AttName = name; m_AttValue = value; }

  void SetBase(GeoBase* base) { clearSubItems(); m_Base = base; setOpen(true); }

  bool mightContainSubItems() override;
  void itemOpennessChanged(bool isNowOpen) override;
  void paintItem(juce::Graphics& g, int width, int height) override;
  void itemClicked(const juce::MouseEvent&) override;
  void itemDoubleClicked(const juce::MouseEvent&) override;

private:
  GeoBase* m_Base;
  GeoBase::Feature m_Feature;
  juce::String m_AttName;
  juce::String m_AttValue;
};

class SelTreeViewer  : public juce::TreeView, public juce::ActionBroadcaster {
public:
  SelTreeViewer();
  ~SelTreeViewer() override;

  void SetBase(GeoBase* base);

private:
  std::unique_ptr<SelTreeItem> m_rootItem;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelTreeViewer)
};
