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

std::string ios_get_path_in_bundle() {
    std::string s;
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
    NSString* docsDir = [paths objectAtIndex:0];
    s = [docsDir UTF8String];
    return s;
}


@implementation AUIViewController

extern int(* _gEntry)(const AStringVector&);

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

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(gestureHandlerMethod:)];
    [view addGestureRecognizer:tapRecognizer];
    controller = self;
    [self setNeedsStatusBarAppearanceUpdate];
}
-(void)gestureHandlerMethod:(UITapGestureRecognizer*)sender {
    CGPoint p = [sender locationInView:NULL];
    double scale = [[UIScreen mainScreen] scale];
    //IosEntry::handleClick(p.y * scale, [[UIScreen mainScreen] nativeBounds].size.width - p.x * scale);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    AWindow::getWindowManager().getWindows().front()->redraw();
}
- (void)viewDidLayoutSubviews {
    CGSize size = self.view.bounds.size;
    float scale = (float)self.view.contentScaleFactor;
    AWindow::getWindowManager().getWindows().front()->setSize(size.width * scale, size.height * scale);
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
