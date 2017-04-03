
#include "BaseElement.h"

BaseElement::BaseElement(std::string id) 
:Thread(id),
m_elementId(id),
m_threaded(true),
m_pull(false)
{
	m_parameters["threaded"] = "yes";
	m_parameters["pull"] = "no";
	m_parameters["fps"] = "60";
}

BaseElement::~BaseElement()
{
	
}

int BaseElement::init()
{
	m_threaded = m_parameters["threaded"] == "yes";
	m_pull = m_parameters["pull"] == "yes";

	m_fps = atof(m_parameters["fps"].c_str());

	return 0;
}

int BaseElement::term()
{
	int count = 0;
	while (isThreadRunning() == true && count < 3)
	{
		stopThread(100);
		count++;
	}

	return 0;
}

