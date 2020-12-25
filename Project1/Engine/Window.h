#pragma once
#include <GLFW/glfw3.h>
#include <functional>

namespace Events
{
	#define EVENT_TYPE_FUNCS(type) \
	virtual EventType getEventType() const override { return getStaticEventType();} \
	static EventType getStaticEventType(){return type;}

	enum EventType
	{
		none,
		KeyPressed,
		KeyReleased,
		MousePressed,
		MouseReleased,
		MouseMoved
	};

	struct Event
	{
		virtual EventType getEventType() const = 0;
		virtual ~Event() = default;
	};

	//*************KeyEvents******************
	struct KeyEvent : Event
	{
		int KeyCode;
		KeyEvent(const int key) :
			KeyCode(key){}
	};

	struct KeyPressedEvent : KeyEvent
	{
		KeyPressedEvent(const int key) :
			KeyEvent(key) {}

		EVENT_TYPE_FUNCS(KeyPressed)
	};

	struct KeyReleasedEvent : KeyEvent
	{
		KeyReleasedEvent(const int key) :
			KeyEvent(key) {}

		EVENT_TYPE_FUNCS(KeyReleased)
	};
	//*******************************************

	//*************MouseMovedEvents***************
	struct MouseMoveEvent : Event
	{
		float mouseX, mouseY;
		MouseMoveEvent(const float x, const float y)
			: mouseX(x), mouseY(y){}

		EVENT_TYPE_FUNCS(MouseMoved)
	};
	//********************************************

	//*************MouseClickEvents***************
	struct MousePressedEvent : Event
	{
		float mouseCode;

		MousePressedEvent(float keyCode) :
			mouseCode(keyCode){}

		EVENT_TYPE_FUNCS(MousePressed)
	};

	struct MouseReleasedEvent : Event
	{
		float mouseCode;

		MouseReleasedEvent(float keyCode) :
			mouseCode(keyCode){}

		EVENT_TYPE_FUNCS(MouseReleased)
	};
	//********************************************

	struct EventDispatcher
	{
		Event& event_;

		EventDispatcher(Event& e) :
			event_(e){}

		template<typename T>
		void dispatch(std::function<void(T&)> fn)
		{
			if (event_.getEventType() == T::getStaticEventType())
			{
				fn(*(T*)&event_);
			}
		}
	};
}

class Window
{
public:
	Window();
	~Window();
	GLFWwindow* getGLFWWindow();
	void setWindowWidth(float width);
	void setWindowHeight(float height);
	void setCursor(bool cursorEnabled);
	void onUpdate();
	void setEventCallback(const std::function<void(Events::Event&)> e);
private:
	struct WindowInfo
	{
		float width;
		float height;
		std::function<void(Events::Event&)> callback;
	};

	WindowInfo windowInfo;

	GLFWwindow* window;
};

