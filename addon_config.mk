meta:
	ADDON_NAME = ofxPvAPI
	ADDON_DESCRIPTION = Interface with Allied Vision cameras on OSX using the Legacy SDK PvAPI
	ADDON_AUTHOR = M. Oostrik
	ADDON_TAGS = "camera" "gigE" "Allied Vision"
	ADDON_URL = https://github.com/moostrik/ofxPvAPI

common:
	ADDON_INCLUDES = src
	ADDON_INCLUDES += libs/PvAPI/include
	ADDON_CPPFLAGS = -D_x64

linux64:
	ADDON_LIBS = libs/PvAPI/lib/linux/libPvAPI.a
	ADDON_LDFLAGS = -lrt
	ADDON_CPPFLAGS += -D_LINUX

osx:
	ADDON_CPPFLAGS += -D_OSX

