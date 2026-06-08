LOCAL_PATH := $(call my-dir)
MAIN_LOCAL_PATH := $(call my-dir)

# ====================================================================
# 1. Объявляем предкомпилированную статическую библиотеку Dobby
# ====================================================================
include $(CLEAR_VARS)
LOCAL_MODULE := dobby
# $(TARGET_ARCH_ABI) автоматически подставит arm64-v8a или armeabi-v7a в зависимости от сборки
LOCAL_SRC_FILES := dobby/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)


# ====================================================================
# 2. Твой основной модуль чит-плагина
# ====================================================================
include $(CLEAR_VARS)

[span_2](start_span)LOCAL_MODULE := HitBoxes[span_2](end_span)

[span_3](start_span)LOCAL_CFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w[span_3](end_span)
[span_4](start_span)LOCAL_CFLAGS += -fno-rtti -fexceptions -fpermissive -Oz[span_4](end_span)

[span_5](start_span)LOCAL_CPPFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w[span_5](end_span)
[span_6](start_span)LOCAL_CPPFLAGS += -fno-rtti -fexceptions -fpermissive -Oz -std=c++17[span_6](end_span)
[span_7](start_span)LOCAL_CPPFLAGS += -Wno-c++17-narrowing -fms-extensions -DNDEBUG[span_7](end_span)

# Добавляем логирование и статическую линковку модуля dobby
[span_8](start_span)LOCAL_LDFLAGS += -Wl,--gc-sections -Wl,--strip-all -llog[span_8](end_span)
LOCAL_STATIC_LIBRARIES += dobby

[span_9](start_span)FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)[span_9](end_span)
[span_10](start_span)FILE_LIST += $(wildcard $(LOCAL_PATH)/*.c)[span_10](end_span)
[span_11](start_span)FILE_LIST += $(wildcard $(LOCAL_PATH)/KittyMemory/*.cpp)[span_11](end_span)

[span_12](start_span)LOCAL_C_INCLUDES += $(wildcard $(LOCAL_PATH)/)[span_12](end_span)
[span_13](start_span)LOCAL_C_INCLUDES += $(wildcard $(LOCAL_PATH)/KittyMemory/)[span_13](end_span)
# Добавляем путь к заголовочному файлу dobby.h
LOCAL_C_INCLUDES += $(LOCAL_PATH)/dobby

[span_14](start_span)LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)[span_14](end_span)

[span_15](start_span)include $(BUILD_SHARED_LIBRARY)[span_15](end_span)
