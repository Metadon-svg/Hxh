#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>

// Подключаем утилиты, Dobby для хуков и KittyMemory для записи в память либки
#include "Utils.h"
#include "dobby.h"
#include "KittyMemory/KittyMemory.h"

#define LOG_TAG "BR_HITBOX_MOD"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Глобальные переменные
void* pChatInstance = nullptr;
uintptr_t g_libBase = 0; // Базовый адрес libblackrussia-client.so

// Указатели на оригинальные функции игры
void (*orig_ChatVM_AddMessage)(void* thiz, uint32_t color, const char* text, void* extra) = nullptr;
void (*orig_sendChatMessage)(JNIEnv* env, jobject thiz, jstring text) = nullptr;


// =============================================================================
// [ДИНАМИЧЕСКИЙ ПАТЧ ХИТБОКСОВ] Твои оригинальные оффсеты костей из PedModelInfo
// =============================================================================
void ApplyHitboxChanges(float MultiplyValue) {
    if (g_libBase == 0) {
        LOGI("Ошибка: Базовый адрес библиотеки равен 0. Патч невозможен.");
        return;
    }

    // Рассчитываем смещения для костей в зависимости от разрядности (64 / 32 бит)
    #if defined(__aarch64__)
        uintptr_t HEAD       = 0x141DF68;
        uintptr_t TORSO_1    = HEAD + 0x20;
        uintptr_t TORSO_2    = TORSO_1 + 0x20;
        uintptr_t MID        = TORSO_2 + 0x20;
        uintptr_t LEFTARM    = MID + 0x20;
        uintptr_t RIGHTARM   = LEFTARM + 0x20;
        uintptr_t LEFTLEG_1  = RIGHTARM + 0x20;
        uintptr_t RIGHTLEG_1 = LEFTLEG_1 + 0x20;
        uintptr_t LEFTLEG_2  = RIGHTLEG_1 + 0x20;
        uintptr_t RIGHTLEG_2 = LEFTLEG_2 + 0x20;
    #else
        uintptr_t HEAD       = 0x141DF68;
        uintptr_t TORSO_1    = HEAD + 0x18;
        uintptr_t TORSO_2    = TORSO_1 + 0x18;
        uintptr_t MID        = TORSO_2 + 0x18;
        uintptr_t LEFTARM    = MID + 0x18;
        uintptr_t RIGHTARM   = LEFTARM + 0x18;
        uintptr_t LEFTLEG_1  = RIGHTARM + 0x18;
        uintptr_t RIGHTLEG_1 = LEFTLEG_1 + 0x18;
        uintptr_t LEFTLEG_2 = RIGHTLEG_1 + 0x18;
        uintptr_t RIGHTLEG_2 = LEFTLEG_2 + 0x18;
    #endif

    // Вычисляем новые размеры на основе переданного MultiplyValue
    float v_HEAD       = 0.15f * MultiplyValue;
    float v_TORSO_1    = 0.2f  * MultiplyValue;
    float v_TORSO_2    = 0.25f * MultiplyValue;
    float v_MID        = 0.25f * MultiplyValue;
    float v_LEFTARM    = 0.16f * MultiplyValue;
    float v_RIGHTARM   = 0.16f * MultiplyValue;
    float v_LEFTLEG_1  = 0.2f  * MultiplyValue;
    float v_RIGHTLEG_1 = 0.2f  * MultiplyValue;
    float v_LEFTLEG_2  = 0.15f * MultiplyValue;
    float v_RIGHTLEG_2 = 0.15f * MultiplyValue;

    // Безопасно пишем новые float значения в память libblackrussia-client.so
    KittyMemory::writeMemory((void*)(g_libBase + HEAD),       &v_HEAD,       sizeof(v_HEAD));
    KittyMemory::writeMemory((void*)(g_libBase + TORSO_1),    &v_TORSO_1,    sizeof(v_TORSO_1));
    KittyMemory::writeMemory((void*)(g_libBase + TORSO_2),    &v_TORSO_2,    sizeof(v_TORSO_2));
    KittyMemory::writeMemory((void*)(g_libBase + MID),        &v_MID,        sizeof(v_MID));
    KittyMemory::writeMemory((void*)(g_libBase + LEFTARM),    &v_LEFTARM,    sizeof(v_LEFTARM));
    KittyMemory::writeMemory((void*)(g_libBase + RIGHTARM),   &v_RIGHTARM,   sizeof(v_RIGHTARM));
    KittyMemory::writeMemory((void*)(g_libBase + LEFTLEG_1),  &v_LEFTLEG_1,  sizeof(v_LEFTLEG_1));
    KittyMemory::writeMemory((void*)(g_libBase + RIGHTLEG_1), &v_RIGHTLEG_1, sizeof(v_RIGHTLEG_1));
    KittyMemory::writeMemory((void*)(g_libBase + LEFTLEG_2),  &v_LEFTLEG_2,  sizeof(v_LEFTLEG_2));
    KittyMemory::writeMemory((void*)(g_libBase + RIGHTLEG_2), &v_RIGHTLEG_2, sizeof(v_RIGHTLEG_2));

    LOGI("Размеры хитбоксов успешно изменены. Множитель: %.2f", MultiplyValue);
}


