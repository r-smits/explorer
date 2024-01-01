#pragma once
#include <iostream>

#define BIT(x) (1 << x)

namespace Explorer {

enum class EventType {
  None = 0,
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,
  AppTick,
  AppUpdate,
  AppRender,
  KeyPressed,
  KeyReleased,
  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled
};

enum EventCategory {
  None = 0,
  EventCatgoryApplication = BIT(0),
  EventCategoryInput = BIT(1),
  EventCategoryKeyboard = BIT(2),
  EventCategoryMouse = BIT(3),
  EventCategoryMouseButton = BIT(4)
};

class Event {
  friend class EventDispatcher;

public:
  virtual EventType getEventType() const = 0;
  virtual const char *getName() const = 0;
  virtual int getCategoryFlags() const = 0;
  virtual std::string toString() const { return getName(); };
  inline bool isInCategory(EventCategory category) { return getCategoryFlags() & category; };
  inline bool isHandled() { return this->handled; }
  inline void done() { this->handled = true; }

private:
  bool handled = false;
};

#define EVENT_CLASS_TYPE(type)                                                                     \
  static EventType getStaticType() { return EventType::type; }                                     \
  virtual EventType getEventType() const override { return getStaticType(); }                      \
  virtual const char *getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category)                                                             \
  virtual int getCategoryFlags() const override { return category; }

class EventDispatcher {
  template <typename T> using eventFn = std::function<bool(T &)>;

public:
  EventDispatcher(Event &event) : event(event) {}

  template <typename T> bool dispatch(eventFn<T> func) {
    if (event.getEventType() == T::getStaticType()) {
      event.handled = func(*(T *)&event);
      return true;
    }
    return false;
  }

private:
  Event &event;
};
} // namespace Explorer
