LOCAL_PATH := $(call my-dir)

# ====================================================================
# 1. Подключаем готовую 64-битную библиотеку Dobby
# ====================================================================
include $(CLEAR_VARS)
LOCAL_MODULE := dobby
# Путь обновлен под твою новую структуру папок
LOCAL_SRC_FILES := dobby/arm64-v8a/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

# ====================================================================
# 2. Сборка твоего чит-плагина хитбоксов
# ====================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := HitBoxes

LOCAL_CFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w -Oz
LOCAL_CPPFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w -Oz -std=c++17 -DNDEBUG

LOCAL_LDFLAGS += -Wl,--gc-sections -Wl,--strip-all -llog
LOCAL_STATIC_LIBRARIES += dobby

FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/KittyMemory/*.cpp)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/KittyMemory/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/dobby

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_SHARED_LIBRARY)
