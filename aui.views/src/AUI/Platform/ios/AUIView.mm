/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "AUIView.h"

#include <AUI/Platform/AWindowManager.h>
#include <AUI/Platform/AWindow.h>
#include <string>
#include <OpenGLES/EAGLDrawable.h>

@implementation AUIView
{
    CADisplayLink* displayLink;
    BOOL animating;
    std::vector<UITouch*> trackedTouches;
}

@synthesize context;

static AUIView* view = nullptr;

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

static std::string ios_get_path_in_bundle() {
    std::string s;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString* docsDir = [paths objectAtIndex:0];
    s = [docsDir UTF8String];
    return s;
}
static const _<AWindow>& auiWindow() {
    return AWindow::getWindowManager().getWindows().front();
}


extern int(* _gEntry)(AStringVector);

static GLuint defaultFb, colorBuffer = 0;

- (instancetype) initWithFrame:(CGRect)frame
{    
    if ((self = [super initWithFrame:frame]))
	{
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        
        if (!context || ![EAGLContext setCurrentContext:context])
		{
            AUI_ASSERT(0);
            return nil;
		}
        
        animating = NO;
        displayLink = nil;
        view = self;
        [self setMultipleTouchEnabled:true];
        
        // setup "default" framebuffer
        glGenFramebuffers(1, &defaultFb);
        

        chdir(ios_get_path_in_bundle().c_str());
        _gEntry({});
    }
	
    return self;
}

- (void) drawRect:(CGRect)rect
{
    [super drawRect:rect];
}

- (void) drawView:(id)sender
{
    [EAGLContext setCurrentContext:context];
    
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        
        AUI_ASSERT(defaultFb == 1);
        
        [self layoutSubviews];
        AUI_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }
    
    if (!AWindow::getWindowManager().getWindows().empty()) {
        AThread::processMessages();
        AWindow::getWindowManager().getWindows().front()->redraw();
        glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
        [context presentRenderbuffer:GL_RENDERBUFFER];
    }
}
- (void) layoutSubviews
{
    [EAGLContext setCurrentContext:context];
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFb);
    
    if (colorBuffer != 0) {
        glDeleteRenderbuffers(1, &colorBuffer);
    }
    
    glGenRenderbuffers(1, &colorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    if (![self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer]) {
        AUI_ASSERT(0);
    }
    
    CGSize size = self.bounds.size;
    float scale = (float)self.contentScaleFactor;
    auiWindow()->setSize({size.width * scale, size.height * scale});
}

- (void) startAnimation
{
    if (!animating)
    {
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];

        [displayLink setFrameInterval:1];

        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        animating = TRUE;
        [self setNeedsDisplay];
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [displayLink invalidate];
        displayLink = nil;
        animating = FALSE;
    }
}

- (void) dealloc
{
    // tear down context
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auto vec = glm::ivec2{location.x * scale, location.y * scale};
        auto it = std::find(trackedTouches.begin(), trackedTouches.end(), nullptr);
        auto index = it - trackedTouches.begin();
        auiWindow()->onPointerPressed({vec, APointerIndex::finger(index)});

        trackedTouches.insert(it, touch);
        
    }
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auto index = std::find(trackedTouches.begin(), trackedTouches.end(), touch) - trackedTouches.begin();
        auiWindow()->onPointerMove(glm::vec2{location.x * scale, location.y * scale}, APointerMoveEvent{APointerIndex::finger(index)});
    }
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auto it = std::find(trackedTouches.begin(), trackedTouches.end(), touch);
        auto index = it - trackedTouches.begin();
        auiWindow()->onPointerReleased({glm::ivec2{location.x * scale, location.y * scale}, APointerIndex::finger(index)});
        *it = nullptr;
    }
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:context];
    float scale = (float)self.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self];
        auto it = std::find(trackedTouches.begin(), trackedTouches.end(), touch);
        auto index = it - trackedTouches.begin();
        auiWindow()->onPointerReleased({.position = glm::ivec2{location.x * scale, location.y * scale}, .pointerIndex = APointerIndex::finger(index), .triggerClick = false});
        *it = nullptr;
    }
}

extern "C" void _aui_ios_redraw() {
    dispatch_async(dispatch_get_main_queue(), ^{
        [view setNeedsDisplay];
    });
}

extern "C" void _aui_ios_setMobileScreenOrientation(AScreenOrientation orientation) {
    dispatch_async(dispatch_get_main_queue(), ^{
        UIInterfaceOrientation currentOrientation = [UIApplication sharedApplication].statusBarOrientation;
        NSNumber *value = [&] {
            switch (orientation) {
                case AScreenOrientation::PORTRAIT: return [NSNumber numberWithInt:UIInterfaceOrientationPortrait];
                case AScreenOrientation::LANDSCAPE: return [NSNumber numberWithInt:UIInterfaceOrientationLandscapeLeft];
                default: return [NSNumber numberWithInt:UIInterfaceOrientationUnknown];
            }
        }();
        [[UIDevice currentDevice] setValue:value forKey:@"orientation"];    
        [UIViewController attemptRotationToDeviceOrientation];
    });
}

@end
