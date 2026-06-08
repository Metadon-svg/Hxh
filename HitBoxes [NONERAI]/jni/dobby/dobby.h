#ifndef DOBBY_H
#define DOBBY_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Основная функция для создания инлайн-хука (перехвата).
 * * @param address      Адрес оригинальной функции в памяти (куда ставим хук).
 * @param replace_call Адрес твоей функции-перехватчика (которая будет вызываться вместо оригинальной).
 * @param origin_call  Указатель, куда Dobby запишет адрес для вызова оригинальной функции.
 * @return             Возвращает 0 при успешном хуке, или отрицательное число при ошибке.
 */
int DobbyHook(void *address, void *replace_call, void **origin_call);

/**
 * Функция для поиска функций (символов) по их имени в загруженных .so библиотеках.
 * Аналог стандартного dlsym, но умеет искать даже скрытые символы.
 * * @param image_name   Имя библиотеки (например, "libblackrussia-client.so"). Если NULL — ищет везде.
 * @param symbol_name  Имя функции (например, "Java_com_blackhub_bronline_game_core_JNILib_sendChatMessage").
 */
void *DobbySymbolResolver(const char *image_name, const char *symbol_name);

/**
 * Возвращает текущую версию сборки Dobby.
 */
const char *DobbyBuildVersion(void);

#ifdef __cplusplus
}
#endif

#endif // DOBBY_H
