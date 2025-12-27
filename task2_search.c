#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#define MAX_NAME 512  // максимальная длина пути к файлу

int main() {
    char dirname[MAX_NAME];  // имя каталога для поиска
    char pattern[MAX_NAME];  // искомая комбинация байт
    int m, N;  // m - длина комбинации, N - ограничение процессов
    
    // ввод имени каталога
    printf("Введите каталог для поиска: ");
    scanf("%255s", dirname);
    while (getchar() != '\n');  // очищаем буфер ввода
    
    // ввод комбинации для поиска
    printf("Введите комбинацию для поиска: ");
    fgets(pattern, MAX_NAME, stdin);
    pattern[strcspn(pattern, "\n")] = 0;  // удаляем символ новой строки
    
    // вычисляем длину комбинации
    m = strlen(pattern);
    if (m <= 0 || m >= 255) {
        printf("Ошибка: длина должна быть 0 < m < 255\n");
        return 1;
    }
    
    printf("Длина комбинации: %d байт\n", m);
    
    // ввод ограничения по процессам
    printf("Максимальное число процессов (N): ");
    scanf("%d", &N);
    while (getchar() != '\n');  // очищаем буфер
    
    // вывод параметров поиска
    printf("\nПараметры поиска:\n");
    printf("Каталог: %s\n", dirname);
    printf("Комбинация: '%s' (%d байт)\n", pattern, m);
    printf("Максимум процессов: %d\n\n", N);
    
    // открываем каталог для чтения
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror("Ошибка открытия каталога");
        return 1;
    }
    
    struct dirent *entry;  // структура для записи каталога
    int active = 0;  // счетчик активных процессов
    int files = 0;   // счетчик обработанных файлов
    
    printf("Начинаем поиск...\n");
    
    // обход всех файлов в каталоге
    while ((entry = readdir(dir)) != NULL) {
        // пропускаем записи . и ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        // формируем полный путь к файлу
        char fullpath[MAX_NAME];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, entry->d_name);
        
        // проверяем, что это обычный файл
        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
            files++;  // увеличиваем счетчик файлов
            
            // если достигли максимума процессов - ждем
            while (active >= N) {
                wait(NULL);  // ждем завершения любого процесса
                active--;    // уменьшаем счетчик активных
            }
            
            // создаем новый процесс для поиска в файле
            pid_t pid = fork();
            
            if (pid == 0) {
                // код выполняется в дочернем процессе
                FILE *file = fopen(fullpath, "rb");  // открываем файл в бинарном режиме
                if (!file) {
                    printf("Процесс %d: не могу открыть %s\n", getpid(), entry->d_name);
                    exit(1);
                }
                
                unsigned char buffer[1024];  // буфер для чтения файла
                size_t bytes;                // количество прочитанных байт
                long total = 0;              // всего прочитано байт
                int matches = 0;             // количество найденных совпадений
                
                // читаем файл блоками по 1024 байта
                while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    // ищем комбинацию в текущем блоке
                    for (size_t i = 0; i <= bytes - m; i++) {
                        int found = 1;  // флаг совпадения
                        // проверяем последовательность байт
                        for (int j = 0; j < m; j++) {
                            if (buffer[i + j] != (unsigned char)pattern[j]) {
                                found = 0;  // не совпадает
                                break;
                            }
                        }
                        if (found) matches++;  // нашли совпадение
                    }
                    total += bytes;  // обновляем счетчик прочитанных байт
                }
                
                fclose(file);  // закрываем файл
                
                // выводим результат как требует задание
                printf("PID: %d, Файл: %s, Байт: %ld, Найдено: %d\n",
                       getpid(), entry->d_name, total, matches);
                
                exit(0);  // завершаем дочерний процесс
            }
            else if (pid > 0) {
                // код выполняется в родительском процессе
                active++;  // увеличиваем счетчик активных процессов
                printf("Запущен процесс %d для: %s\n", pid, entry->d_name);
            }
            else {
                perror("fork ошибка");  // ошибка при создании процесса
            }
        }
    }
    
    closedir(dir);  // закрываем каталог
    
    // ждем завершения всех оставшихся процессов
    while (active > 0) {
        wait(NULL);
        active--;
    }
    
    printf("\nПоиск завершен\n");
    printf("Обработано файлов: %d\n", files);
    
    return 0;
}
