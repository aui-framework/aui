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
    CGPoint prevTransation;
}

+ (AUIViewController*)instance {
    return controller;
}

extern int(* _gEntry)(AStringVector);

- (void)viewDidLoad
{
    [super viewDidLoad];

    GLKView* view = (GLKView*)self.view;
    view.context = [[EAGLContext alloc] initWithAPI:
            kEAGLRenderingAPIOpenGLES3];
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    [EAGLContext setCurrentContext:view.context];
    chdir(ios_get_path_in_bundle().c_str());

    _gEntry({});

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(gestureHandlerMethodTap:)];
    
    UIPanGestureRecognizer *swipeRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self  action:@selector(gestureHandlerMethodScroll:)];
    
    [view addGestureRecognizer:swipeRecognizer];
    [view addGestureRecognizer:tapRecognizer];
    controller = self;
    [self setNeedsStatusBarAppearanceUpdate];
}

-(void)gestureHandlerMethodTap:(UITapGestureRecognizer*)sender {
    CGPoint p = [sender locationInView:self.view];
    float scale = (float)self.view.contentScaleFactor;
    //IosEntry::handleClick(p.y * scale, [[UIScreen mainScreen] nativeBounds].size.width - p.x * scale);
    auiWindow()->onMouseReleased(glm::ivec2{p.x * scale, p.y * scale}, AInput::LBUTTON);
}

-(void)gestureHandlerMethodScroll:(UIPanGestureRecognizer*)sender {
    float scale = (float)self.view.contentScaleFactor;
    CGPoint p = [sender locationInView:self.view];
    CGPoint v = [sender velocityInView:self.view];
    //CGPoint v = {prevTransation.x - t.x, prevTransation.y - t.y};
    //prevTransation = t;
    auiWindow()->onGesture(glm::ivec2{p.x * scale, p.y * scale}, AFingerDragEvent{glm::ivec2{-v.x * scale / 45.f, -v.y * scale / 45.f}});
}


- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    AThread::processMessages();
    auiWindow()->redraw();
}
- (void)viewDidLayoutSubviews {
    CGSize size = self.view.bounds.size;
    float scale = (float)self.view.contentScaleFactor;
    auiWindow()->setSize({size.width * scale, size.height * scale});
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    float scale = (float)self.view.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self.view];
        auiWindow()->onMousePressed(glm::ivec2{location.x * scale, location.y * scale}, AInput::LBUTTON);
    }
    prevTransation = {0, 0};
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    float scale = (float)self.view.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self.view];
        auiWindow()->onMouseMove(glm::ivec2{location.x * scale, location.y * scale});
    }
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    float scale = (float)self.view.contentScaleFactor;
    for (UITouch* touch in touches) {
        CGPoint location = [touch locationInView:self.view];
        auiWindow()->onMouseReleased(glm::ivec2{location.x * scale, location.y * scale}, AInput::LBUTTON);
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
