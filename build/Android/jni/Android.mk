# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH			:= $(call my-dir)
GUI_PATH            := ../../../imGui
SRC_PATH			:= ../../../src
INC_PATH			:= ../../../inc
GLOB_INC_PATH		:= ../../../globalInc
FRM_PATH			:= ../../../AdrenoFramework
FRM_OPENGL_PATH		:= $(FRM_PATH)/OpenGLES
FRM_ANDROID_PATH	:= $(FRM_PATH)/Android


include $(CLEAR_VARS)

LOCAL_MODULE    := RenderToTextureOGLES30

LOCAL_SRC_FILES := $(SRC_PATH)/Scene.cpp \
				   $(FRM_OPENGL_PATH)/FrmFontGLES.cpp \
				   $(FRM_OPENGL_PATH)/FrmMesh.cpp \
				   $(FRM_OPENGL_PATH)/FrmPackedResourceGLES.cpp \
				   $(FRM_OPENGL_PATH)/FrmResourceGLES.cpp \
				   $(FRM_OPENGL_PATH)/FrmShader.cpp \
				   $(FRM_OPENGL_PATH)/FrmUserInterfaceGLES.cpp \
				   $(FRM_OPENGL_PATH)/FrmUtilsGLES.cpp \
				   $(FRM_PATH)/FrmApplication.cpp \
				   $(FRM_PATH)/FrmFont.cpp \
				   $(FRM_PATH)/FrmInput.cpp \
				   $(FRM_PATH)/FrmPackedResource.cpp \
				   $(FRM_PATH)/FrmUserInterface.cpp \
				   $(FRM_PATH)/FrmUtils.cpp \
				   $(FRM_ANDROID_PATH)/FrmApplication_Platform.cpp \
				   $(FRM_ANDROID_PATH)/FrmFile_Platform.cpp \
				   $(FRM_ANDROID_PATH)/FrmStdlib_Platform.cpp \
				   $(FRM_ANDROID_PATH)/FrmUtils_Platform.cpp  \
				   $(GUI_PATH)/imgui.cpp  \
				   $(GUI_PATH)/imgui_draw.cpp  \
				   $(GUI_PATH)/imgui_demo.cpp  \
				   $(GUI_PATH)/imgui_impl_android_gl3.cpp  \
				   
				   

LOCAL_C_INCLUDES	:= $(SRC_PATH) \
					   $(INC_PATH) \
					   $(GLOB_INC_PATH) \
					   $(FRM_PATH)	\
					   $(FRM_OPENGL_PATH) \
					   $(FRM_ANDROID_PATH)  \
					   $(GUI_PATH)   \
				   
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv3

LOCAL_CFLAGS	+= -Wno-write-strings -Wno-conversion-null

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
