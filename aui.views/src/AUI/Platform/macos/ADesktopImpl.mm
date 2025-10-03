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

#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ARaiiHelper.h>

#include <AUI/Platform/AWindow.h>
#include <AUI/Util/AImageDrawable.h>

#include <AppKit/AppKit.h>
#include <Availability.h>

#if !defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || !defined(__MAC_11_0) || __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_11_0
#define AUI_MACOS_SUPPORTS_CONTENTTYPES 0
#else
#define AUI_MACOS_SUPPORTS_CONTENTTYPES 1
#endif

#if AUI_MACOS_SUPPORTS_CONTENTTYPES == 1
#include <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#endif

glm::ivec2 ADesktop::getMousePosition() {
    @autoreleasepool {
        NSPoint mouseLocation = [NSEvent mouseLocation];

        // Convert screen coordinates to flipped coordinates
        NSScreen* mainScreen = [NSScreen mainScreen];
        mouseLocation.y = mainScreen.frame.size.height - mouseLocation.y;

        return { mouseLocation.x, mouseLocation.y };
    }
}

void ADesktop::setMousePos(const glm::ivec2& pos) {
    CGPoint cgPosition = CGPointMake(pos.x, pos.y);
    CGWarpMouseCursorPosition(cgPosition);
}

AFuture<APath> ADesktop::browseForDir(AWindowBase* parent, const APath& startingLocation) {
    AUI_NULLSAFE(parent)->blockUserInput();
    AUI_DEFER { AUI_NULLSAFE(parent)->blockUserInput(false); };

    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:NO];
        [panel setCanChooseDirectories:YES];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanCreateDirectories:YES];

        if (startingLocation.exists()) {
            NSString* path = [NSString stringWithUTF8String:startingLocation.c_str()];
            [panel setDirectoryURL:[NSURL fileURLWithPath:path]];
        }

        NSWindow* nativeParentWindow = nil;
        if (auto d = dynamic_cast<AWindow*>(parent)) {
            nativeParentWindow = static_cast<NSWindow*>(d->nativeHandle());
        }

        __block NSModalResponse response = NSModalResponseCancel;

        if (nativeParentWindow) {
            [panel
                beginSheetModalForWindow:nativeParentWindow
                       completionHandler:^(NSModalResponse result) {
                         response = result;
                         [NSApp stopModal];
                       }];
            [NSApp runModalForWindow:panel];
        } else {
            response = [panel runModal];
        }

        if (response == NSModalResponseOK) {
            NSURL* selectedFolder = [[panel URLs] objectAtIndex:0];
            return AFuture(APath([[selectedFolder path] UTF8String]));
        }
    }

    return AFuture(APath());
}

AFuture<APath>
ADesktop::browseForFile(AWindowBase* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    AUI_NULLSAFE(parent)->blockUserInput();
    AUI_DEFER { AUI_NULLSAFE(parent)->blockUserInput(false); };

    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanCreateDirectories:NO];

        if (startingLocation.exists()) {
            NSString* path = [NSString stringWithUTF8String:startingLocation.c_str()];
            [panel setDirectoryURL:[NSURL fileURLWithPath:path]];
        }

        if (!extensions.empty()) {
#if AUI_MACOS_SUPPORTS_CONTENTTYPES == 1
            NSMutableArray<UTType*>* allowedTypes = [NSMutableArray array];
            for (const auto& ext : extensions) {
                if (ext.extension == "*") {
                    [allowedTypes removeAllObjects];
                    break;
                }

                UTType* type =
                    [UTType typeWithFilenameExtension:[NSString stringWithUTF8String:ext.extension.c_str()]];

                [allowedTypes addObject:type];
            }

            [panel setAllowedContentTypes:allowedTypes];
#else
            NSMutableArray<NSString*>* allowedExtensions = [NSMutableArray array];
            for (const auto& ext : extensions) {
                if (ext.extension == "*") {
                    [allowedExtensions removeAllObjects];
                    break;
                }

                [allowedExtensions addObject:[NSString stringWithUTF8String:ext.extension.c_str()]];
            }

            [panel setAllowedFileTypes:allowedExtensions];
#endif
        }

        NSWindow* nativeParentWindow = nil;
        if (auto d = dynamic_cast<AWindow*>(parent)) {
            nativeParentWindow = static_cast<NSWindow*>(d->nativeHandle());
        }

        __block NSModalResponse response = NSModalResponseCancel;

        if (nativeParentWindow) {
            [panel
                beginSheetModalForWindow:nativeParentWindow
                       completionHandler:^(NSModalResponse result) {
                         response = result;
                         [NSApp stopModal];
                       }];
            [NSApp runModalForWindow:panel];
        } else {
            response = [panel runModal];
        }

        if (response == NSModalResponseOK) {
            NSURL* selectedFolder = [[panel URLs] objectAtIndex:0];
            return AFuture(APath([[selectedFolder path] UTF8String]));
        }
    }

    return AFuture(APath());
}

_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    NSString* filePath = [NSString stringWithUTF8String:file.c_str()];

    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    NSImage* icon = [workspace iconForFile:filePath];

    if (icon) {
        NSBitmapImageRep* bitmapRep = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:nil
                          pixelsWide:icon.size.width
                          pixelsHigh:icon.size.height
                       bitsPerSample:8
                     samplesPerPixel:4
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                         bytesPerRow:0
                        bitsPerPixel:0];

        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:bitmapRep]];

        [icon
            drawInRect:NSMakeRect(0, 0, icon.size.width, icon.size.height)
              fromRect:NSZeroRect
             operation:NSCompositingOperationCopy
              fraction:1.0];

        [NSGraphicsContext restoreGraphicsState];

        unsigned char* rawData = [bitmapRep bitmapData];
        NSUInteger dataLength = icon.size.width * icon.size.height * 4;   // 4 bytes per pixel (RGBA)

        AImage image(
            AByteBufferView(reinterpret_cast<const char*>(rawData), dataLength),
            glm::uvec2(icon.size.width, icon.size.height), APixelFormat::RGBA | APixelFormat::BYTE);

        return _new<AImageDrawable>(_new<AImage>(image));
    }

    return nil;
}

void ADesktop::playSystemSound(ADesktop::SystemSound s) {
    @autoreleasepool {
        switch (s) {
            case ADesktop::SystemSound::ASTERISK:
                [[NSSound soundNamed:@"Basso"] play];
                break;
            case ADesktop::SystemSound::QUESTION:
                [[NSSound soundNamed:@"Ping"] play];
                break;
        }
    }
}
