/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include <typeinfo>
#include "JuceDemoHeader.h"

//==============================================================================
struct AlphabeticDemoSorter
{
    static int compareElements (const JuceDemoTypeBase* first, const JuceDemoTypeBase* second)
    {
        return first->name.compare (second->name);
    }
};

JuceDemoTypeBase::JuceDemoTypeBase (const String& demoName)  : name (demoName)
{
    AlphabeticDemoSorter sorter;
    getDemoTypeList().addSorted (sorter, this);
}

JuceDemoTypeBase::~JuceDemoTypeBase()
{
    getDemoTypeList().removeFirstMatchingValue (this);
}

Array<JuceDemoTypeBase*>& JuceDemoTypeBase::getDemoTypeList()
{
    static Array<JuceDemoTypeBase*> list;
    return list;
}

//==============================================================================
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC

// Just add a simple icon to the Window system tray area or Mac menu bar..
class DemoTaskbarComponent  : public SystemTrayIconComponent,
                              private Timer
{
public:
    DemoTaskbarComponent()
    {
        setIconImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
        setIconTooltip ("Juce Demo App!");
    }

    void mouseDown (const MouseEvent&) override
    {
        // On OSX, there can be problems launching a menu when we're not the foreground
        // process, so just in case, we'll first make our process active, and then use a
        // timer to wait a moment before opening our menu, which gives the OS some time to
        // get its act together and bring our windows to the front.

        Process::makeForegroundProcess();
        startTimer (50);
    }

    // This is invoked when the menu is clicked or dismissed
    static void menuInvocationCallback (int chosenItemID, DemoTaskbarComponent*)
    {
        if (chosenItemID == 1)
            JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    void timerCallback() override
    {
        stopTimer();

        PopupMenu m;
        m.addItem (1, "Quit the Juce demo");

        // It's always better to open menus asynchronously when possible.
        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (menuInvocationCallback, this));
    }
};

#endif

bool juceDemoRepaintDebuggingActive = false;

#include "WebCamSource.h"
#include "DisplaySink.h"
#include "VirtualCamSink.h"
#include "resizer.h"

class ContentComponent1 : public Component, private Timer, public Button::Listener
{
	LookAndFeel_V3 lookAndFeelV3;

	WebCamSource* soruce;
	DisplaySink* displaySink;
	VirtualCamSink* vcamSink;
	Resizer* resizer;
	Image* img;

	ScopedPointer<TextButton> button;
	ScopedPointer<ChildProcess> childProcess;

	StringArray strArray;
public:
	ContentComponent1():childProcess(nullptr)
	{
		// set lookAndFeel colour properties
		lookAndFeelV3.setColour(Label::textColourId, Colours::white);
		lookAndFeelV3.setColour(Label::textColourId, Colours::white);
		lookAndFeelV3.setColour(ToggleButton::textColourId, Colours::white);
		LookAndFeel::setDefaultLookAndFeel(&lookAndFeelV3);

		childProcess = new ChildProcess();

		button = new TextButton("Start streaming");
		addAndMakeVisible(button);
		button->setSize(80, 30);
		button->addListener(this);

		//Building Graph
		img = new  Image(Image::RGB, 854, 480, false);

		soruce = new WebCamSource("webcam1");
#ifndef USE_DUMMY_IMAGE
		soruce->setParameter("threaded", "no");
#endif
		displaySink = new DisplaySink("display_sink");
		displaySink->setParameter("threaded", "no");
		displaySink->setParameter("pull", "yes");

		vcamSink = new VirtualCamSink("vcam_sink");

		resizer = new Resizer("resizer");
		resizer->setParameter("threaded", "yes");

		soruce->addOutputElements(resizer);

		resizer->addOutputElements(displaySink);
		resizer->addOutputElements(vcamSink);

		soruce->init();
		resizer->init();
		displaySink->init();
		vcamSink->init();

		displaySink->start();
		vcamSink->start();
		resizer->start();
		soruce->start();

		startTimer(33);

		strArray.add("C:\\Program Files\\Adobe\\Flash Media Live Encoder 3.2\\FMLECmd.exe");
		strArray.add(String("\\p ") + "\"C:\\Program Files\\Adobe\\Flash Media Live Encoder 3.2\\youtube.xml\"" );
	}

