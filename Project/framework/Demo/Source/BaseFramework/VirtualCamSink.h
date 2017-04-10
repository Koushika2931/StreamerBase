#pragma once

#include "BaseElement.h"


class SeaStarVCamClient;

class VirtualCamSink : public BaseElement
{
	SeaStarVCamClient *m_vCamClient;

	
public:
	VirtualCamSink(std::string id);

	~VirtualCamSink();

	int init();

	int term();

	int sink(std::shared_ptr<BlockData> sharedData);

	virtual ELEMENT_TYPE getElementType() { return BaseElement::SINK; };

	virtual std::string getElementName() { return "VirtualCamSink"; }

	virtual BaseElement* createObject(std::string id)
	{
		return new VirtualCamSink(id);
	}
};