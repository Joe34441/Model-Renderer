#pragma once

#include "Event.h"


//observer class is an abstract base class for observers that allows conrete observers to derive from it
class Observer
{
public:
	//constructor
	Observer() = default;
	//destructor
	virtual ~Observer() = default;
	//copy, move and assignment operators have been disabled for this class
	Observer(const Observer&) = delete;
	Observer(const Observer&&) = delete;
	Observer& operator=(const Observer&) = delete;

	//function toperform a call to the abstract call function to post the param event to the observer function
	void exec(Event* e)
	{
		call(e);
	}
	//function to get an instance of the object/class the function pointer is bound to
	virtual void* Instance() = 0;

private:
	//abstract call function takes an event as a parameter to be implemented in derived classes
	virtual void call(Event* e) = 0;
};

//template class MemberObserver is an observer that is a class with a designated obersvation member function
template<typename T, typename ConcreteEvent>
class MemberObserver : public Observer
{
public:
	//typedefine for the pointer to the class member function
	typedef void (T::* MemberFunction)(ConcreteEvent*);
	//constructor
	MemberObserver(T* a_instance, MemberFunction a_function) : m_instance(a_instance), m_memberFunction(a_function) {}
	//destructor
	~MemberObserver() { m_instance = nullptr; }
	//function to return a pointer to the instance of the class the member function belongs to
	void* Instance() { return (void*)m_instance; }

private:
	//implementation of abstract base class function for calling observer function
	void call(Event* e)
	{
		//cast event to correct type
		(m_instance->*m_memberFunction)(static_cast<ConcreteEvent*>(e));
	}

private:
	//the member function in the class that we hold a pointer to
	MemberFunction m_memberFunction;
	//the class instance to call the function to
	T* m_instance;
};

//template class GlobalObserver, the template argument ConcreteEvent is deduced by the compiler
template<typename ConcreteEvent>
class GlobalObserver : public Observer
{
	//typedefine of function pointer to non-member function that takes a concrete event as a parameter
	typedef void (*Function)(ConcreteEvent*);
	//constructor sets function pointer member to point to paramter function
	GlobalObserver(Function a_function) : m_function(a_function) {}
	//destructor
	~GlobalObserver() {}
	//instance function implementation, global functions have no instances so returns nullptr
	void* Instance() { return nullptr; }

private:
	//call function will call global function member variable with Event parameter
	void call(Event* e)
	{
		//cast event to crorrect type
		(*m_function)(static_cast<ConcreteEvent*>(e));
	}

private:
	//member variable pointer to global/static function
	Function m_function;
};