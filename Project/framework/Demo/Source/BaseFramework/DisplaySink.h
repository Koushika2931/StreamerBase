#pragma once

#include "BaseElement.h"


class DisplaySink : public BaseElement
{
public:
	DisplaySink(std::string id): BaseElement(id)
	{

	}

	~DisplaySink()
	{

	}

	int init()
	{
		return BaseElement::init();
	}

	int term() 
	{
		return BaseElement::term();
	}

	virtual ELEMENT_TYPE getElementType() { return BaseElement::SINK; };

	virtual std::string getElementName() { return "DisplaySink"; }

	virtual BaseElement* createObject(std::string id)
	{
		return new DisplaySink(id);
	}
};