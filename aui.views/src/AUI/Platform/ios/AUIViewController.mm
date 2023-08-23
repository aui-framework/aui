// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
//  AUIViewController.m
//  Cube
//
//  Created by Alexey Titov on 02.12.2021.
//

#import "AUIViewController.h"
#import <CoreFoundation/CoreFoundation.h>

#include <string>
#include <AUI/Platform/AWindowManager.h>
#include <AUI/Platform/AWindow.h>

@interface AUIViewController()
@end


AUIViewController* controller;

const _<AWindow>& auiWindow() {
    return AWindow::getWindowManager().getWindows().front();
}

std::string ios_get_path_in_bundle() {
    std::string s;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString* docsDir = [paths objectAtIndex:0];
    s = [docsDir UTF8String];
    return s;
}


@implementation AUIViewController {
    glm::ivec2 originPoint;
    glm::ivec2 prevReportedScrollPoint;
    GLKView* view;
    NSMutableArray<UITouch*>* trackedTouches;
}

+ (AUIViewController*)instance {
    return controller;
}


extern int(* _gEntry)(AStringVector);

- (void)viewDidLoad
{
    [super viewDidLoad];
    trackedTouches = [[NSMutableArray<UITouch*> alloc] init];

    view = (GLKView*)self.view;
    view.context = [[EAGLContext alloc] initWithAPI:
            kEAGLRenderingAPIOpenGLES3];
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    [EAGLContext setCurrentContext:view.context];
    self.preferredFramesPerSecond = 60;
    chdir(ios_get_path_in_bundle().c_str());

    _gEntry({});

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(gestureHandlerMethodTap:)];
    tapRecognizer.cancelsTouchesInView = NO;
    UIPanGestureRecognizer *swipeRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self  action:@selector(gestureHandlerMethodScroll:)];
    swipeRecognizer.cancelsTouchesInView = NO;
    
    [view addGestureRecognizer:swipeRecognizer];
    [view addGestureRecognizer:tapRecognizer];
    controller = self;
    [self setNeedsStatusBarAppearanceUpdate];
}

-(void)gestureHandlerMethodTap:(UITapGestureRecognizer*)sender {
    [EAGLContext setCurrentContext:view.context];
    CGPoint p = [sender locationInView:self.view];
    float scale = (float)self.view.contentScaleFactor;
    //IosEntry::handleClick(p.y * scale, [[UIScreen mainScreen] nativeBounds].size.width - p.x * scale);
    auiWindow()->onPointerReleased({glm::ivec2{p.x * scale, p.y * scale}, APointerIndex::finger(0)});
}

-(void)gestureHandlerMethodScroll:(UIPanGestureRecognizer*)sender {
    [EAGLContext setCurrentContext:view.context];
    float millis = auiWindow()->frameMillis() / 1000.f;
    float scale = (float)self.view.contentScaleFactor;
    CGPoint p = [sender locationInView:self.view];
    //CGPoint v = [sender velocityInView:self.view];
    
    auto currentPosition = glm::ivec2{p.x * scale, p.y * scale};
    auto delta = prevReportedScrollPoint - currentPosition;
    prevReportedScrollPoint = currentPosition;
    
    //CGPoint v = {prevTransation.x - t.x, prevTransation.y - t.y};
    //prevTransation = t;
    //auiWindow()->onGesture(originPoint, AFingerDragEvent{delta});
}


- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    [EAGLContext setCurrentContext:view.context];
    AThread::processMessages();
    auiWindow()->redraw();
}
- (void)viewDidLayoutSubviews {
    [EAGLContext setCurrentContext:view.context];
    CGSize size = self.view.bounds.size;
    float scale = (float)self.view.contentScaleFactor;
    auiWindow()->setSize({size.width * scale, size.height * scale});
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:view.context];
    float scale = (float)self.view.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self.view];
        auto vec = glm::ivec2{location.x * scale, location.y * scale};
        originPoint = prevReportedScrollPoint = vec;
        auiWindow()->onPointerPressed({vec, APointerIndex::finger([trackedTouches count])});
        [trackedTouches addObject:touch];
    }
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:view.context];
    float scale = (float)self.view.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self.view];
        auiWindow()->onPointerMove(glm::vec2{location.x * scale, location.y * scale}, APointerMoveEvent{APointerIndex::finger([trackedTouches indexOfObject:touch])});
    }
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [EAGLContext setCurrentContext:view.context];
    float scale = (float)self.view.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self.view];
        auiWindow()->onPointerReleased({glm::ivec2{location.x * scale, location.y * scale}, APointerIndex::finger([trackedTouches indexOfObject:touch])});
        [trackedTouches removeObject:touch];
    }
}

- (UIStatusBarStyle)preferredStatusBarStyle
{
    return UIStatusBarStyleDarkContent;
}
/*
- (BOOL)shouldAutorotate {
    return YES;
}

-(NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskLandscape | UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation
{
    return UIInterfaceOrientationLandscapeLeft;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationMaskLandscape || interfaceOrientation == UIInterfaceOrientationMaskLandscapeLeft || interfaceOrientation == UIInterfaceOrientationMaskLandscapeRight);
}
*/
@end
