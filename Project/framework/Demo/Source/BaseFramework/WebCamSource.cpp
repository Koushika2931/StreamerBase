
#include "WebCamSource.h"

void WebCamSource::imageReceived(const Image &image)
{
	Image newImage = image.convertedToFormat(Image::PixelFormat::ARGB);
	size_t imageSize = newImage.getHeight() * newImage.getHeight() * 4;

	std::shared_ptr<BlockData> sharedData = std::make_shared<BlockData>(m_elementId, imageSize);

	Image::BitmapData bitmapdata(newImage, Image::BitmapData::readOnly);

	memcpy(sharedData->getData(), bitmapdata.data, imageSize);

	send(sharedData);
}

int WebCamSource::create()
{
	static unsigned char rcol = 0;
	rcol++;
	rcol = rcol % 255;
	Graphics g(*image);
	juce::Colour col(rcol, rcol, rcol);
	g.fillAll(col);

	//Image newImage = image.convertedToFormat(Image::ARGB);
	size_t imageSize = image->getHeight() * image->getHeight() * 4;

	std::shared_ptr<BlockData> sharedData = std::make_shared<BlockData>(m_elementId, imageSize);

	Image::BitmapData bitmapdata(*image, Image::BitmapData::readOnly);

	memcpy(sharedData->getData(), bitmapdata.data, imageSize);

	send(sharedData);

	return 0;
}