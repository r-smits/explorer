#include <Control/ViewDelegate.h>

Explorer::ViewDelegate::ViewDelegate(MTL::Device* device) : MTK::ViewDelegate(), renderer(new Renderer(device)) {}

Explorer::ViewDelegate::~ViewDelegate() {
    delete renderer;
}

void Explorer::ViewDelegate::drawInMTKView(MTK::View* view) {
    renderer->draw(view);
}
