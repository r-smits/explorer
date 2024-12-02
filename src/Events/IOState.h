#pragma once
#include <Events/KeyCodes.h>
#include <pch.h>
#include <simd/simd.h>

namespace EXP {

class IO {

private:
  static void set(int keyCode);
  static void unset(int keyCode);

public:
  static bool isPressed(int keyCode);
	static simd::float2 getMouse();

public: // Event
  static void onEvent(Event& event);
  static bool onKeyPressed(KeyPressedEvent& event);
  static bool onKeyReleased(KeyReleasedEvent& event);
  static bool onMouseMoved(MouseMoveEvent& event);
  static bool onMousePressed(MouseButtonPressedEvent& event);
  static bool onMouseReleased(MouseButtonReleasedEvent& event);
};

}; // namespace EXP
