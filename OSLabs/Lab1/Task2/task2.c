#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LOGIN_LEN 6
#define MAX_USERS 100
#define MAX_CMD_LEN 100
#define DATETIME_FORMAT "%d:%d:%d %d:%d:%d"
#define USERS_FILE "users.txt"

typedef struct {
    char login[MAX_LOGIN_LEN + 1];
    int pin;
    int request_limit; // -1 если нет ограничений
} User;

User users[MAX_USERS];
int user_count = 0;
int current_session_requests = 0;

// Функции для работы с пользователями
void load_users();
void save_users();
int find_user(const char *login);
bool is_valid_login(const char *login);
bool is_valid_pin(int pin);
void register_user();
User* login_user();

// Функции команд
void cmd_time();
void cmd_date();
void cmd_howmuch(const char *datetime, const char *flag);
void cmd_logout();
void cmd_sanctions(User *current_user, const char *username);
long calculate_time_diff(struct tm *target_time, const char *flag);

// Вспомогательные функции
bool is_prime(unsigned char n);
void to_lower_case(char *str);

// Главные функции
void shell_loop(User *user);
void print_welcome();

// ==================== РЕАЛИЗАЦИЯ ФУНКЦИЙ ====================

// Загрузка пользователей из файла
void load_users() {
    FILE *f = fopen(USERS_FILE, "r");
    if (!f) {
        // Файл не существует, это нормально при первом запуске
        return;
    }
    
    user_count = 0;
    while (fscanf(f, "%6s %d %d", 
                  users[user_count].login, 
                  &users[user_count].pin, 
                  &users[user_count].request_limit) == 3) {
        user_count++;
        if (user_count >= MAX_USERS) break;
    }
    fclose(f);
}

// Сохранение пользователей в файл
void save_users() {
    FILE *f = fopen(USERS_FILE, "w");
    if (!f) {
        printf("Error: Cannot save users file!\n");
        return;
    }
    
    for (int i = 0; i < user_count; i++) {
        fprintf(f, "%s %d %d\n", users[i].login, users[i].pin, users[i].request_limit);
    }
    fclose(f);
}

// Поиск пользователя по логину
int find_user(const char *login) {
    char lower_login[MAX_LOGIN_LEN + 1];
    strcpy(lower_login, login);
    to_lower_case(lower_login);
    
    for (int i = 0; i < user_count; i++) {
        char current_login[MAX_LOGIN_LEN + 1];
        strcpy(current_login, users[i].login);
        to_lower_case(current_login);
        
        if (strcmp(current_login, lower_login) == 0) {
            return i;
        }
    }
    return -1;
}

// Проверка валидности логина
bool is_valid_login(const char *login) {
    if (strlen(login) == 0 || strlen(login) > MAX_LOGIN_LEN) {
        return false;
    }
    
    for (int i = 0; login[i] != '\0'; i++) {
        if (!isalnum(login[i])) {
            return false;
        }
    }
    return true;
}

// Проверка валидности PIN
bool is_valid_pin(int pin) {
    return pin >= 0 && pin <= 100000;
}

// Регистрация нового пользователя
void register_user() {
    if (user_count >= MAX_USERS) {
        printf("Error: Maximum number of users reached (%d).\n", MAX_USERS);
        return;
    }
    
    User new_user;
    
    printf("=== REGISTRATION ===\n");
    printf("Enter login (max %d letters/digits): ", MAX_LOGIN_LEN);
    scanf("%6s", new_user.login);
    
    // Проверка логина
    if (!is_valid_login(new_user.login)) {
        printf("Error: Invalid login. Use only letters and digits, max %d characters.\n", MAX_LOGIN_LEN);
        return;
    }
    
    if (find_user(new_user.login) != -1) {
        printf("Error: User with login '%s' already exists.\n", new_user.login);
        return;
    }
    
    printf("Enter PIN (0-100000): ");
    if (scanf("%d", &new_user.pin) != 1) {
        printf("Error: Invalid PIN format.\n");
        while (getchar() != '\n'); // Очистка буфера
        return;
    }
    
    if (!is_valid_pin(new_user.pin)) {
        printf("Error: PIN must be between 0 and 100000.\n");
        return;
    }
    
    new_user.request_limit = -1; // Нет ограничений по умолчанию
    
    users[user_count] = new_user;
    user_count++;
    save_users();
    
    printf("Registration successful! User '%s' created.\n", new_user.login);
}

