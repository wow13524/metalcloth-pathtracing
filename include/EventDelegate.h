#pragma once

class EventDelegate {
    public:
        virtual ~EventDelegate() = default;
        virtual void keyDown(unsigned int keyCode) {};
        virtual void keyUp(unsigned int keyCode) {};
        virtual void mouseDragged(float deltaX, float deltaY) {};
};