# aui_app

Provide application info for deployment (i.e. name, author, license, icon, etc...).

## Syntax

```cmake
aui_app(
    # common
    [TARGET <target-name>]
    [NAME <application-name>]
    [ICON <path-to-icon-svg>]
    [VENDOR <vendor-name>]
    [COPYRIGHT <copyright-string>]
    [VERSION <version>]

    [NO_INCLUDE_CPACK]

    # linux only
    [LINUX_DESKTOP <path-to-desktop-file>]
    
    # apple only
    [APPLE_TEAM_ID <apple-team-id>]
    [APPLE_BUNDLE_IDENTIFIER <bundle-identifier>]
    [APPLE_SIGN_IDENTITY <sign-identity>]

    # ios only
    [IOS_VERSION <target-ios-version>]
    [IOS_DEVICE <target-ios-device>]
    [IOS_CONTROLLER <custom-ios-controller-class-name>]
)
```

### TARGET

Specify main executable of the application which will be executed when your application is launched.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|+||
|Linux|+||
|macOS|+||
|Android|+|does not matter|
|iOS|+||

### NAME

Specify application display name.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|+||
|Linux|+||
|macOS|+||
|Android|+||
|iOS|+||

### ICON

Specify SVG icon location (defaults to `icon.svg` if exists).


|Platform|Required|Traits|
|--------|--------|------|
|Windows|||
|Linux|+||
|macOS|+|generates `*.icns` file from your `svg`|
|Android|+||
|iOS|+||

### VENDOR

Specify application author's name or company.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|||
|Linux|||
|macOS|||
|Android|||
|iOS|||

### COPYRIGHT

Specify application copyright string (not file)

|Platform|Required|Traits|
|--------|--------|------|
|Windows|||
|Linux||does not matter|
|macOS|+||
|Android|||
|iOS|+||


### VERSION

Specify application version. If not set, the value of `PROJECT_VERSION` used instead.

|Platform|Required|Traits|
|--------|--------|------|
|Windows|+||
|Linux|+||
|macOS|+||
|Android|+||
|iOS|+||

### NO_INCLUDE_CPACK

By default `aui_app` includes CPack in order to allow to generate packages using CPack. You can disable this behaviour by specifying `NO_INCLUDE_CPACK` flag.

### LINUX_DESKTOP

Specify custom `*.desktop` file. If not set, `*.desktop` file is generated from `aui_app` arguments.

|Platform|Required|Traits|
|--------|--------|------|
|Linux|+||


### APPLE_TEAM_ID

Specify your Apple Team ID (https://discussions.apple.com/thread/7942941).

|Platform|Required|Traits|
|--------|--------|------|
|macOS|+||
|iOS|+||

### APPLE_BUNDLE_IDENTIFIER

Specify bundle identifier. If not set, the value of `NAME` used instead.

|Platform|Required|Traits|
|--------|--------|------|
|macOS|||
|iOS|||

### APPLE_SIGN_IDENTITY

Specify Apple sign identity. Defaults to "iPhone Developer".

|Platform|Required|Traits|
|--------|--------|------|
|macOS||unused|
|iOS|||

### IOS_VERSION

Specify target iOS platform. Defaults to "14.3".

|Platform|Required|Traits|
|--------|--------|------|
|iOS|||

### IOS_DEVICE

Specify target iOS device. Applicable values are `IPHONE`, `IPAD` or `BOTH`. Defaults to `BOTH`.

|Platform|Required|Traits|
|--------|--------|------|
|iOS|||

### IOS_CONTROLLER

Specify custom view controller class name. Defaults to `AUIViewController`.

|Platform|Required|Traits|
|--------|--------|------|
|iOS|||
