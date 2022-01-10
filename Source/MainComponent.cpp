//==============================================================================
// MainComponent.cpp
//
// Author : F.Becirspahic
// Date : 10/12/2021
//==============================================================================

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	GDALAllRegister();	// Registration des drivers

	m_MapView.reset(new MapView);
	addAndMakeVisible(m_MapView.get());
	m_MapView.get()->SetBase(&m_Base);
	m_MapView.get()->addActionListener(this);

  m_MenuBar.reset(new juce::MenuBarComponent(this));
  addAndMakeVisible(m_MenuBar.get());
  setApplicationCommandManagerToWatch(&m_CommandManager);
  m_CommandManager.registerAllCommandsForTarget(this);
	
	m_LayerViewer.reset(new LayerViewer);
	addAndMakeVisible(m_LayerViewer.get());
	m_LayerViewer.get()->SetBase(&m_Base);
	m_LayerViewer.get()->SetActionListener(this);

	m_RasterLayerViewer.reset(new RasterLayerViewer);
	addAndMakeVisible(m_RasterLayerViewer.get());
	m_RasterLayerViewer.get()->SetBase(&m_Base);
	m_RasterLayerViewer.get()->SetActionListener(this);

	m_SelectionViewer.reset(new SelectionViewer);
	addAndMakeVisible(m_SelectionViewer.get());
	m_SelectionViewer.get()->SetBase(&m_Base);
	m_SelectionViewer.get()->SetActionListener(this);
	
	m_FeatureViewer.reset(new FeatureViewer("Feature", juce::Colours::grey, juce::DocumentWindow::allButtons));
	m_FeatureViewer.get()->setVisible(false);

	m_Panel.reset(new juce::ConcertinaPanel);
	addAndMakeVisible(m_Panel.get());
	m_Panel.get()->addPanel(-1, m_LayerViewer.get(), false);
	m_Panel.get()->addPanel(-1, m_RasterLayerViewer.get(), false);
	m_Panel.get()->addPanel(-1, m_SelectionViewer.get(), false);
	m_Panel.get()->setCustomPanelHeader(m_Panel.get()->getPanel(0), new juce::TextButton(juce::translate("Layers")), true);
	m_Panel.get()->setCustomPanelHeader(m_Panel.get()->getPanel(1), new juce::TextButton(juce::translate("Images")), true);
	m_Panel.get()->setCustomPanelHeader(m_Panel.get()->getPanel(2), new juce::TextButton(juce::translate("Selection")), true);

	// set up the layout and resizer bars..
	m_VerticalLayout.setItemLayout(0, -0.2, -1.0, -0.65); 
	m_VerticalLayout.setItemLayout(1, 8, 8, 8);           // the vertical divider drag-bar thing is always 8 pixels wide
	m_VerticalLayout.setItemLayout(2, -0.35, -0.6, -0.35);  
	
	m_VerticalDividerBar.reset(new juce::StretchableLayoutResizerBar(&m_VerticalLayout, 1, true));
	addAndMakeVisible(m_VerticalDividerBar.get());

  // this lets the command manager use keypresses that arrive in our window to send out commands
  addKeyListener(m_CommandManager.getKeyMappings());

  setSize (800, 600);

	juce::RuntimePermissions::request(juce::RuntimePermissions::readExternalStorage,
		[](bool granted)
		{
			if (!granted)
			{
				juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
					"Permissions warning",
					"External storage access permission not granted, some files"
					" may be inaccessible.");
			}
		});
}

MainComponent::~MainComponent()
{
	
}

void MainComponent::resized()
{
	auto b = getLocalBounds();
	m_MenuBar.get()->setBounds(b.removeFromTop(juce::LookAndFeel::getDefaultLookAndFeel()
		.getDefaultMenuBarHeight()));
	juce::Rectangle<int> R;
	R.setLeft(0);
	R.setRight(b.getRight());
	R.setTop(juce::LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight());
	R.setBottom(b.getBottom());
	//m_MapView->setBounds(R);

	Component* vcomps[] = { m_MapView.get(), m_VerticalDividerBar.get(), m_Panel.get()};

	m_VerticalLayout.layOutComponents(vcomps, 3,
		R.getX(), R.getY(), R.getWidth(), R.getHeight(),
		false,     // lay out side-by-side
		true);     // resize the components' heights as well as widths

}

