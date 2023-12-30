#include "Events/Events.h"

namespace Explorer {

class MouseMoveEvent : Event {
public:
  MouseMoveEvent(float x, float y) : mouseX(x), mouseY(y){};

  inline float getX() const { return mouseX; }
  inline float getY() const { return mouseY; }

  std::string toString() const override {
    std::stringstream ss;
    ss << "Mouse to position: "
       << "(" << mouseX << ", " << mouseY << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
  float mouseX;
  float mouseY;
};

class MouseButtonEvent : Event {
public:
  int getMouseButton() const { return button; }
  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
protected:
  MouseButtonEvent(int button) : button(button) {}
  int button;
};

class MouseButtonPressedEvent : MouseButtonEvent {

public:
  MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

  std::string toString() const override {
    std::stringstream ss;
    ss << "Mouse pressed event: " << button;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : MouseButtonEvent {

public:
  MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

  std::string toString() const override {
    std::stringstream ss;
    ss << "Mouse released event: " << button;
    return ss.str();
  }

  EVENT_CLASS_TYPE(MouseButtonReleased)
};

}; // namespace Explorer
