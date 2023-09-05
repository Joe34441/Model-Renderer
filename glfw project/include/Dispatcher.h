#pragma once

#include "Observer.h"
#include "Event.h"

#include <map>
#include <list>
#include <functional>
#include <typeinfo>
#include <typeindex>


//typedefine for std::list<Observer*> objects to improve code readability
typedef std::list<Observer*> ObserverList;
//dispatcher class, responsible for handling events and notifying any observers of a particular event this
//could be improved to utilise a queue or ring buffer to process events at the start of the game loop
//at present all events are handled immediately
class Dispatcher
{
public:
	static Dispatcher* GetInstance() { return m_instance; }
	static Dispatcher* CreateInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new Dispatcher();
		}
		return m_instance;
	}
	static void DestroyInstance()
	{
		if (m_instance)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

	//subscription function to subscribe observers to an event, member function pointer implementation
	template<typename T, typename ConcreteEvent>
	void Subscribe(T* a_instance, void(T::* memberFunction)(ConcreteEvent*))
	{
		//get a list of observers from the subscribers map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		//if observers is null there are no observers for this event yet
		if (observers == nullptr)
		{
			//create new list for event type and add this into the subscribers map
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
		}
		//push a new member observer into the observers list from the subscribers map
		observers->push_back(new MemberObserver<T, ConcreteEvent>(a_instance, memberFunction));
	}

	//subscribe method for global functions to become event subscribers
	template<typename ConcreteEvent>
	void Subscribe(void(*Function)(ConcreteEvent*))
	{
		//get a list of observers from the subscriber map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		if (observers == nullptr)
		{
			//create new list for event type and add this into the subscriber map
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
		}
		//push a new member observer into the observers list from the subscriber map
		observers->push_back(new GlobalObserver<ConcreteEvent>(Function));
	}

	//function to publish an event has occured and notify all subscribers to the event
	template<typename ConcreteEvent>
	void Publish(ConcreteEvent* e, bool cleanup = false)
	{
		//get the list of observers from the map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		if (observers == nullptr) { return; }
		//for each observer notify them that the event has occured
		for(auto& handler : *observers)
		{
			handler->exec(e);
			//if an event has been handled by a subscriber then we do not need to keep notifying other subscribers
			if (static_cast<Event*>(e)->IsHandled())
			{
				break;
			}
		}
		//as we could pass through "new ConcreteEvent()" we should call delete if needed
		if (cleanup) { delete e; }
	}

protected:
	//keep the constructors protected and ise this dispatcher class as a singleton object
	Dispatcher() {};
	~Dispatcher()
	{
		//clean up the subscriber map
		for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
		{
			ObserverList* obs = it->second;
			for (auto o = obs->begin(); o != obs->end(); ++o)
			{
				delete (*o);
				(*o) = nullptr;
			}
			delete obs;
		}
	};


private:
	static Dispatcher* m_instance;
	//a hash map of observers uses typeid of Event class as an index into the map
	std::map<std::type_index, ObserverList*> m_subscribers;
};