//==============================================================================
// Nom des menus
//==============================================================================
juce::StringArray MainComponent::getMenuBarNames()
{
	return { juce::translate("File"), juce::translate("Edit"), juce::translate("Layers"), juce::translate("View"), "?"};
}

juce::PopupMenu MainComponent::getMenuForIndex(int menuIndex, const juce::String& menuName)
{
	juce::PopupMenu menu;

	if (menuIndex == 0)	// File
	{
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuNew);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuOpenImage);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuOpenVector);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuOpenFolder);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuQuit);
	}
	else if (menuIndex == 1) // Edit
	{
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuTest);
	}
	else if (menuIndex == 2) // Layers
	{
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuAddVectorLayer);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuAddRasterLayer);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuZoomTotal);
	}
	else if (menuIndex == 3)
	{
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuShowLayerViewer);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuShowSelectionViewer);
		menu.addCommandItem(&m_CommandManager, CommandIDs::menuShowFeatureViewer);
	}
	else if (menuIndex == 4) // Help
	{
		menu.addCommandItem(&m_CommandManager, CommandIDs::gdalAbout);
	}

	return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{

}

juce::ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
	return NULL;
}

//==============================================================================
//
//==============================================================================
void MainComponent::getAllCommands(juce::Array<juce::CommandID>& c)
{
	juce::Array<juce::CommandID> commands{ CommandIDs::menuNew, CommandIDs::menuOpenImage, CommandIDs::menuOpenVector, CommandIDs::menuOpenFolder,
		CommandIDs::menuQuit, CommandIDs::menuUndo, CommandIDs::menuAddVectorLayer, CommandIDs::menuAddRasterLayer, CommandIDs::menuZoomTotal,
		CommandIDs::menuTest, CommandIDs::menuShowLayerViewer, CommandIDs::menuShowSelectionViewer, 
		CommandIDs::menuShowFeatureViewer, CommandIDs::gdalAbout };
	c.addArray(commands);
}

//==============================================================================
//
//==============================================================================
void MainComponent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
	switch (commandID)
	{
	case CommandIDs::menuNew:
		result.setInfo(juce::translate("New Window"), "Places the menu bar inside the application window", "Menu", 0);
		result.addDefaultKeypress('n', juce::ModifierKeys::ctrlModifier);
		break;
	case CommandIDs::menuOpenImage:
		result.setInfo(juce::translate("Open Image"), "Open an image", "Menu", 0);
		result.addDefaultKeypress('o', juce::ModifierKeys::ctrlModifier);
		break;
	case CommandIDs::menuOpenVector:
		result.setInfo(juce::translate("Open Vector"), "Open an vector file", "Menu", 0);
		//result.addDefaultKeypress('o', juce::ModifierKeys::ctrlModifier);
		break;
	case CommandIDs::menuOpenFolder:
		result.setInfo(juce::translate("Open Folder"), "Open a folder", "Menu", 0);
		result.addDefaultKeypress('b', juce::ModifierKeys::ctrlModifier);
		break;
	case CommandIDs::menuQuit:
		result.setInfo(juce::translate("Quit"), "Uses a burger menu", "Menu", 0);
		result.addDefaultKeypress('q', juce::ModifierKeys::ctrlModifier);
		break;
	case CommandIDs::menuAddVectorLayer:
		result.setInfo(juce::translate("Add vector data"), juce::translate("Add vector data"), "Menu", 0);
		break;
	case CommandIDs::menuAddRasterLayer:
		result.setInfo(juce::translate("Add raster data"), juce::translate("Add raster data"), "Menu", 0);
		break;
	case CommandIDs::menuZoomTotal:
		result.setInfo(juce::translate("Zoom total"), juce::translate("Zoom total"), "Menu", 0);
		break;
	case CommandIDs::menuShowLayerViewer:
		result.setInfo(juce::translate("View Layer Viewer"), juce::translate("View Layer Viewer"), "Menu", 0);
		if (m_LayerViewer.get() != nullptr)
			result.setTicked(m_LayerViewer.get()->isVisible());
		break;
	case CommandIDs::menuShowSelectionViewer:
		result.setInfo(juce::translate("View Selection Viewer"), juce::translate("View Selection Viewer"), "Menu", 0);
		if (m_SelectionViewer.get() != nullptr)
			result.setTicked(m_SelectionViewer.get()->isVisible());
		break;
	case CommandIDs::menuShowFeatureViewer:
		result.setInfo(juce::translate("View Feature Viewer"), juce::translate("View Feature Viewer"), "Menu", 0);
		if (m_FeatureViewer.get() != nullptr)
			result.setTicked(m_FeatureViewer.get()->isVisible());
		break;
	case CommandIDs::gdalAbout:
		result.setInfo(juce::translate("About GdalMap"), juce::translate("About GdalMap"), "Menu", 0);
		break;
	default:
		result.setInfo("Test", "Test menu", "Menu", 0);
		break;
	}
}

