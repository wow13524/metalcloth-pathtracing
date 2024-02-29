#pragma once

#include "EventView.h"
#include "Metal.hpp"
#include "ViewDelegate.hpp"

class ApplicationDelegate: public NS::ApplicationDelegate {
    public:
        ~ApplicationDelegate();

        virtual void applicationWillFinishLaunching(NS::Notification *pNotification) override;
        virtual void applicationDidFinishLaunching(NS::Notification *pNotification) override;
        virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application *pSender) override;

    private:
        MTL::Device *_pDevice;
        EventView *_pView;
        NS::Window *_pWindow;
        ViewDelegate *_pViewDelegate;

        NS::Menu* createMenuBar();
};