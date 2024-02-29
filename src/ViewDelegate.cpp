#include "ViewDelegate.hpp"

ViewDelegate::ViewDelegate(MTL::Device *pDevice, EventView *pView) {
    this->_pRenderer = new Renderer(pDevice, pView);
}

ViewDelegate::~ViewDelegate() {
    delete this->_pRenderer;
}

void ViewDelegate::drawInMTKView(MTK::View *pView) {
    this->_pRenderer->draw(pView);
}