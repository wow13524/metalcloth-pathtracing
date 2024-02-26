#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include "ApplicationDelegate.hpp"

int main(int argc, char **argv) {
    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

    ApplicationDelegate applicationDelegate;

    NS::Application *pSharedApplication = NS::Application::sharedApplication();
    pSharedApplication->setDelegate(&applicationDelegate);
    pSharedApplication->run();

    pPool->release();

    return 0;
}