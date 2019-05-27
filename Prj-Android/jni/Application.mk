APP_BUILD_SCRIPT := $(call my-dir)/toolchain/Android.mk
APP_MODULES := anyrtmp-jni
#APP_ABI := armeabi armeabi-v7a x86

NDK_PATH := H:\DEVTOOLS\ide\android-sdk\ndk-bundle #C:\Users\root\AppData\Local\Android\Sdk\ndk-bundle
#NDK_STL_INC := $(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.9/include

APP_OPTIM        := debug #release
APP_CFLAGS   += -O0 -g -D_DEBUG    #+= -O3
#APP_STL := gnustl_shared
#APP_STL := gnustl_static
APP_STL := c++_static
#NDK_TOOLCHAIN_VERSION = 4.9

APP_PLATFORM := android-24