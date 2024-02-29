#include "ApplicationDelegate.hpp"

ApplicationDelegate::~ApplicationDelegate() {
    this->_pDevice->release();
    this->_pView->release();
    this->_pWindow->release();
    delete this->_pViewDelegate;
}

NS::Menu* ApplicationDelegate::createMenuBar() {
    NS::Menu *pMainMenu = NS::Menu::alloc()->init();

    NS::MenuItem *pAppMenuItem = NS::MenuItem::alloc()->init();
    NS::Menu *pAppMenu = NS::Menu::alloc()->init(NS::String::string("Cloth Simulation", NS::StringEncoding::UTF8StringEncoding));

    NS::String *appName = NS::RunningApplication::currentApplication()->localizedName();
    NS::String *quitItemName = NS::String::string("Quit ", NS::StringEncoding::UTF8StringEncoding)->stringByAppendingString(appName);
    SEL quitCb = NS::MenuItem::registerActionCallback("appQuit", [](void*, SEL, const NS::Object *pSender){
        auto pApp = NS::Application::sharedApplication();
        pApp->terminate(pSender);
    });

    NS::MenuItem *pAppQuitItem = pAppMenu->addItem(quitItemName, quitCb, NS::String::string("q", NS::StringEncoding::UTF8StringEncoding));
    pAppQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
    pAppMenuItem->setSubmenu(pAppMenu);

    pMainMenu->addItem(pAppMenuItem);

    pAppMenuItem->release();
    pAppMenu->release();

    return pMainMenu->autorelease();
}

void ApplicationDelegate::applicationWillFinishLaunching(NS::Notification *pNotification) {
    NS::Menu* pMenu = createMenuBar();
    NS::Application* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
    pApp->setMainMenu(pMenu);
    pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void ApplicationDelegate::applicationDidFinishLaunching(NS::Notification *pNotification) {
    CGRect frame {
        .origin = {0.0, 0.0},
        .size = {1280.0, 720.0}
    };

    this->_pWindow = NS::Window::alloc()->init(
        frame,
        NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
        NS::BackingStoreBuffered,
        false
    );

    this->_pDevice = MTL::CreateSystemDefaultDevice();

    this->_pView = EventView::alloc()->init(frame, this->_pDevice);
    this->_pView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    this->_pView->setClearColor(MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0));

    this->_pViewDelegate = new ViewDelegate(this->_pDevice, this->_pView);
    this->_pView->setDelegate(this->_pViewDelegate);

    this->_pWindow->setContentView(this->_pView);
    this->_pWindow->setTitle(NS::String::string("Project 4 - Cloth Simulation", NS::StringEncoding::UTF8StringEncoding));
    this->_pWindow->makeKeyAndOrderFront(nullptr);

    NS::Application* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
    pApp->activateIgnoringOtherApps(true);
}

bool ApplicationDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application *pSender) {
    return true;
}