#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_USERS 100
#define MAX_LOGIN_LENGTH 6
#define MAX_COMMAND_LENGTH 100

typedef struct {
    char login[MAX_LOGIN_LENGTH + 1];
    int pin;
    int is_active;
    int request_limit;
    int request_count;
} User;

User users[MAX_USERS];
int user_count = 0;
User* current_user = NULL;

// Функции для работы с пользователями
int register_user();
int login_user();
void logout_user();
void save_users_to_file();
void load_users_from_file();

// Функции команд
void show_time();
void show_date();
void howmuch_time(const char* datetime, const char* flag);
void sanctions_user(const char* username);

// Вспомогательные функции
int is_valid_login(const char* login);
int is_valid_pin(int pin);
time_t parse_datetime(const char* datetime);

int main() {
    load_users_from_file();
    
    while (1) {
        if (current_user == NULL) {
            printf("\n=== Добро пожаловать в оболочку ===\n");
            printf("1. Регистрация\n");
            printf("2. Авторизация\n");
            printf("3. Выход\n");
            printf("Выберите действие: ");
            
            int choice;
            if (scanf("%d", &choice) != 1) {
                printf("Ошибка ввода!\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n'); // полная очистка буфера
            
            switch (choice) {
                case 1:
                    register_user();
                    break;
                case 2:
                    login_user();
                    break;
                case 3:
                    save_users_to_file();
                    printf("До свидания!\n");
                    return 0;
                default:
                    printf("Неверный выбор!\n");
            }
        } else {
            if (current_user->request_limit > 0 && 
                current_user->request_count >= current_user->request_limit) {
                printf("Превышен лимит запросов! Выполняется выход.\n");
                logout_user();
                continue;
            }
            
            printf("\n[%s] Введите команду: ", current_user->login);
            
            char command[MAX_COMMAND_LENGTH];
            if (fgets(command, sizeof(command), stdin) == NULL) {
                printf("Ошибка ввода!\n");
                continue;
            }
            command[strcspn(command, "\n")] = 0;
            
            if (strlen(command) == 0) continue;
            
            // Улучшенный разбор команд с учетом пробелов в параметрах
            char cmd_name[20];
            char param1[50] = "";
            char param2[10] = "";
            
            int parsed = sscanf(command, "%19s %49[^\n]", cmd_name, param1);
            
            if (parsed == 0) continue;
            
            // Для команды Howmuch нужен особый разбор
            if (strcmp(cmd_name, "Howmuch") == 0) {
                // Разбираем дату/время и флаг отдельно
                char datetime_part[50] = "";
                char flag_part[10] = "";
                
                // Ищем последнее слово как флаг
                char* last_space = strrchr(command, ' ');
                if (last_space != NULL) {
                    strcpy(flag_part, last_space + 1);
                    // Копируем все между "Howmuch" и флагом как datetime
                    char* howmuch_pos = strstr(command, "Howmuch");
                    if (howmuch_pos != NULL) {
                        char* datetime_start = howmuch_pos + 7; // пропускаем "Howmuch"
                        while (*datetime_start == ' ') datetime_start++; // пропускаем пробелы
                        
                        int datetime_len = last_space - datetime_start;
                        if (datetime_len > 0 && datetime_len < sizeof(datetime_part)) {
                            strncpy(datetime_part, datetime_start, datetime_len);
                            datetime_part[datetime_len] = '\0';
                        }
                    }
                }
                
                if (strlen(datetime_part) > 0 && strlen(flag_part) > 0) {
                    howmuch_time(datetime_part, flag_part);
                    current_user->request_count++;
                } else {
                    printf("Неверный формат команды Howmuch!\n");
                    printf("Используйте: Howmuch дд:ММ:гггг чч:мм:сс -флаг\n");
                    printf("Флаги: -s (секунды), -m (минуты), -h (часы), -y (годы)\n");
                }
            }
            else if (strcmp(cmd_name, "Time") == 0) {
                show_time();
                current_user->request_count++;
            }
            else if (strcmp(cmd_name, "Date") == 0) {
                show_date();
                current_user->request_count++;
            }
            else if (strcmp(cmd_name, "Logout") == 0) {
                logout_user();
            }
            else if (strcmp(cmd_name, "Sanctions") == 0) {
                if (strlen(param1) > 0) {
                    sanctions_user(param1);
                    current_user->request_count++;
                } else {
                    printf("Укажите имя пользователя!\n");
                }
            }
            else {
                printf("Неизвестная команда: %s\n", cmd_name);
                printf("Доступные команды: Time, Date, Howmuch, Logout, Sanctions\n");
            }
        }
    }
    
    return 0;
}

// Регистрация и авторизация (без изменений)
int register_user() {
    if (user_count >= MAX_USERS) {
        printf("Достигнуто максимальное количество пользователей!\n");
        return 0;
    }
    
    char login[MAX_LOGIN_LENGTH + 1];
    int pin;
    
    printf("Введите логин (до %d символов, только латинские буквы и цифры): ", MAX_LOGIN_LENGTH);
    if (scanf("%6s", login) != 1) {
        printf("Ошибка ввода логина!\n");
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');
    
    if (!is_valid_login(login)) {
        printf("Неверный формат логина!\n");
        return 0;
    }
    
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].login, login) == 0) {
            printf("Пользователь с таким логином уже существует!\n");
            return 0;
        }
    }
    
    printf("Введите PIN-код (0-100000): ");
    if (scanf("%d", &pin) != 1) {
        printf("Ошибка ввода PIN-кода!\n");
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');
    
    if (!is_valid_pin(pin)) {
        printf("Неверный формат PIN-кода!\n");
        return 0;
    }
    
    strcpy(users[user_count].login, login);
    users[user_count].pin = pin;
    users[user_count].is_active = 1;
    users[user_count].request_limit = 0;
    users[user_count].request_count = 0;
    user_count++;
    
    printf("Пользователь %s успешно зарегистрирован!\n", login);
    save_users_to_file();
    return 1;
}