	~ContentComponent1()
	{
		stopTimer();

		soruce->stop();
		resizer->stop();
		displaySink->stop();
		vcamSink->stop();

		soruce->term();
		resizer->term();
		displaySink->term();
		vcamSink->term();

		delete soruce; soruce = nullptr;
		delete resizer; resizer = nullptr;
		delete displaySink; displaySink = nullptr;
		delete vcamSink; vcamSink = nullptr;
	}

	void resized() override
	{
		button->setBounds(getWidth() - 100, 10, button->getWidth(), button->getHeight());
		Component::resized();
	}

	void buttonClicked(Button *btn)
	{
		if (btn == button.get())
		{
			if (button->getButtonText() == "Start streaming")
			{
				button->setButtonText("Stop streaming");

				if (childProcess != nullptr && childProcess->isRunning())
				{
					childProcess->kill();
					childProcess->waitForProcessToFinish(200);
				}

				if (childProcess->start(strArray, 0) == false)
					Logger::outputDebugString("Failed to launch FMLE");
			}
			else
			{
				button->setButtonText("Start streaming");

				childProcess->kill();
				childProcess->waitForProcessToFinish(200);
			}
		}
	}

	void timerCallback() override
	{
		repaint();
	}


	void paint(Graphics& g) override
	{
		double startTime = 0.0;

		if (displaySink->getQueueLength() > 0)
		{
			std::shared_ptr<BlockData> imgdata = displaySink->getData();
			if (imgdata.get() != nullptr)
			{
				Image::BitmapData bitmapdata(*img, Image::BitmapData::writeOnly);
				memcpy(bitmapdata.data, imgdata->getData(), imgdata->getSize());
			}
		}

		g.drawImageAt(*img, 5, 5);

		return;
	}

	
};


//==============================================================================
static ScopedPointer<ApplicationCommandManager> applicationCommandManager;
static ScopedPointer<AudioDeviceManager> sharedAudioDeviceManager;

MainAppWindow::MainAppWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      Colours::lightgrey,
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setResizable (true, false);
    setResizeLimits (400, 400, 10000, 10000);

   #if JUCE_IOS || JUCE_ANDROID
    setFullScreen (true);
   #else
    setBounds ((int) (0.1f * getParentWidth()),
               (int) (0.1f * getParentHeight()),
               1000, 600);
   #endif

    contentComponent = new ContentComponent1();
    setContentNonOwned (contentComponent, false);
    setVisible (true);

    // this lets the command manager use keypresses that arrive in our window to send out commands
    addKeyListener (getApplicationCommandManager().getKeyMappings());

   #if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    taskbarIcon = new DemoTaskbarComponent();
   #endif

   #if JUCE_ANDROID
    setOpenGLRenderingEngine();
   #endif

    triggerAsyncUpdate();
}

MainAppWindow::~MainAppWindow()
{
    clearContentComponent();
    contentComponent = nullptr;
    applicationCommandManager = nullptr;
    sharedAudioDeviceManager = nullptr;

   #if JUCE_OPENGL
    openGLContext.detach();
   #endif
}

void MainAppWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}

ApplicationCommandManager& MainAppWindow::getApplicationCommandManager()
{
    if (applicationCommandManager == nullptr)
        applicationCommandManager = new ApplicationCommandManager();

    return *applicationCommandManager;
}

AudioDeviceManager& MainAppWindow::getSharedAudioDeviceManager()
{
    if (sharedAudioDeviceManager == nullptr)
    {
        sharedAudioDeviceManager = new AudioDeviceManager();
        RuntimePermissions::request (RuntimePermissions::recordAudio, runtimePermissionsCallback);
    }

    return *sharedAudioDeviceManager;
}

