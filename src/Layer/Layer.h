#pragma once
#include <Control/AppProperties.h>
#include <pch.h>

namespace EXP {

class Layer {

public:
  Layer(MTL::Device* device, EXP::AppProperties* config, const std::string& name = "Layer");
  virtual ~Layer();

  virtual void onAttach();
  virtual void onDetach();
  virtual void onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder);
  virtual void onEvent(Event& event);

  inline const std::string& getName() { return name; }

protected:
  AppProperties* config;
  MTL::Device* device;

private:
  const std::string name;
};

} // namespace EXP
