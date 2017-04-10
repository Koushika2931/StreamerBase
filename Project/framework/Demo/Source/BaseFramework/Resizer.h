#pragma once

#include "BaseElement.h"


class Resizer : public BaseElement
{
	Graphics* m_graphics;
	Image* m_imageToDraw;
	Image* m_srcHolder;

	unsigned int m_dstWidth, m_dstHeight, m_srcWidth, m_srcHeight;

	size_t m_ouputImageSz;

public:
	Resizer(std::string id);

	~Resizer();

	int init();

	int term();

	int process(std::shared_ptr<BlockData> sharedData);

	virtual ELEMENT_TYPE getElementType() { return BaseElement::TRANSFORM; };

	virtual std::string getElementName() { return "Resizer"; }

	virtual BaseElement* createObject(std::string id)
	{
		return new Resizer(id);
	}
};