LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := HitBoxes

# Флаги компиляции
LOCAL_CFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CFLAGS += -fno-rtti -fexceptions -fpermissive -Oz

LOCAL_CPPFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CPPFLAGS += -fno-rtti -fexceptions -fpermissive -Oz -std=c++17
LOCAL_CPPFLAGS += -Wno-c++17-narrowing -fms-extensions -DNDEBUG

# Линковка логов Андроида
LOCAL_LDLIBS += -llog
LOCAL_LDFLAGS += -Wl,--gc-sections -Wl,--strip-all

# --- [ЖЕЛЕЗОБЕТОННЫЙ МЕТОД] Передаем пути инклудов напрямую компилятору Clang ---
LOCAL_CFLAGS += -I$(LOCAL_PATH)
LOCAL_CFLAGS += -I$(LOCAL_PATH)/KittyMemory
LOCAL_CFLAGS += -I$(LOCAL_PATH)/dobby

LOCAL_CPPFLAGS += -I$(LOCAL_PATH)
LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/KittyMemory
LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/dobby

# Стандартный поиск заголовочных файлов для NDK
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/KittyMemory \
                    $(LOCAL_PATH)/dobby

# Явное перечисление исходников, чтобы исключить любые ошибки файловой системы GitHub Actions
LOCAL_SRC_FILES := main.cpp \
                   KittyMemory/KittyMemory.cpp \
                   KittyMemory/KittyArm64.cpp \
                   KittyMemory/KittyPtrValidator.cpp \
                   KittyMemory/KittyScanner.cpp

include $(BUILD_SHARED_LIBRARY)
