# README #

Based on a previously private repository for openframeworks <= 8.4.
The static libPvAPI.a will not compile with OF 0.9.x. Fortunately the  libPvAPI.dylib does work when properly added to the target.
The PvApi.h must be slightly modified

Tested with the Prosilica GC750 (monochrome) and the on OSX 10.13
previous versions tested with Mako G-125C (color) and on Windows 7


# TODO #

- [x] fix tearing
- [x] make threaded
- [x] figure out how to include libPvAPI.dylib in app
- [ ] refactor camFPS
- [ ] fix slowdown of setAllParametersFromCam in parameterConnector
- [ ] add flip and rotate without texture
- [ ] better naming of classes
- [ ] test windows
- [ ] make linux


# FOR MAC #

1.	Add `ofxPvApi` to your addons folder
2.	Download the [legacy SDK](https://www.alliedvision.com/fileadmin/content/software/software/PvAPI/PvAPI_1.28_OSX.tgz "PvAPI_1.28_OSX.tgz") from [Allied Vision](https://www.alliedvision.com/en/support/software-downloads.html "Software Downloads") and copy the following files from the SDK to the addon.
*	 `/bin-pc/x64/4.2/libPvAPI.dylib` (or  `/bin-pc/x86/4.2/libPvAPI.dylib` for 32 bit) into `/ofxPvApi/libs/PvAPI/lib/osx/`
*	 `/inc-pc/PvApi.h` into `/ofxPvApi/libs/PvAPI/include`
3. 	modify `PvApi.h` to include
```
    #ifdef TARGET_OSX
    #define _OSX
    #define _x64
    #endif
```
*	before `#ifndef PVAPI_H_INCLUDE` (line 79) will do fine
*	as far as I could determen `#define _x86`  does the same as  `#define _x64`
4. 	Create examples using the projectGenerator
5.	include libPvAPI.dylib in the examples (or your app) by adding the following lines to Project -> Build Phases -> Run Script
```
    # Copy libPvAPI and change install directory for fmod to run
    rsync -aved ../../../addons/ofxPvApi/libs/PvAPI/lib/osx/libPvAPI.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Frameworks/";
    install_name_tool -change libPvAPI.dylib @executable_path/../Frameworks/libPvAPI.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";
```
*	When adding the addon manually in Xcode make sure to add the libPvAPI.dylib to Project -> Build Settings -> Other Linker Flags `../../../addons/ofxPvApi/libs/PvAPI/lib/osx/libPvAPI.dylib`
6. 	Turn off the Firewall
7. 	~~Set MTU to Jumbo / 9000 (System Preferences -> Network -> Ethernet -> Advanced -> Hardware -> Configure Manually -> MTU)~~


# KNOWN ISSUES AND FACTS #

*	Turn off the Firewall!
*	when setting persistentIP to false, unplug and replug the the camera.
*	when setting persistentIP to true make sure you know what you are doing.
*	the default pixel format is mono, deal with it.
*	according to the driver attribute document the autoGain should work together with the autoExposure for best lightness result. In my experiments using them simultanious gives weird results.
*	the example has no initial settings.XML file, save (right upper corner of the ofxGui) to create one.
*	The cameras don't work great on wifi, but on low resolutions they might work.
*	Sometimes it helps to turn of the Wifi


# FOR WINDOWS #
PvAPI SDK for Windows  v1.28
NOT TESTED FOR OF 0.9 / Win 10, but procedure should be similar

1.	Add `ofxPvApi` to your addons folder
2.	Download the [legacy SDK](https://www.alliedvision.com/fileadmin/content/software/software/PvAPI/PvAPI_win_1.28.exe "PvAPI SDK for Windows  v1.28") from  [Allied Vision](https://www.alliedvision.com/en/support/software-downloads.html "Software Downloads") and copy the following files from the SDK to the addon.
*	`/inc-pc/PvApi.h` into `/ofxPvApi/libs/PvAPI/include`
*	`/lib-pc/PvAPI.lib` into `/ofxPvApi/libs/PvAPI/lib/win32/`
*	`PvAPI.dll` into `/ofxPvApi/libs/PvAPI/lib/win32/`
3. 	IN Properties -> C/C++ -> additional include directories ADD
*	`..\..\..\addons\ofxPvApi\src`
*	`..\..\..\addons\ofxPvApi\libs\PvAPI\include`
4.	~~IN Properties -> Linker -> Additional Lybrary Directories ADD~~
*	~~* ..\..\..\addons\ofxPvApi\libs\PvAPI\lib\win32~~
*	For some reason this was not necessary, but i had to add the PvAPI.lib to the solution explorer

5.	copy `PvAPI.dll`: IN Build Events -> Post Build Event - > Command Line ADD
	`xcopy /y "$(ProjectDir)..\..\..\addons\ofxPvApi\libs\PvAPI\lib\win32\PvAPI.dll"  "$(ProjectDir)bin"`
6.	Turn off the Firewall (cameras don't work otherwise)