void MainAppWindow::runtimePermissionsCallback (bool wasGranted)
{
    int numInputChannels = wasGranted ? 2 : 0;
    sharedAudioDeviceManager->initialise (numInputChannels, 2, nullptr, true, String(), nullptr);
}

MainAppWindow* MainAppWindow::getMainAppWindow()
{
    for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
        if (MainAppWindow* maw = dynamic_cast<MainAppWindow*> (TopLevelWindow::getTopLevelWindow (i)))
            return maw;

    return nullptr;
}

void MainAppWindow::handleAsyncUpdate()
{
    // This registers all of our commands with the command manager but has to be done after the window has
    // been created so we can find the number of rendering engines available
    //ApplicationCommandManager& commandManager = MainAppWindow::getApplicationCommandManager();
    //commandManager.registerAllCommandsForTarget (contentComponent);
    //commandManager.registerAllCommandsForTarget (JUCEApplication::getInstance());
}

void MainAppWindow::showMessageBubble (const String& text)
{
    currentBubbleMessage = new BubbleMessageComponent (500);
    getContentComponent()->addChildComponent (currentBubbleMessage);

    AttributedString attString;
    attString.append (text, Font (15.0f));

    currentBubbleMessage->showAt (Rectangle<int> (getLocalBounds().getCentreX(), 10, 1, 1),
                                  attString,
                                  500,  // numMillisecondsBeforeRemoving
                                  true,  // removeWhenMouseClicked
                                  false); // deleteSelfAfterUse
}

static const char* openGLRendererName = "OpenGL Renderer";

StringArray MainAppWindow::getRenderingEngines() const
{
    StringArray renderingEngines;

    if (ComponentPeer* peer = getPeer())
        renderingEngines = peer->getAvailableRenderingEngines();

   #if JUCE_OPENGL
    renderingEngines.add (openGLRendererName);
   #endif

    return renderingEngines;
}

void MainAppWindow::setRenderingEngine (int index)
{
    /*showMessageBubble (getRenderingEngines()[index]);

   #if JUCE_OPENGL
    if (getRenderingEngines()[index] == openGLRendererName
          && contentComponent != nullptr
          && ! contentComponent->isShowingOpenGLDemo())
    {
        openGLContext.attachTo (*getTopLevelComponent());
        return;
    }

    openGLContext.detach();
   #endif

    if (ComponentPeer* peer = getPeer())
        peer->setCurrentRenderingEngine (index);*/
}

void MainAppWindow::setOpenGLRenderingEngine()
{
    setRenderingEngine (getRenderingEngines().indexOf (openGLRendererName));
}

int MainAppWindow::getActiveRenderingEngine() const
{
   #if JUCE_OPENGL
    if (openGLContext.isAttached())
        return getRenderingEngines().indexOf (openGLRendererName);
   #endif

    if (ComponentPeer* peer = getPeer())
        return peer->getCurrentRenderingEngine();

    return 0;
}