//==============================================================================
// Resultat d'une commande
//==============================================================================
bool MainComponent::perform(const InvocationInfo& info)
{
	switch (info.commandID)
	{
	case CommandIDs::menuNew:
		Clear();
		break;
	case CommandIDs::menuOpenImage:
		OpenFile();
		break;
	case CommandIDs::menuOpenVector:
		OpenVector();
		break;
	case CommandIDs::menuOpenFolder:
		OpenFolder();
		break;
	case CommandIDs::menuQuit:
		juce::JUCEApplication::quit();
		break;
	case CommandIDs::menuTest:
		Test();
		break;
	case CommandIDs::menuAddVectorLayer:
		AddVectorLayer();
		break;
	case CommandIDs::menuAddRasterLayer:
		AddRasterLayer();
		break;
	case CommandIDs::menuZoomTotal:
		m_MapView.get()->ZoomWorld();
		break;
	case CommandIDs::menuShowLayerViewer:
		if (m_LayerViewer.get() == nullptr)
			return false;
		m_LayerViewer.get()->setVisible(!m_LayerViewer.get()->isVisible());
		break;
	case CommandIDs::menuShowSelectionViewer:
		if (m_SelectionViewer.get() == nullptr)
			return false;
		m_SelectionViewer.get()->setVisible(!m_SelectionViewer.get()->isVisible());
		break;
	case CommandIDs::menuShowFeatureViewer:
		if (m_FeatureViewer.get() == nullptr)
			return false;
		m_FeatureViewer.get()->setVisible(!m_FeatureViewer.get()->isVisible());
		break;
	case CommandIDs::gdalAbout:
		AboutGdalMap();
		break;
	default:
		return false;
	}

	return true;
}

//==============================================================================
// Gestion des actions
//==============================================================================
void MainComponent::actionListenerCallback(const juce::String& message)
{
	if (message == "UpdateRepres") {
		if (m_MapView != nullptr)
			m_MapView.get()->RenderMap();
		return;
	}
	if (message == "UpdateSelectFeatures") {
		m_SelectionViewer.get()->SetBase(&m_Base);
		if (m_Base.GetSelectionCount() >= 1) {
			GeoBase::Feature geoFeature = m_Base.GetSelection(m_Base.GetSelectionCount()-1);
			m_Base.SelectFeatureFields(geoFeature.IdLayer(), geoFeature.Id());
			m_FeatureViewer.get()->SetBase(&m_Base);
		}
		m_MapView.get()->RenderMap(false, false, true);
		return;
	}
	
	juce::StringArray T;
	T.addTokens(message, ":", "");

	if (T[0] == "SelectFeature") {
		if (T.size() < 3)
			return;
		GIntBig id = T[1].getLargeIntValue();
		int idLayer = T[2].getIntValue();
		m_Base.SelectFeatureFields(idLayer, id);
		m_FeatureViewer.get()->SetBase(&m_Base);
		return;
	}
	if (T[0] == "ZoomEnvelope") {
		if (T.size() < 5)
			return;
		OGREnvelope env;
		env.MinX = T[1].getDoubleValue();
		env.MaxX = T[2].getDoubleValue();
		env.MinY = T[3].getDoubleValue();
		env.MaxY = T[4].getDoubleValue();
		m_MapView.get()->ZoomEnvelope(env, 10.);
	}
}