// =============================================================================
// [РАБОТА С ЧАТОМ GUI] Перехват указателя и отправка локальных сообщений
// =============================================================================
void new_ChatVM_AddMessage(void* thiz, uint32_t color, const char* text, void* extra) {
    if (pChatInstance == nullptr) {
        pChatInstance = thiz; // Вылавливаем структуру чата Noesis GUI из игры
        LOGI("Указатель на ChatVM успешно получен: %p", pChatInstance);
    }
    orig_ChatVM_AddMessage(thiz, color, text, extra);
}

void SendLocalMessage(const char* message) {
    if (pChatInstance != nullptr && orig_ChatVM_AddMessage != nullptr) {
        // Выводим текст белым цветом, внутри строки поддерживаются {HEX} цвета SAMP
        orig_ChatVM_AddMessage(pChatInstance, 0xFFFFFFFF, message, nullptr);
    } else {
        LOGI("Лог (чат еще не инициализирован): %s", message);
    }
}


// =============================================================================
// [ПЕРЕХВАТ ВВОДА ИГРОКА] Обработка команд /sizehb и /hbinfo
// =============================================================================
void new_sendChatMessage(JNIEnv* env, jobject thiz, jstring text) {
    if (text != nullptr) {
        const char* nativeString = env->GetStringUTFChars(text, nullptr);
        
        if (nativeString != nullptr) {
            LOGI("Игрок ввел: %s", nativeString);

            // Обработка команды /sizehb [значение]
            if (Utils::IsCommand(nativeString, "/sizehb")) {
                // Если игрок ввел просто /sizehb — ставим дефолтный множитель 1.5
                float multiply = Utils::GetFloatArg(nativeString, 1.5f);

                // Вызываем наш блок изменения памяти с твоими оффсетами костей
                ApplyHitboxChanges(multiply);
                
                // Отправляем красивый локальный репорт в чат
                char messageBuffer[128];
                snprintf(messageBuffer, sizeof(messageBuffer), "{00FF00}[HitBox Mod] {FFFFFF}Множитель хитбоксов изменен на: {FFFF00}%.2f", multiply);
                SendLocalMessage(messageBuffer);
                
                env->ReleaseStringUTFChars(text, nativeString);
                return; // Блокируем отправку текста, чтобы сервер и админы ничего не узнали
            }

            // Обработка команды /hbinfo
            if (Utils::IsCommand(nativeString, "/hbinfo")) {
                SendLocalMessage("{00FF00}[HitBox Mod] {FFFFFF}Статус: {00FF00}Активен {FFFFFF}| База: libblackrussia-client.so");
                env->ReleaseStringUTFChars(text, nativeString);
                return; // Тоже блокируем
            }

            env->ReleaseStringUTFChars(text, nativeString);
        }
    }

    // Обычные сообщения и чужие команды пропускаем на сервер без изменений
    orig_sendChatMessage(env, thiz, text);
}


// =============================================================================
// [ИНИЦИАЛИЗАЦИЯ] Поток установки хуков
// =============================================================================
void* hack_thread(void*) {
    proc_info libInfo;
    
    // Ждём, пока Блек Раша загрузит в память свой клиентский файл
    while (!KittyMemory::getLibraryProcInfo("libblackrussia-client.so", libInfo)) {
        sleep(1);
    }
    
    g_libBase = libInfo.base;
    LOGI("Библиотека libblackrussia-client.so найдена по адресу: %p", (void*)g_libBase);

    // Твои проверенные оффсеты функций из Radare2
    uintptr_t offset_sendChatMessage = 0x00cddf04; // Перехват строки ввода
    uintptr_t offset_AddMessage      = 0x008664f8; // Точка вывода текста в чат GUI

    // Ставим хуки через Dobby
    DobbyHook((void*)(g_libBase + offset_sendChatMessage), (void*)new_sendChatMessage, (void**)&orig_sendChatMessage);
    DobbyHook((void*)(g_libBase + offset_AddMessage), (void*)new_ChatVM_AddMessage, (void**)&orig_ChatVM_AddMessage);

    LOGI("Все хуки на Блек Рашу успешно установлены!");
    return nullptr;
}

// Конструктор, вызываемый при загрузке .so инжектором
__attribute__((constructor)) void init() {
    pthread_t t;
    pthread_create(&t, nullptr, hack_thread, nullptr);
}