Path MainAppWindow::getJUCELogoPath()
{
    return Drawable::parseSVGPath (
        "M250,301.3c-37.2,0-67.5-30.3-67.5-67.5s30.3-67.5,67.5-67.5s67.5,30.3,67.5,67.5S287.2,301.3,250,301.3zM250,170.8c-34.7,0-63,28.3-63,63s28.3,63,63,63s63-28.3,63-63S284.7,170.8,250,170.8z"
        "M247.8,180.4c0-2.3-1.8-4.1-4.1-4.1c-0.2,0-0.3,0-0.5,0c-10.6,1.2-20.6,5.4-29,12c-1,0.8-1.5,1.8-1.6,2.9c-0.1,1.2,0.4,2.3,1.3,3.2l32.5,32.5c0.5,0.5,1.4,0.1,1.4-0.6V180.4z"
        "M303.2,231.6c1.2,0,2.3-0.4,3.1-1.2c0.9-0.9,1.3-2.1,1.1-3.3c-1.2-10.6-5.4-20.6-12-29c-0.8-1-1.9-1.6-3.2-1.6c-1.1,0-2.1,0.5-3,1.3l-32.5,32.5c-0.5,0.5-0.1,1.4,0.6,1.4L303.2,231.6z"
        "M287.4,191.3c-0.1-1.1-0.6-2.2-1.6-2.9c-8.4-6.6-18.4-10.8-29-12c-0.2,0-0.3,0-0.5,0c-2.3,0-4.1,1.9-4.1,4.1v46c0,0.7,0.9,1.1,1.4,0.6l32.5-32.5C287,193.6,287.5,192.5,287.4,191.3z"
        "M252.2,287.2c0,2.3,1.8,4.1,4.1,4.1c0.2,0,0.3,0,0.5,0c10.6-1.2,20.6-5.4,29-12c1-0.8,1.5-1.8,1.6-2.9c0.1-1.2-0.4-2.3-1.3-3.2l-32.5-32.5c-0.5-0.5-1.4-0.1-1.4,0.6V287.2z"
        "M292.3,271.2L292.3,271.2c1.2,0,2.4-0.6,3.2-1.6c6.6-8.4,10.8-18.4,12-29c0.1-1.2-0.3-2.4-1.1-3.3c-0.8-0.8-1.9-1.2-3.1-1.2l-45.9,0c-0.7,0-1.1,0.9-0.6,1.4l32.5,32.5C290.2,270.8,291.2,271.2,292.3,271.2z"
        "M207.7,196.4c-1.2,0-2.4,0.6-3.2,1.6c-6.6,8.4-10.8,18.4-12,29c-0.1,1.2,0.3,2.4,1.1,3.3c0.8,0.8,1.9,1.2,3.1,1.2l45.9,0c0.7,0,1.1-0.9,0.6-1.4l-32.5-32.5C209.8,196.8,208.8,196.4,207.7,196.4z"
        "M242.6,236.1l-45.9,0c-1.2,0-2.3,0.4-3.1,1.2c-0.9,0.9-1.3,2.1-1.1,3.3c1.2,10.6,5.4,20.6,12,29c0.8,1,1.9,1.6,3.2,1.6c1.1,0,2.1-0.5,3-1.3c0,0,0,0,0,0l32.5-32.5C243.7,236.9,243.4,236.1,242.6,236.1z"
        "M213.8,273.1L213.8,273.1c-0.9,0.9-1.3,2-1.3,3.2c0.1,1.1,0.6,2.2,1.6,2.9c8.4,6.6,18.4,10.8,29,12c0.2,0,0.3,0,0.5,0h0c1.2,0,2.3-0.5,3.1-1.4c0.7-0.8,1-1.8,1-2.9v-45.9c0-0.7-0.9-1.1-1.4-0.6l-13.9,13.9L213.8,273.1z"
        "M197.2,353c-4.1,0-7.4-1.5-10.4-5.4l4-3.5c2,2.6,3.9,3.6,6.4,3.6c4.4,0,7.4-3.3,7.4-8.3v-24.7h5.6v24.7C210.2,347.5,204.8,353,197.2,353z"
        "M232.4,353c-8.1,0-15-6-15-15.8v-22.5h5.6v22.2c0,6.6,3.9,10.8,9.5,10.8c5.6,0,9.5-4.3,9.5-10.8v-22.2h5.6v22.5C247.5,347,240.5,353,232.4,353z"
        "M272,353c-10.8,0-19.5-8.6-19.5-19.3c0-10.8,8.8-19.3,19.5-19.3c4.8,0,9,1.6,12.3,4.4l-3.3,4.1c-3.4-2.4-5.7-3.2-8.9-3.2c-7.7,0-13.8,6.2-13.8,14.1c0,7.9,6.1,14.1,13.8,14.1c3.1,0,5.6-1,8.8-3.2l3.3,4.1C280.1,351.9,276.4,353,272,353z"
        "M290.4,352.5v-37.8h22.7v5H296v11.2h16.5v5H296v11.6h17.2v5H290.4z");
};
