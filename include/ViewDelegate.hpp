#pragma once

#include "Metal.hpp"
#include "Renderer.hpp"

class ViewDelegate: public MTK::ViewDelegate {
    public:
        ViewDelegate(MTL::Device *pDevice, MTK::View *pView);
        virtual ~ViewDelegate() override;

        virtual void drawInMTKView(MTK::View *pView) override;
    private:
        Renderer* _pRenderer;
};