//==============================================================================
// Choix d'un repertoire
//==============================================================================
juce::String MainComponent::OpenFolder(juce::String optionName, juce::String mes)
{
	juce::String path, message;
	if (!optionName.isEmpty())
		path = GetAppOption(optionName);
	if (mes.isEmpty())
		message = juce::translate("Choose a directory...");

	juce::FileChooser fc(message, path, "*", false);
	if (fc.browseForDirectory()) {
		auto result = fc.getURLResult();
		auto name = result.isLocalFile() ? result.getLocalFile().getFullPathName() : result.toString(true);

		if (!optionName.isEmpty())
			SaveAppOption(optionName, name);
		return name;
	}
	return "";
}

//==============================================================================
// Choix d'un fichier
//==============================================================================
juce::String MainComponent::OpenFile(juce::String optionName, juce::String mes, juce::String filter)
{
	juce::String path, message, filters;
	if (!optionName.isEmpty())
		path = GetAppOption(optionName);
	if (mes.isEmpty())
		message = juce::translate("Choose a file...");
	if (filter.isEmpty())
		filters = "*";

	juce::FileChooser fc(message, path, filters, false);
	if (fc.browseForFileToOpen()) {
		auto result = fc.getURLResult();
		auto name = result.isLocalFile() ? result.getLocalFile().getFullPathName() : result.toString(true);

		if (!optionName.isEmpty())
			SaveAppOption(optionName, name);
		return name;
	}
	return "";
}


//==============================================================================
// Choix d'un fichier vecteur
//==============================================================================
void MainComponent::OpenVector()
{
	
}

//==============================================================================
// Gestion des options de l'application
//==============================================================================
juce::String MainComponent::GetAppOption(juce::String name)
{
	juce::PropertiesFile::Options options;
	options.applicationName = "GdalMap";
	juce::ApplicationProperties app;
	app.setStorageParameters(options);
	juce::PropertiesFile* file = app.getUserSettings();
	return file->getValue(name);
}

void MainComponent::SaveAppOption(juce::String name, juce::String value)
{
	juce::PropertiesFile::Options options;
	options.applicationName = "GdalMap";
	juce::ApplicationProperties app;
	app.setStorageParameters(options);
	juce::PropertiesFile* file = app.getUserSettings();
	file->setValue(name, value);
	app.saveIfNeeded();
}

//==============================================================================
// Retire tous les jeux de donnees charges
//==============================================================================
void MainComponent::Clear()
{
	m_MapView.get()->StopThread();
	m_Base.Clear();
	m_FeatureViewer.get()->SetBase(&m_Base);
	m_SelectionViewer.get()->SetBase(&m_Base);
	m_LayerViewer.get()->SetBase(&m_Base);
	m_RasterLayerViewer.get()->SetBase(&m_Base);
	m_MapView.get()->SetBase(&m_Base);
}

