#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <queue>

#include "../JuceLibraryCode/JuceHeader.h"

class MetaData
{
public:
	MetaData() {}

	virtual ~MetaData() {}
};

class BlockData
{
	void* m_data;
	std::string m_id;
	size_t m_size;

	MetaData* m_metadata;

	BlockData(const BlockData&) {}
		
	BlockData& operator=(const BlockData&) {}

public:
	BlockData(std::string id, int sz)
		:m_metadata(0),
		m_id(id),
		m_size(sz)
	{
		m_data = new char[sz];
	}

	~BlockData()
	{
		if (m_data != nullptr) delete m_data; m_data = nullptr;
	}

	void* getData() { return m_data; }

	size_t getSize() { return m_size; }
};

class BaseElement : private Thread
{
protected:
	std::string m_elementId;

	std::map<std::string, std::string> m_parameters;

	std::vector<BaseElement*> m_outputElements;

	bool m_threaded;
	bool m_pull;
	float m_fps;

	std::queue<std::shared_ptr<BlockData>> m_dataQueue;

	BaseElement(const BaseElement&) : Thread(m_elementId){}

	BaseElement& operator=(const BaseElement&) {}
public:
	enum ELEMENT_TYPE { SOURCE = 0, TRANSFORM, SINK };

	BaseElement(std::string id);

	virtual ~BaseElement();

	virtual int init();

	virtual int term();

	std::string getElementId() { return m_elementId; }

	virtual ELEMENT_TYPE getElementType() = 0;

	virtual std::string getElementName() = 0;

	virtual std::string getElementDescription() { return std::string();  };

	virtual BaseElement* createObject(std::string id) = 0;

	bool addOutputElements(BaseElement* element)
	{
		m_outputElements.push_back(element);
		return true;
	}

	bool setParameter(std::string key, std::string val)
	{
		if (m_parameters.find(key) != m_parameters.end())
		{
			m_parameters[key] = val;
		}
		return false;
	}

	virtual int start()
	{
		if (m_threaded)
			startThread();
		
		return 0;
	}

	virtual int stop()
	{
		signalThreadShouldExit();
		return 0;
	}

	virtual void run()
	{
		int64 startTime = Time::currentTimeMillis();
		while (threadShouldExit() == false)
		{
			if (getElementType() == TRANSFORM)
			{
				if (m_dataQueue.size() > 0) {
					std::shared_ptr<BlockData> sharedData = m_dataQueue.front();
					m_dataQueue.pop();
					process(sharedData);
				}
			}
			else if (getElementType() == SOURCE)
			{
				float desiredSleep = 30;
				create();
				/*if (Time::currentTimeMillis() - startTime >= 1000/m_fps)
				{
					startTime = Time::currentTimeMillis();
					create();
				}
				else
					desiredSleep = (Time::currentTimeMillis() - startTime) / 2;*/
				Thread::sleep(33);
			}
			else if (getElementType() == SINK)
			{
				if (m_dataQueue.size() > 0) {
					std::shared_ptr<BlockData> sharedData = m_dataQueue.front();
					m_dataQueue.pop();
					sink(sharedData);
				}
			}

			if (m_dataQueue.size() == 0)
				wait(100);
		}
	}

	virtual void send(std::shared_ptr<BlockData> sharedData)
	{
		std::vector<BaseElement*>::iterator iter = m_outputElements.begin();

		while (iter != m_outputElements.end())
		{
			(*iter)->receive(sharedData);
			iter++;
		}
	}

	virtual void receive(std::shared_ptr<BlockData> sharedData)
	{
		if (m_threaded || m_pull)
		{
			if(m_dataQueue.size() <= 20) m_dataQueue.push(sharedData);
			if(m_threaded) notify();
		}
		else
		{
			if (getElementType() == TRANSFORM)
			{
				process(sharedData);
			}
			else if (getElementType() == SINK)
			{
				sink(sharedData);
			}
		}
	}

	virtual int create() { return 0;  }

	virtual int process(std::shared_ptr<BlockData> sharedData) { return 0; }

	virtual int sink(std::shared_ptr<BlockData> sharedData) { return 0; }

	std::shared_ptr<BlockData> getData() { 
		std::shared_ptr<BlockData> data = m_dataQueue.front();
		m_dataQueue.pop();
		return data;
	}

	int getQueueLength() { return m_dataQueue.size(); }
};