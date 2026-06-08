#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>

// Подключаем Dobby и KittyMemory, которые прописаны в Android.mk
#include "dobby.h"
#include "KittyMemory/KittyMemory.h"

// Добавляем твой заголовочный файл Utils
#include "Utils.h"

#define LOG_TAG "HITBOX_MOD"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Указатель на оригинальную функцию игры
void (*orig_SendCommand)(const char* text) = nullptr;

// Наша функция-хук, которая перехватывает ввод игрока
void new_SendCommand(const char* text) {
    if (text != nullptr) {
        // Пишем в логи абсолютно всё, что ввёл игрок (для тестов)
        LOGI("Игрок отправил в чат/консоль: %s", text);

        // 1. Обработка команды /sizehb
        if (strstr(text, "sizehb") != nullptr) {
            LOGI("==== Сработал перехват команды /sizehb! ====");
            
            /* ЗДЕСЬ НАПИШИ СВОЙ КОД ДЛЯ ИЗМЕНЕНИЯ РАЗМЕРА ХИТБОКСОВ.
               Здесь ты уже можешь использовать функции из Utils.h, если они там есть.
            */
            
            // ВАЖНО: возвращаем пустоту, чтобы игра не писала "Команда не найдена"
            return; 
        }

        // 2. Обработка команды /hbinfo
        if (strstr(text, "hbinfo") != nullptr) {
            LOGI("==== Сработал перехват команды /hbinfo! ====");
            
            /* ЗДЕСЬ НАПИШИ СВОЙ КОД ДЛЯ ВЫВОДА ИНФОРМАЦИИ. */
            
            return; // Блокируем ошибку игры
        }
    }

    // Если это была любая другая обычная команда или сообщение — отдаем её игре обратно
    if (orig_SendCommand != nullptr) {
        orig_SendCommand(text);
    }
}

// Поток, который ждет загрузки игры и ставит хук
void* hack_thread(void*) {
    proc_info libInfo;
    
    // Ждем, пока Блек Раша загрузит свой основной бинарник в память.
    while (!KittyMemory::getLibraryProcInfo("libSAMP.so", libInfo)) {
        sleep(1);
    }
    
    uintptr_t libBase = libInfo.base;
    LOGI("Библиотека игры найдена по адресу: 0x%lx", libBase);

    // !!! ВНИМАНИЕ: Замени 0x123456 на ТВОЙ реальный оффсет функции SendCommand !!!
    uintptr_t target_offset = 0x123456; 
    void* target_address = (void*)(libBase + target_offset);

    // Ставим хук с помощью Dobby
    if (DobbyHook(target_address, (void*)new_SendCommand, (void**)&orig_SendCommand) == 0) {
        LOGI("Хук Dobby на команды УСПЕШНО установлен!");
    } else {
        LOGE("ОШИБКА: Dobby не смог примениться к оффсету 0x%lx", target_offset);
    }

    return nullptr;
}

// Этот блок автоматически сработает при инжекте .so файла в игру
__attribute__((constructor)) void init() {
    pthread_t t;
    pthread_create(&t, nullptr, hack_thread, nullptr);
}
