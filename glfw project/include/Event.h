#pragma once

//event class is an abstract base class for concrete event classes to inherit from
class Event
{
public:
	//constructor
	Event() : m_bHandled(false) {}
	//destructor
	virtual ~Event() {};
	//using command creates an alias for const char*
	using DescriptorType = const char*;
	//abstract function to be implemented in derived class, returns the type of Event as a const char*
	virtual DescriptorType type() const = 0;
	//function used to set if an event has been handled
	void Handled() { m_bHandled = true; }
	//function to set that an event has been handled
	bool IsHandled() { return m_bHandled; }

private:
	//variable indicates if event has been processed or not
	bool m_bHandled;
};