// Авторизация пользователя
User* login_user() {
    char login[MAX_LOGIN_LEN + 1];
    int pin;
    
    printf("=== LOGIN ===\n");
    printf("Login: ");
    scanf("%6s", login);
    printf("PIN: ");
    
    if (scanf("%d", &pin) != 1) {
        printf("Error: Invalid PIN format.\n");
        while (getchar() != '\n');
        return NULL;
    }
    
    int user_index = find_user(login);
    if (user_index == -1) {
        printf("Error: User '%s' not found.\n", login);
        return NULL;
    }
    
    if (users[user_index].pin != pin) {
        printf("Error: Invalid PIN for user '%s'.\n", login);
        return NULL;
    }
    
    printf("Login successful! Welcome, %s!\n", login);
    current_session_requests = 0; // Сброс счетчика запросов
    return &users[user_index];
}

// Команда Time - вывод текущего времени
void cmd_time() {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    printf("Current time: %02d:%02d:%02d\n", 
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

// Команда Date - вывод текущей даты
void cmd_date() {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    printf("Current date: %02d:%02d:%04d\n", 
           timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
}

// Проверка простого числа
bool is_prime(unsigned char n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

// Приведение строки к нижнему регистру
void to_lower_case(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
}

// Расчет разницы во времени для команды Howmuch
long calculate_time_diff(struct tm *target_time, const char *flag) {
    time_t target_t = mktime(target_time);
    time_t now_t = time(NULL);
    
    double diff_seconds = difftime(now_t, target_t);
    
    if (strcmp(flag, "-s") == 0) {
        return (long)diff_seconds;
    } else if (strcmp(flag, "-m") == 0) {
        return (long)(diff_seconds / 60);
    } else if (strcmp(flag, "-h") == 0) {
        return (long)(diff_seconds / 3600);
    } else if (strcmp(flag, "-y") == 0) {
        return (long)(diff_seconds / (3600 * 24 * 365.25));
    }
    
    return 0;
}

// Команда Howmuch - расчет прошедшего времени
void cmd_howmuch(const char *datetime, const char *flag) {
    int day, month, year, hour, minute, second;
    
    // Парсинг даты и времени - исправленный формат
    if (sscanf(datetime, "%d:%d:%d", &day, &month, &year) != 3) {
        printf("Error: Invalid date format. Use 'dd:MM:yyyy hh:mm:ss'\n");
        return;
    }
    
    // Извлечение времени из второго параметра (если есть)
    char time_part[20];
    if (strlen(datetime) > 11) { // после даты есть пробел и время
        sscanf(datetime + 11, "%d:%d:%d", &hour, &minute, &second);
    } else {
        // Время по умолчанию, если не указано
        hour = 0;
        minute = 0;
        second = 0;
    }
    
    // Проверка валидности даты/времени
    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1900 || year > 2100 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        printf("Error: Invalid datetime values.\n");
        printf("Date: %02d:%02d:%04d, Time: %02d:%02d:%02d\n", day, month, year, hour, minute, second);
        return;
    }
    
    // Проверка валидности флага
    if (strcmp(flag, "-s") != 0 && strcmp(flag, "-m") != 0 && 
        strcmp(flag, "-h") != 0 && strcmp(flag, "-y") != 0) {
        printf("Error: Invalid flag. Use -s (seconds), -m (minutes), -h (hours), -y (years)\n");
        return;
    }
    
    struct tm target_time = {0};
    target_time.tm_mday = day;
    target_time.tm_mon = month - 1;
    target_time.tm_year = year - 1900;
    target_time.tm_hour = hour;
    target_time.tm_min = minute;
    target_time.tm_sec = second;
    target_time.tm_isdst = -1; // Автоматическое определение летнего времени
    
    long result = calculate_time_diff(&target_time, flag);
    
    const char *unit = "";
    if (strcmp(flag, "-s") == 0) unit = "seconds";
    else if (strcmp(flag, "-m") == 0) unit = "minutes";
    else if (strcmp(flag, "-h") == 0) unit = "hours";
    else if (strcmp(flag, "-y") == 0) unit = "years";
    
    printf("Time passed since %02d:%02d:%04d %02d:%02d:%02d: %ld %s\n", 
           day, month, year, hour, minute, second, result, unit);
}

// Команда Logout
void cmd_logout() {
    printf("Logging out...\n");
}

// Команда Sanctions - установка ограничений
void cmd_sanctions(User *current_user, const char *username) {
    if (strcmp(current_user->login, username) == 0) {
        printf("Error: You cannot set sanctions for yourself.\n");
        return;
    }
    
    int user_index = find_user(username);
    if (user_index == -1) {
        printf("Error: User '%s' not found.\n", username);
        return;
    }
    
    printf("Warning: You are about to set request limits for user '%s'\n", username);
    printf("Enter 52 to confirm: ");
    
    int confirmation;
    if (scanf("%d", &confirmation) != 1) {
        printf("Error: Invalid input.\n");
        while (getchar() != '\n');
        return;
    }
    
    if (confirmation != 52) {
        printf("Sanctions cancelled.\n");
        return;
    }
    
    printf("Enter maximum requests per session for user '%s': ", username);
    int max_requests;
    if (scanf("%d", &max_requests) != 1 || max_requests < 1) {
        printf("Error: Invalid number of requests.\n");
        while (getchar() != '\n');
        return;
    }
    
    users[user_index].request_limit = max_requests;
    save_users();
    printf("Sanctions set: user '%s' is now limited to %d requests per session.\n", 
           username, max_requests);
}

// Главный цикл оболочки
void shell_loop(User *user) {
    char input[MAX_CMD_LEN];
    
    printf("\n=== SHELL STARTED ===\n");
    printf("Available commands: Time, Date, Howmuch, Logout, Sanctions\n");
    
    while (1) {
        printf("\n%s@shell> ", user->login);
        
        // Чтение команды
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Удаление символа новой строки
        input[strcspn(input, "\n")] = 0;
        
        // Пропуск пустых строк
        if (strlen(input) == 0) {
            continue;
        }
        
        // Проверка ограничений запросов
        if (user->request_limit != -1) {
            if (current_session_requests >= user->request_limit) {
                printf("Error: Request limit exceeded (%d/%d). Logging out.\n", 
                       current_session_requests, user->request_limit);
                break;
            }
        }
        
        current_session_requests++;
        
        // Разбор команды
        char command[20], param1[50], param2[20];
        int args = sscanf(input, "%19s %49s %19s", command, param1, param2);
        
        to_lower_case(command);
        
        if (strcmp(command, "time") == 0) {
            cmd_time();
        } else if (strcmp(command, "date") == 0) {
            cmd_date();
        // В функции shell_loop замените разбор команды Howmuch:
        } else if (strcmp(command, "howmuch") == 0) {
            if (args >= 3) {
            // Объединяем param1 и param2 для даты и времени
            char datetime[60];
            snprintf(datetime, sizeof(datetime), "%s %s", param1, param2);
            
            // Третий параметр - флаг
            char flag[10];
            if (args >= 4) {
                // Есть четвертый параметр для флага
                sscanf(input, "%*s %*s %*s %9s", flag);
                cmd_howmuch(datetime, flag);
            } else {
                printf("Usage: Howmuch <dd:MM:yyyy> <hh:mm:ss> <-s|-m|-h|-y>\n");
                printf("Example: Howmuch 25:12:2023 10:30:00 -h\n");
            }
        } else {
            printf("Usage: Howmuch <dd:MM:yyyy> <hh:mm:ss> <-s|-m|-h|-y>\n");
            printf("Example: Howmuch 25:12:2023 10:30:00 -h\n");
        }
} else if (strcmp(command, "logout") == 0) {
            cmd_logout();
            break;
        } else if (strcmp(command, "sanctions") == 0) {
            if (args >= 2) {
                cmd_sanctions(user, param1);
            } else {
                printf("Usage: Sanctions <username>\n");
            }
        } else if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("  Time                    - Show current time\n");
            printf("  Date                    - Show current date\n");
            printf("  Howmuch <datetime> <flag> - Calculate time passed\n");
            printf("  Logout                  - Log out from shell\n");
            printf("  Sanctions <username>    - Set request limits for user\n");
        } else {
            printf("Error: Unknown command '%s'. Type 'help' for available commands.\n", command);
        }
    }
}

// Приветственное сообщение
void print_welcome() {
    printf("=========================================\n");
    printf("        SIMPLE SHELL v1.0\n");
    printf("=========================================\n");
}

// Главная функция
int main() {
    load_users();
    print_welcome();
    
    while (1) {
        printf("\n=== MAIN MENU ===\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("3. Exit\n");
        printf("Choose option (1-3): ");
        
        char choice[10];
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            break;
        }
        
        int option = atoi(choice);
        
        switch (option) {
            case 1: {
                User *logged_user = login_user();
                if (logged_user != NULL) {
                    shell_loop(logged_user);
                }
                break;
            }
            case 2:
                register_user();
                break;
            case 3:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Error: Invalid option. Please choose 1-3.\n");
                break;
        }
    }
    
    return 0;
}