#pragma once
#include <pch.h>

namespace EXP {

class MouseMoveEvent : public Event {
public:
  MouseMoveEvent(float x, float y) : mouseX(x), mouseY(y){};

  inline float getX() const { return mouseX; }
  inline float getY() const { return mouseY; }

	std::string toString() const override {
    std::stringstream ss;
    ss << "MouseMoveEvent :: (" << getX() << ", " << getY() << ")";
    return ss.str();
  }



  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
protected:
  float mouseX;
  float mouseY;
};

class MouseButtonEvent : public Event {
public:
  int getMouseButton() const { return button; }
  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
protected:
  MouseButtonEvent(int button) : button(button) {}
  int button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {

public:
  MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

  std::string toString() const override {
    std::stringstream ss;
    ss << "MousePressedButtonEvent :: (" << button << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {

public:
  MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

  std::string toString() const override {
    std::stringstream ss;
    ss << "MouseReleasedButtonEvent :: (" << button << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonReleased)
};

}; // namespace EXP
