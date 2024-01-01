#pragma once
#include <Events/Events.h>
#include <pch.h>

namespace Explorer {

class Layer {

public:
  Layer(MTL::Device* device, const std::string &name = "Layer");
  virtual ~Layer();

  virtual void onAttach();
  virtual void onDetach();
  virtual void onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder);
  virtual void onEvent(Event &event);

  inline const std::string &getName() { return name; }

protected:
	MTL::Device* device;
	MTL::CommandQueue* queue;

private:
	const std::string name;
};

} // namespace Explorer
