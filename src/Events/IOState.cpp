#include "Events/MouseEvent.h"
#include <Events/IOState.h>
#include <pch.h>

namespace EXP {

std::unordered_map<int, float> mouseState = {
    {MOUSE_X, 0},
    {MOUSE_Y, 0}
};

std::unordered_map<int, bool> keyState = {
    {          KEY_A, false},
    {          KEY_A, false},
    {          KEY_B, false},
    {          KEY_C, false},
    {          KEY_D, false},
    {          KEY_E, false},
    {          KEY_F, false},
    {          KEY_G, false},
    {          KEY_H, false},
    {          KEY_I, false},
    {          KEY_J, false},
    {          KEY_K, false},
    {          KEY_L, false},
    {          KEY_M, false},
    {          KEY_N, false},
    {          KEY_O, false},
    {          KEY_P, false},
    {          KEY_Q, false},
    {          KEY_R, false},
    {          KEY_S, false},
    {          KEY_T, false},
    {          KEY_U, false},
    {          KEY_V, false},
    {          KEY_W, false},
    {          KEY_X, false},
    {          KEY_Y, false},
    {          KEY_Z, false},
    {      KEY_SPACE, false},
    {        KEY_TAB, false},
    {      KEY_MINUS, false},
    {     KEY_EQUALS, false},
    {      KEY_ENTER, false},
    {  KEY_PARAGRAPH, false},
    {KEY_PARENTHESIS, false},
    {        KEY_ONE, false},
    {        KEY_TWO, false},
    {      KEY_THREE, false},
    {       KEY_FOUR, false},
    {       KEY_FIVE, false},
    {        KEY_SIX, false},
    {      KEY_SEVEN, false},
    {      KEY_EIGHT, false},
    {       KEY_NINE, false},
    {       KEY_ZERO, false},
    {       ARROW_UP, false},
    {     ARROW_DOWN, false},
    {    ARROW_RIGHT, false},
    {       ARROW_UP, false},
    {  KEY_FWD_SLASH, false},
    {        KEY_DOT, false},
    {      KEY_COMMA, false},
    {      KEY_THREE, false},
    {      KEY_THREE, false},
    {          MOUSE, false},
    {     MOUSE_DRAG, false},
    {       ARROW_UP, false},
    {     ARROW_DOWN, false},
    {     ARROW_LEFT, false},
    {    ARROW_RIGHT, false}
};
} // namespace EXP

void EXP::IO::set(int keyCode) { keyState[keyCode] = true; }
void EXP::IO::unset(int keyCode) { keyState[keyCode] = false; }

bool EXP::IO::isPressed(int keyCode) { return keyState.at(keyCode); }
simd::float2 EXP::IO::getMouse() { return {mouseState[MOUSE_X], mouseState[MOUSE_Y]};}

void EXP::IO::onEvent(Event& event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<KeyPressedEvent>(IO::onKeyPressed);
  dispatcher.dispatch<KeyReleasedEvent>(IO::onKeyReleased);
  dispatcher.dispatch<MouseMoveEvent>(IO::onMouseMoved);
  dispatcher.dispatch<MouseButtonPressedEvent>(IO::onMousePressed);
  dispatcher.dispatch<MouseButtonReleasedEvent>(IO::onMouseReleased);
}

bool EXP::IO::onKeyPressed(KeyPressedEvent& event) {
  IO::set(event.getKey());
  return true;
}

bool EXP::IO::onKeyReleased(KeyReleasedEvent& event) {
  IO::unset(event.getKey());
  return true;
}

bool EXP::IO::onMouseMoved(MouseMoveEvent& event) {
  mouseState[MOUSE_X] = event.getX();
  mouseState[MOUSE_Y] = event.getY();
  return true;
}

bool EXP::IO::onMousePressed(MouseButtonPressedEvent& event) {
  keyState[MOUSE] = true;
  return true;
}

bool EXP::IO::onMouseReleased(MouseButtonReleasedEvent& event) {
  keyState[MOUSE] = false;
  return true;
}
