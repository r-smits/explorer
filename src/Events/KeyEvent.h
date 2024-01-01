#pragma once
#include <pch.h>
#include <sstream>
#include <string>

namespace Explorer {

class KeyEvent : Explorer::Event {

public:
  inline const char *getKey() const { return key; }
  EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
  KeyEvent(const char *key) : key(key) {}

  const char *key;
};

class KeyPressedEvent : KeyEvent {

public:
  KeyPressedEvent(const char *key, int repeatCount)
      : Explorer::KeyEvent(key), repeatCount(repeatCount) {}

  inline int getRepeatCount() const { return this->repeatCount; }

  std::string toString() const override {
    std::stringstream ss;
    ss << "KeyPressed :: (" << this->key << ")(" << this->repeatCount << ")";
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyPressed)

private:
  int repeatCount;
};

class KeyReleasedEvent : KeyEvent {

public:
  KeyReleasedEvent(const char *key) : Explorer::KeyEvent(key) {}

  std::string toString() const override {
    std::stringstream stringstream;
    stringstream << "KeyReleased :: (" << this->key << ")";
    return stringstream.str();
  }

  EVENT_CLASS_TYPE(KeyReleased)
};

} // namespace Explorer
