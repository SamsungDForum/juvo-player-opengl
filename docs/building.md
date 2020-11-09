How to build?
=======

## Prerequisites

libgles.so can be compiled with mobile Tizen toolchain available in Tizen Studio.
This article explains how to compile a native library with Tizen Studio CLI, but Tizen Studio IDE may be used as well.

1. Tizen Studio can be downloaded from https://developer.tizen.org/development/tizen-studio/download.
   Please follow installation guides available here: https://developer.tizen.org/development/tizen-studio/download/installing-tizen-studio.
2. Install MOBILE-4.0-NativeAppDevelopment-CLI package:
    ```
    ${tizen-studio}/package-manager/package-manager-cli.bin install MOBILE-4.0-NativeAppDevelopment-CLI
    ```
   'tizen-studio' is a absolute path to directory where you installed Tizen Studio.

## Building

All build commands need to be executed from the project root directory. This directory contains the project_def.prop file,
which is used by 'tizen build-native' script to build a native code.

Debug configuration:
```
${tizen-studio}/tools/ide/bin/tizen build-native -a arm -c llvm -C Debug -r mobile-4.0-device.core
```

Release configuration:
```
${tizen-studio}/tools/ide/bin/tizen build-native -a arm -c llvm -C Release -r mobile-4.0-device.core
```

libgles.so will be available in {Debug/Release} depending on build configuration.

Note:
The name of 'tizen' command is 'tizen.bat' on Windows OS.

TODO: Check emulator build.

## Supported TVs

The ability to execute a native code compiled by Tizen mobile toolchain is allowed on 2021 Tizen TV models and later.
2020 and previous models are not supported.
