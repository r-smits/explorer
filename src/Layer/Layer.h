#pragma once
#include <Events/Events.h>
#include <pch.h>

namespace Explorer {

class Layer {

public:
  Layer(const std::string &name = "Layer");
  virtual ~Layer();

  virtual void onAttach();
  virtual void onDetach();
  virtual void onUpdate();
  virtual void onEvent(Event &event);

  inline const std::string &getName() { return name; }

private:
  std::string name;
};

} // namespace Explorer
