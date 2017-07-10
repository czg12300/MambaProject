LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := soundtouch
LOCAL_SRC_FILES := source/AAFilter.cpp  \
                   source/FIFOSampleBuffer.cpp \
                   source/FIRFilter.cpp \
                   source/cpu_detect_x86.cpp \
                   source/sse_optimized.cpp \
                   source/RateTransposer.cpp \
                   source/SoundTouch.cpp \
                   source/InterpolateCubic.cpp  \
                   source/InterpolateLinear.cpp \
                   source/InterpolateShannon.cpp \
                   source/TDStretch.cpp \
                   source/BPMDetect.cpp \
                   source/mmx_optimized.cpp \
                   source/PeakFinder.cpp



LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
# Use ARM instruction set instead of Thumb for improved calculation performance in ARM CPUs
#LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
