#pragma once
#include <pch.h>
#include <sstream>
#include <string>

namespace EXP {

class KeyEvent : public Event {

public:
  inline const int getKey() const { return keyCode; }
  EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
  KeyEvent(const int keyCode) : keyCode(keyCode) {}

  const int keyCode;
};

class KeyPressedEvent : public KeyEvent {

public:
  KeyPressedEvent(const int keyCode, int repeatCount)
      : KeyEvent(keyCode), repeatCount(repeatCount) {}

  inline int getRepeatCount() const { return this->repeatCount; }

  std::string toString() const override {
    std::stringstream ss;
    ss << "KeyPressed :: (" << this->keyCode << ")(" << this->repeatCount << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyPressed)

private:
  int repeatCount;
};

class KeyReleasedEvent : public KeyEvent {

public:
  KeyReleasedEvent(const int keyCode) : KeyEvent(keyCode) {}

  std::string toString() const override {
    std::stringstream stringstream;
    stringstream << "KeyReleased :: (" << this->keyCode << ")";
    return stringstream.str();
  }

  EVENT_CLASS_TYPE(KeyReleased)
};

} // namespace EXP
