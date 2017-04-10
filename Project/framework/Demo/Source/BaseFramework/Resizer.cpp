

#include "Resizer.h"
#include <iostream>

Resizer::Resizer(std::string id) : BaseElement(id),
m_dstWidth(640),
m_dstHeight(480),
m_srcWidth(854),
m_srcHeight(480)
{
	m_parameters["sourceWidth"] = "640";
	m_parameters["sourceHeight"] = "480";
	m_parameters["destWidth"] = "854";
	m_parameters["destHeight"] = "480";
}

Resizer::~Resizer()
{

}

int Resizer::init()
{
	int rv = 0;
	BaseElement::init();

	m_dstWidth = atoi(m_parameters["destWidth"].c_str());
	m_dstHeight = atoi(m_parameters["destHeight"].c_str());

	m_srcWidth = atoi(m_parameters["sourceWidth"].c_str());
	m_srcHeight = atoi(m_parameters["sourceHeight"].c_str());
	
	m_imageToDraw = new Image(Image::RGB, m_dstWidth, m_dstHeight, true);

	m_srcHolder = new Image(Image::RGB, m_srcWidth, m_srcHeight, true);

	m_graphics = new Graphics(*m_imageToDraw);

	m_ouputImageSz = m_dstHeight * m_dstWidth * 3;

	return rv;
}


int Resizer::process(std::shared_ptr<BlockData> sharedData)
{
	int rt = 0;

	//Copy incoming data to image
	Image::BitmapData bitmapdata1(*m_srcHolder, Image::BitmapData::writeOnly);

	memcpy(bitmapdata1.data, sharedData->getData(), sharedData->getSize());

	m_graphics->drawImage(*m_srcHolder, Rectangle<float>(0, 0, m_dstWidth, m_dstHeight), RectanglePlacement::fillDestination);
	
	Image::BitmapData bitmapdata2(*m_imageToDraw, Image::BitmapData::readOnly);

	std::shared_ptr<BlockData> sharedData1 = std::make_shared<BlockData>(m_elementId, m_ouputImageSz);

	memcpy(sharedData1->getData(), bitmapdata2.data, m_ouputImageSz);

	send(sharedData1);

	return rt;
}

int Resizer::term()
{
	
	return BaseElement::term();
}