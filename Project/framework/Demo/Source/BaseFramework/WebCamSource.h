#pragma once

#include "BaseElement.h"


//#define USE_DUMMY_IMAGE 1

class WebCamSource : public BaseElement,
						public CameraImageListener
{
private:
	CameraDevice* m_camera;
	Random rnd;
	Image* image; //Test Image
public:
	WebCamSource(std::string id) :BaseElement(id), m_camera(nullptr), rnd(10000)
	{
		m_parameters["cam_id"] = "0";

		image = new Image(Image::ARGB, 640, 480, true);
	}

	~WebCamSource()
	{

	}

	BaseElement::ELEMENT_TYPE getElementType() { return BaseElement::SOURCE; };

	std::string getElementName() { return "WebCamSource"; }

	BaseElement* createObject(std::string id)
	{
		return new WebCamSource(id);
	}


	int init()
	{
		BaseElement::init();

		StringArray devices = CameraDevice::getAvailableDevices();
		Logger::writeToLog("Number of Devices :" + String(devices.size()));

#ifndef USE_DUMMY_IMAGE
		m_camera = CameraDevice::openDevice(1, 640, 480);
#endif
		if (m_camera == nullptr) return -1;

		return 0;
	}

	int term()
	{
		if (m_camera != nullptr)
		{
			m_camera->removeListener(this);
			delete m_camera;
			m_camera = nullptr;
		}

		return BaseElement::term();
	}

	virtual int start()
	{
		if (m_camera != nullptr)  m_camera->addListener(this);
		return BaseElement::start();
	}

	int create();

	void imageReceived(const Image &image);
};