int login_user() {
    char login[MAX_LOGIN_LENGTH + 1];
    int pin;
    
    printf("Введите логин: ");
    if (scanf("%6s", login) != 1) {
        printf("Ошибка ввода логина!\n");
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');
    
    printf("Введите PIN-код: ");
    if (scanf("%d", &pin) != 1) {
        printf("Ошибка ввода PIN-кода!\n");
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');
    
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].login, login) == 0 && users[i].pin == pin && users[i].is_active) {
            current_user = &users[i];
            current_user->request_count = 0;
            printf("Добро пожаловать, %s!\n", login);
            return 1;
        }
    }
    
    printf("Неверный логин или PIN-код!\n");
    return 0;
}

void logout_user() {
    if (current_user != NULL) {
        printf("До свидания, %s!\n", current_user->login);
        current_user = NULL;
    }
}

void show_time() {
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    printf("Текущее время: %02d:%02d:%02d\n", 
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void show_date() {
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    printf("Текущая дата: %02d:%02d:%04d\n", 
           timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
}

void howmuch_time(const char* datetime, const char* flag) {
    printf("Отладочная информация: datetime='%s', flag='%s'\n", datetime, flag);
    
    time_t target_time = parse_datetime(datetime);
    if (target_time == -1) {
        printf("Неверный формат даты/времени! Используйте: дд:ММ:гггг чч:мм:сс\n");
        return;
    }
    
    time_t now = time(NULL);
    double diff_seconds = difftime(now, target_time);
    
    if (diff_seconds < 0) {
        printf("Указанное время еще не наступило!\n");
        return;
    }
    
    // Правильная проверка флагов
    if (strcmp(flag, "-s") == 0) {
        printf("Прошло секунд: %.0f\n", diff_seconds);
    }
    else if (strcmp(flag, "-m") == 0) {
        printf("Прошло минут: %.2f\n", diff_seconds / 60);
    }
    else if (strcmp(flag, "-h") == 0) {
        printf("Прошло часов: %.2f\n", diff_seconds / 3600);
    }
    else if (strcmp(flag, "-y") == 0) {
        printf("Прошло лет: %.2f\n", diff_seconds / (3600 * 24 * 365.25));
    }
    else {
        printf("Неверный флаг: %s\n", flag);
        printf("Допустимые флаги: -s (секунды), -m (минуты), -h (часы), -y (годы)\n");
    }
}

void sanctions_user(const char* username) {
    if (strcmp(username, current_user->login) == 0) {
        printf("Нельзя установить ограничения самому себе!\n");
        return;
    }
    
    User* target_user = NULL;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].login, username) == 0) {
            target_user = &users[i];
            break;
        }
    }
    
    if (target_user == NULL) {
        printf("Пользователь %s не найден!\n", username);
        return;
    }
    
    printf("Для подтверждения ограничений введите 52: ");
    int confirmation;
    if (scanf("%d", &confirmation) != 1) {
        printf("Ошибка ввода!\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    
    if (confirmation != 52) {
        printf("Подтверждение неверно!\n");
        return;
    }
    
    printf("Введите лимит запросов для пользователя %s: ", username);
    int limit;
    if (scanf("%d", &limit) != 1) {
        printf("Ошибка ввода!\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    
    if (limit < 0) {
        printf("Лимит не может быть отрицательным!\n");
        return;
    }
    
    target_user->request_limit = limit;
    printf("Лимит запросов для пользователя %s установлен в %d\n", username, limit);
    save_users_to_file();
}

int is_valid_login(const char* login) {
    if (strlen(login) == 0 || strlen(login) > MAX_LOGIN_LENGTH) return 0;
    
    for (int i = 0; login[i] != '\0'; i++) {
        if (!isalnum(login[i])) return 0;
    }
    return 1;
}

int is_valid_pin(int pin) {
    return (pin >= 0 && pin <= 100000);
}

time_t parse_datetime(const char* datetime) {
    struct tm timeinfo = {0};
    int day, month, year, hour, minute, second;
    
    if (sscanf(datetime, "%d:%d:%d %d:%d:%d", 
               &day, &month, &year, &hour, &minute, &second) == 6) {
        // Полный формат с датой и временем
    } else if (sscanf(datetime, "%d:%d:%d", &day, &month, &year) == 3) {
        // Только дата, время = 00:00:00
        hour = minute = second = 0;
    } else {
        return -1;
    }
    
    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1900 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        return -1;
    }
    
    timeinfo.tm_mday = day;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    timeinfo.tm_isdst = -1;
    
    return mktime(&timeinfo);
}

void save_users_to_file() {
    FILE* file = fopen("users.dat", "wb");
    if (file == NULL) return;
    
    fwrite(&user_count, sizeof(int), 1, file);
    fwrite(users, sizeof(User), user_count, file);
    fclose(file);
}

void load_users_from_file() {
    FILE* file = fopen("users.dat", "rb");
    if (file == NULL) return;
    
    fread(&user_count, sizeof(int), 1, file);
    fread(users, sizeof(User), user_count, file);
    fclose(file);
}