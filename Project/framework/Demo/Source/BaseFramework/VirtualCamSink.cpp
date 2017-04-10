

#include "VirtualCamSink.h"
#include "..\..\..\SeaStarVCam\SeaStarVCamClient\SeaStarVCamClient.h"
#include <iostream>

VirtualCamSink::VirtualCamSink(std::string id) : BaseElement(id)
{
	m_parameters["webcamname"] = "0";
}

VirtualCamSink::~VirtualCamSink()
{

}

int VirtualCamSink::init()
{
	int rv = 0;
	BaseElement::init();

	m_vCamClient = new SeaStarVCamClient();

	long shMemLen = 854 * 480 * 3;

	if (m_vCamClient->init(m_parameters["webcamname"].c_str(), shMemLen))
		rv = 0;
	else
	{
		std::cout << "CoralVCamSink: SeaStarVCamClient initialization failed";
		rv = -1;
	}
		

	return rv;
}


int VirtualCamSink::sink(std::shared_ptr<BlockData> sharedData)
{
	bool rt = m_vCamClient->sink((unsigned char*)sharedData->getData(), (long)sharedData->getSize());

	std::cout << "Writing to Virtual Camera";

	return rt;
}

int VirtualCamSink::term()
{
	if (m_vCamClient)
	{
		m_vCamClient->term();
		delete m_vCamClient;
		m_vCamClient = 0;
	}

	return BaseElement::term();
}