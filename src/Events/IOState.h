#pragma once
#include <Events/KeyCodes.h>
#include <pch.h>

namespace Explorer {

class IO {

private:
  static void set(int keyCode);
	static void unset(int keyCode);

public:
  static bool isPressed(int keyCode);

public: // Event
  static void onEvent(Event &event);
  static bool onKeyPressed(KeyPressedEvent &event);
  static bool onKeyReleased(KeyReleasedEvent &event);
};

}; // namespace Explorer
