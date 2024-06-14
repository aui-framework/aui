/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
//  AUIViewController.m
//  Cube
//
//  Created by Alexey Titov on 02.12.2021.
//

#import "AUIViewController.h"
#import <CoreFoundation/CoreFoundation.h>
#import "AUIView.h"

#include <string>
#include <AUI/Platform/AWindowManager.h>
#include <AUI/Platform/AWindow.h>

@interface AUIViewController()
{
}
@end

@implementation AUIViewController {
}

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

@end
