//==============================================================================
//    SelTreeViewer.cpp
//
//  Created: 14 Mar 2022 3:50:54pm
//  Author:  FBecirspahic
//==============================================================================

#include <JuceHeader.h>
#include "SelTreeViewer.h"



bool SelTreeItem::mightContainSubItems()
{
  if (m_Base != nullptr)
    return true;
  if (m_Feature.IdLayer() >= 0)
    return true;
  return false;
}

void SelTreeItem::itemOpennessChanged(bool isNowOpen)
{
  if (m_Base == nullptr)
    return;
  if (isNowOpen) {
    if (m_Feature.IdLayer() < 0) { // Racine
      for (size_t i = 0; i < m_Base->GetSelectionCount(); i++) {
        SelTreeItem* item = new SelTreeItem(m_Base, m_Base->GetSelection(i));
        addSubItem(item);
      }
      return;
    }
    else {  // Feature
      m_Base->SelectFeatureFields(m_Feature.IdLayer(), m_Feature.Id());
      for (int i = 0; i < m_Base->GetFieldCount(); i++) {
        SelTreeItem* item = new SelTreeItem(m_Base->GetFieldName(i), m_Base->GetFieldValue(i));
        addSubItem(item);
      }
      return;
    }
  }
  else
    clearSubItems();
}

void SelTreeItem::paintItem(juce::Graphics& g, int width, int height)
{
  g.setFont(15.0f);
  if ((m_Feature.IdLayer() >= 0) && (m_Base != nullptr)) {
    OGRLayer* ogrLayer = m_Base->GetOGRLayer(m_Feature.IdLayer());
    if (ogrLayer == nullptr)
      return;
    if (isSelected())
      g.fillAll(juce::Colours::teal);
    g.drawText(juce::String(ogrLayer->GetName()), 4, 0, width - 4, height, juce::Justification::centredLeft, true);
    return;
  }
  juce::String text = m_AttName;
  g.setColour(juce::Colours::coral);
  g.drawText(text, 4, 0, width / 4 - 4, height, juce::Justification::centredLeft, true);
  text = m_AttValue;
  g.setColour(juce::Colours::lightyellow);
  g.drawText(text, 4 + width / 4, 0, 3 * width / 4 - 4, height, juce::Justification::centredLeft, true);
}

void SelTreeItem::itemClicked(const juce::MouseEvent&)
{
  if ((m_Feature.IdLayer() >= 0) && (m_Base != nullptr)) {
    SelTreeViewer* viewer = static_cast<SelTreeViewer*>(getOwnerView());
    viewer->sendActionMessage("SelectFeature:" + juce::String(m_Feature.Id()) + ":" + juce::String(m_Feature.IdLayer()));
  }
}

void SelTreeItem::itemDoubleClicked(const juce::MouseEvent&)
{
  if ((m_Feature.IdLayer() >= 0) && (m_Base != nullptr)) {
    SelTreeViewer* viewer = static_cast<SelTreeViewer*>(getOwnerView());
    OGREnvelope env = m_Feature.Envelope();
    OGRLayer* layer = m_Base->GetOGRLayer(m_Feature.IdLayer());
    if (layer == nullptr)
      return;
    env = GeoBase::ConvertEnvelop(env, layer->GetSpatialRef(), m_Base->SpatialRef());
    viewer->sendActionMessage("ZoomEnvelope:" + juce::String(env.MinX, 2) + ":" + juce::String(env.MaxX, 2) + ":" +
      juce::String(env.MinY, 2) + ":" + juce::String(env.MaxY, 2));
  }
}

//==============================================================================
SelTreeViewer::SelTreeViewer()
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  m_rootItem.reset(new SelTreeItem);
  setRootItem(m_rootItem.get());

}

SelTreeViewer::~SelTreeViewer()
{
  setRootItem(nullptr);
}

void SelTreeViewer::SetBase(GeoBase* base)
{
  m_rootItem.get()->SetBase(base);
  repaint();
  setRootItemVisible(false);

}
