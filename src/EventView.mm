#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "EventDelegate.h"
#include "EventView.h"

@interface EventViewExtender: MTKView 
@property EventDelegate *eventDelegate;
- (id)init:(CGRect)frame device:(id<MTLDevice>)device;
@end

EventView* EventView::alloc() {
    return (EventView*)[EventViewExtender alloc];
}

EventView* EventView::init(CGRect frame, MTL::Device *pDevice) {
    [(id)this init:frame device:(id)pDevice];
    return this;
}

void EventView::release() {
  [(id)this release];
}

void EventView::setEventDelegate(EventDelegate *delegate) {
    EventViewExtender* self = (id)this;
    self.eventDelegate = delegate;
}

@implementation EventViewExtender
- (id)init:(CGRect)frame device:(id<MTLDevice>)device {
  [self becomeFirstResponder];
  [self initWithFrame:frame device:device];
  return self;
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)keyDown:(NSEvent*)event {
  self.eventDelegate->keyDown(event.keyCode);
}

- (void)keyUp:(NSEvent*)event {
  self.eventDelegate->keyUp(event.keyCode);
}

- (void)mouseDragged:(NSEvent*)event {
  self.eventDelegate->mouseDragged(event.deltaX, event.deltaY);
}
@end