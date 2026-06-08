#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <map>
#include <functional>
#include <pthread.h>
#include <unistd.h>
#include <jni.h>

// Подключаем Dobby Hook для инлайн-хуков
#include "dobby.h" 
#include "Utils.h"

using namespace std;

#define libName "libblackrussia-client.so"

// --- Глобальные переменные состояния ---
JavaVM* g_JavaVM = nullptr;
bool hitboxesEnabled = true;
float MultiplyValue = 1.5f;

// --- Настройки смещений хитбоксов ---
#if defined(__aarch64__)
    uintptr_t HEAD = 0x141DF68;
    uintptr_t STEP = 0x20;
#else
    uintptr_t HEAD = 0x141DF68;
    uintptr_t STEP = 0x18;
#endif

// --- СИСТЕМА РЕГИСТРАЦИИ КОМАНД ---
using CmdCallback = std::function<void(const std::string&)>;
std::map<std::string, CmdCallback> cmdRegistry;

void RegisterCMD(const std::string& cmdName, CmdCallback callback) {
    cmdRegistry[cmdName] = callback;
}

// Инициализация всех команд софта
void SetupCommands() {
    
    // Команда /sizehb [значение / off]
    RegisterCMD("/sizehb", [](const std::string& args) {
        if (args == "off" || args.empty()) {
            hitboxesEnabled = false;
            // Локальный вывод добавишь сюда, когда найдешь адрес AddMessageToChat
            return;
        }

        char* endptr;
        float val = strtof(args.c_str(), &endptr);

        // Проверяем, число ли введено
        if (endptr != args.c_str()) {
            if (val > 5.0f) val = 5.0f;
            if (val < 1.0f) val = 1.0f;

            MultiplyValue = val;
            hitboxesEnabled = true;
            // Фоновый поток main_thread мгновенно применит новый MultiplyValue
        }
    });

    // Инфо-команда
    RegisterCMD("/hbinfo", [](const std::string& args) {
        // Локальный вывод информации о разработчике
    });
}

// Парсер и обработчик ввода
bool ProcessPlayerCommand(const std::string& input) {
    if (input.empty() || input[0] != '/') return false;

    size_t spacePos = input.find(' ');
    std::string cmd = (spacePos == std::string::npos) ? input : input.substr(0, spacePos);
    std::string args = (spacePos == std::string::npos) ? "" : input.substr(spacePos + 1);

    auto it = cmdRegistry.find(cmd);
    if (it != cmdRegistry.end()) {
        it->second(args); 
        return true; // Возвращаем true, чтобы заблокировать отправку текста на сервер
    }

    return false; // Если команда не наша (например /report, /time) — отдаем игре
}

// --- ХУК НА ОТПРАВКУ ТЕКСТА ---
// Указатель на оригинальную JNI-функцию отправки сообщений игры
void (*orig_sendChatMessage)(JNIEnv* env, jobject thiz, jstring text) = nullptr;

// Наша функция-перехватчик
void hook_sendChatMessage(JNIEnv* env, jobject thiz, jstring text) {
    if (!env || !text) return;

    // Конвертируем jstring в std::string для проверки в ProcessPlayerCommand
    const char* c_str = env->GetStringUTFChars(text, nullptr);
    if (c_str) {
        std::string playerInput(c_str);
        env->ReleaseStringUTFChars(text, c_str);

        // Передаем строку в нашу систему команд
        if (ProcessPlayerCommand(playerInput)) {
            // Если это наша команда — выходим! Оригинал игры не вызовется, сервер ничего не получит
            return; 
        }
    }

    // Если это обычное сообщение или чужая команда — пропускаем её в игру как обычно
    if (orig_sendChatMessage) {
        orig_sendChatMessage(env, thiz, text);
    }
}

// --- ОСНОВНОЙ ПОТОК ПЛАГИНА ---
void *main_thread(void *) {
    // Ждем полной загрузки библиотеки игры
    do { sleep(1); } while (!isLibraryLoaded(libName));

    // Настраиваем команды
    SetupCommands();

    uintptr_t libBase = getAbsoluteAddress(libName, 0);
    if (libBase) {
        // Ставим хук через Dobby на JNI-метод отправки сообщений (0x00cddf04)
        DobbyHook((void*)(libBase + 0x00cddf04), (void*)hook_sendChatMessage, (void**)&orig_sendChatMessage);
    }

    // Размеры костей по умолчанию
    float defaults[10] = { 0.15f, 0.2f, 0.25f, 0.25f, 0.16f, 0.16f, 0.2f, 0.2f, 0.15f, 0.15f };

    // Цикл модификации памяти (работает независимо от команд)
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(8)); // ~125 тиков под 120Hz экраны

        float currentMultiplier = hitboxesEnabled ? MultiplyValue : 1.0f;
        uintptr_t baseAddress = getAbsoluteAddress(libName, HEAD);

        if (baseAddress) {
            for (int i = 0; i < 10; i++) {
                Utils::WriteMemory<float>(baseAddress + (i * STEP), defaults[i] * currentMultiplier);
            }
        }
    }

    return nullptr;
}

// --- Сохраняем JavaVM при инициализации .so ---
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JavaVM = vm;
    return JNI_VERSION_1_6;
}

// Конструктор плагина
__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
