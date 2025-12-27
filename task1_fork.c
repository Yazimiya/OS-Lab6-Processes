#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

// Функция для вывода информации о процессе
void print_process_info(const char* process_name) {
    struct timeval tv;          // Структура для времени с микросекундами
    struct tm *tm_info;         // Структура для разбора времени
    
    gettimeofday(&tv, NULL);    // Получаем текущее время
    tm_info = localtime(&tv.tv_sec);  // Преобразуем секунды в структуру tm
    
    // Выводим информацию: имя процесса, PID, PPID, время
    printf("%s: PID = %d, PPID = %d, Время = %02d:%02d:%02d:%03ld\n",
           process_name, getpid(), getppid(),
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
           tv.tv_usec / 1000);  // Преобразуем микросекунды в миллисекунды
}

int main() {
    printf("Лабораторная работа 6. Задание 1 \n");
    printf("Текущий каталог: ");
    fflush(stdout); // Принудительно выводим буфер
    system("pwd");  // Выводим текущий каталог
    printf("\n");
    
    // ПЕРВЫЙ вызов fork() - создаем первый дочерний процесс
    pid_t pid1 = fork();
    
    if (pid1 < 0) {
        // Ошибка при создании процесса
        perror("Ошибка fork()");
        exit(1);
    }
    
    if (pid1 == 0) {
        // Этот код выполняется только в ПЕРВОМ ДОЧЕРНЕМ процессе
        print_process_info("Дочерний процесс 1");
        sleep(1);  // Задержка 1 секунда для наглядности 
        exit(0);  // Завершаем дочерний процесс
    }
    
    // Этот код выполняется только в РОДИТЕЛЬСКОМ процессе
    // ВТОРОЙ вызов fork() - создаем второй дочерний процесс
    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        perror("Ошибка fork()");
        exit(1);
    }
    
    if (pid2 == 0) {
        // Этот код выполняется только во ВТОРОМ ДОЧЕРНЕМ процессе
        print_process_info("Дочерний процесс 2");
        sleep(1);  // Задержка 1 секунда для наглядности
        exit(0);
    }
    
    // Этот код выполняется только в ОРИГИНАЛЬНОМ РОДИТЕЛЬСКОМ процессе
    print_process_info("Родительский процесс");
    
    // Ждем завершения обоих дочерних процессов
    wait(NULL);
    wait(NULL);
    
    printf("\n--- Выполняем команду ps -x ---\n");
    printf("Ищем наши процессы в списке:\n");
    // Выполняем команду ps -x и фильтруем вывод
    system("ps -x | grep -E '(PID|task1_fork)'");
    
    return 0;
}

