#pragma once

#include "EventView.h"
#include "Metal.hpp"
#include "Renderer.hpp"

class ViewDelegate: public MTK::ViewDelegate {
    public:
        ViewDelegate(MTL::Device *pDevice, EventView *pView);
        virtual ~ViewDelegate() override;

        virtual void drawInMTKView(MTK::View *pView) override;
    private:
        Renderer* _pRenderer;
};