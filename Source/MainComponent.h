//==============================================================================
// MainComponent.h
//
// Author : F.Becirspahic
// Date : 10/12/2021
//==============================================================================

#pragma once

#define JUCE_MODAL_LOOPS_PERMITTED 1

#include <JuceHeader.h>
#include "MapView.h"
#include "GeoBase.h"
#include "LayerViewer.h"
#include "FeatureViewer.h"
#include "RasterLayerViewer.h"
#include "DtmViewer.h"
#include "SelTreeViewer.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component,
                      public juce::ApplicationCommandTarget,
                      public juce::MenuBarModel,
                      public juce::ActionListener
{
public:
  // Liste des commandes de l'application
  enum CommandIDs
  {
    menuNew = 1,
    menuOpenImage,
    menuOpenVector,
    menuOpenFolder,
    menuQuit,
    menuUndo,
    menuTranslate,
    menuAddVectorLayer,
    menuAddRasterLayer,
    menuAddDtmLayer,
    menuZoomTotal,
    menuZoomLevel,
    menuTest,
    menuShowSidePanel,
    menuShowFeatureViewer,
    menuAddOSM,
    menuAddGeoportailOrthophoto,
    menuAddGeoportailOrthohisto,
    menuAddGeoportailSatellite,
    menuAddGeoportailCartes,
    gdalAbout
  };

  //==============================================================================
  MainComponent();
  ~MainComponent() override;

  //==============================================================================
  void resized() override;

  // Gestion des menus
  juce::StringArray getMenuBarNames() override;
  juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName) override;
  void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

  // Gestion des commandes
  ApplicationCommandTarget* getNextCommandTarget() override;
  void getAllCommands(juce::Array<juce::CommandID>& c) override;
  void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
  bool perform(const InvocationInfo& info) override;

  // Gestion des actions
  void actionListenerCallback(const juce::String& message) override;

private:
  juce::ApplicationCommandManager m_CommandManager;
  std::unique_ptr<juce::MenuBarComponent> m_MenuBar;
  std::unique_ptr<LayerViewer> m_LayerViewer;
  std::unique_ptr<RasterLayerViewer> m_RasterLayerViewer;
  std::unique_ptr<SelTreeViewer> m_SelTreeViewer;
  std::unique_ptr<FeatureViewer> m_FeatureViewer;
  std::unique_ptr<DtmViewer> m_DtmViewer;
  std::unique_ptr<MapView> m_MapView;

  juce::StretchableLayoutManager m_VerticalLayout;
  std::unique_ptr<juce::StretchableLayoutResizerBar> m_VerticalDividerBar;
  std::unique_ptr <juce::ConcertinaPanel> m_Panel;
 
  GeoBase   m_Base;
 
  juce::String OpenFolder(juce::String optionName = "", juce::String mes = "");
  juce::String OpenFile(juce::String optionName = "", juce::String mes = "", juce::String filter = "");

  juce::String GetAppOption(juce::String name);
  void SaveAppOption(juce::String name, juce::String value);

  void Clear();
  void AboutGdalMap();
  void Translate();

  void OpenVector();

  bool AddVectorLayer();
  bool AddRasterLayer(juce::String rasterfile = "");
  bool AddMultiRasterLayer(juce::String server = "");
  bool AddDtmLayer(juce::String dtmfile = "");
  bool AddOSMServer();

  void Test();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
