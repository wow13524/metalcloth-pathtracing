#pragma once

#include "EventDelegate.h"
#include "Metal.hpp"

class EventView: public MTK::View {
    public:
        static EventView* alloc();
        EventView* init(CGRect frame, MTL::Device *pDevice);
        void release();
        void setEventDelegate(EventDelegate *delegate);
};