//==============================================================================
// A propos
//==============================================================================
void MainComponent::AboutGdalMap()
{
	juce::String version = GDALVersionInfo("VERSION_NUM");
	juce::String info = GDALVersionInfo("-version");
	juce::String message = "GDAL Version : " + version + "\n" + info + "\n";
	message += "JUCE Version : " + juce::String(JUCE_MAJOR_VERSION) + "."
		+ juce::String(JUCE_MINOR_VERSION) + "." + juce::String(JUCE_BUILDNUMBER);
	juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
		juce::translate("About GdalMap"), message, "OK");
}

//==============================================================================
// Ajout d'une couche vectorielle
//==============================================================================
bool MainComponent::AddVectorLayer()
{
	juce::String filename = OpenFile("VectorPath");
	if (filename.isEmpty())
		return false;

	if (!m_Base.OpenVectorDataset(filename.getCharPointer())) {
		juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "GdalMap", "Impossible d'ouvrir le fichier " + filename, "OK");
			return false;
	}
	m_Frame = m_Base.GetEnvelope();
	m_MapView.get()->SetFrame(m_Frame);
	m_LayerViewer.get()->SetBase(&m_Base);

	return true;
}

//==============================================================================
// Ajout d'une couche vectorielle
//==============================================================================
bool MainComponent::AddRasterLayer()
{
	juce::String filename = OpenFile("RasterPath");
	if (filename.isEmpty())
		return false;
	juce::File file(filename);
	juce::String name = file.getFileNameWithoutExtension();

	if (!m_Base.OpenRasterDataset(filename.getCharPointer(), name.getCharPointer()) ){
		juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "GdalMap", "Impossible d'ouvrir le fichier " + filename, "OK");
		return false;
	}
	m_Frame = m_Base.GetEnvelope();
	m_MapView.get()->SetFrame(m_Frame);
	m_RasterLayerViewer.get()->SetBase(&m_Base);

	return true;
}

void MainComponent::Test()
{
	/*
	juce::String filename = OpenFile();
	if (filename.isEmpty())
		return;
	juce::File file(filename);
	if (!file.exists())
		return;
	juce::LocalisedStrings::setCurrentMappings(new juce::LocalisedStrings(file, true));
	m_FeatureViewer.get()->UpdateColumnName();
	m_LayerViewer.get()->UpdateColumnName();
	m_SelectionViewer.get()->UpdateColumnName();

	return;*/


	juce::String server = "WMTS:https://maps.wien.gv.at/wmts/1.0.0/WMTSCapabilities.xml,layer=lb";
	server = "WMTS:https://wxs.ign.fr/ortho/geoportail/wmts?EXCEPTIONS=text/xml&&SERVICE=WMTS&VERSION=1.0.0&REQUEST=GetCapabilities,layer=ORTHOIMAGERY.ORTHOPHOTOS";
	//server = "WMTS:https://wxs.ign.fr/ortho/geoportail/wmts?EXCEPTIONS=text/xml&&SERVICE=WMTS&VERSION=1.0.0&REQUEST=GetCapabilities,layer=PCRS.LAMB93";
	//server = "WMTS:https://wxs.ign.fr/orthohisto/geoportail/wmts?EXCEPTIONS=text/xml&&SERVICE=WMTS&VERSION=1.0.0&REQUEST=GetCapabilities,layer=ORTHOIMAGERY.ORTHOPHOTOS.1950-1965";
	//server = "WMTS:https://wxs.ign.fr/orthohisto/geoportail/wmts?EXCEPTIONS=text/xml&&SERVICE=WMTS&VERSION=1.0.0&REQUEST=GetCapabilities,layer=ORTHOIMAGERY.ORTHOPHOTOS2011-2015";
	//server = "TMS:https://tile.openstreetmap.org/${z}/${x}/${y}.png";
	//server = "WMS:http://a.tile.opencyclemap.org/cycle/${z}/${x}/${y}.png";
	if (!m_Base.OpenRasterDataset(server.toStdString().c_str(), "WMTS")) {
		return;
	}
	m_Frame = m_Base.GetEnvelope();
	m_MapView.get()->SetFrame(m_Frame);
	m_LayerViewer.get()->SetBase(&m_Base);
}