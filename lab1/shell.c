#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "shell.h"
#include "util.h"

extern char **__environ;

void execute_external_command(char *tokens[], int token_count, char *input_file, char *output_file, bool append_output)
{
    bool background = false; // Флаг для определения того, будет ли задача выполняться в фоне

    // Ищем "&" в токенах, если есть - задача фоновая
    if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0)
    {
        background = true;
        tokens[token_count - 1] = NULL;
    }
    // Создаём дочерний процесс
    pid_t child_pid = fork();

    if (child_pid == 0)
    { // Дочерний процесс
        if (input_file)
        { // Определен входной файл
            int input_fd = open(input_file, O_RDONLY);
            int oldinput_fd = dup(0);

            if (input_fd < 0)
            {
                fprintf(stderr, "Cannot open input file!\n");
                exit(EXIT_FAILURE);
            }

            // Переопределяем стандартный ввод
            if (dup2(input_fd, STDIN_FILENO) == -1)
            {
                fprintf(stderr, "Cannot redirect input!\n");
            }

            // fflush(stdin);
            // dup2(oldinput_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (background)
        { // Фоновая задача
            int dev_null_fd = open("/dev/null", O_RDWR);

            if (dev_null_fd < 0)
            {
                fprintf(stderr, "Cannot access /dev/null!\n");
                exit(EXIT_FAILURE);
            }

            if (!input_file)
            {
                dup2(dev_null_fd, STDIN_FILENO);
            }

            if (!output_file)
            {
                dup2(dev_null_fd, STDOUT_FILENO);
            }
            if (strcmp(tokens[0], "top") == 0) {
                tokens[token_count - 1] = "-n";
                tokens[token_count] = "1";
                tokens[token_count + 1] = "-b";
                tokens[token_count + 2] = NULL;
            }
            execvp(tokens[0], tokens);
            dup2(dev_null_fd, STDERR_FILENO);

            close(dev_null_fd);
        }
        else
        {
            execvp(tokens[0], tokens);
            fprintf(stderr, "Cannot exec program!\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (child_pid > 0)
    { // Родительский процесс
        if (!background)
        {                                // Если задача не фоновая, то
            waitpid(child_pid, NULL, 0); // Ждем ее завершения
        }
    }
    else
    {
        fprintf(stderr, "Cannot fork process!\n");
    }
}

void execute_help_command()
{
    FILE *less_pipe;

    // Создаем канал к процессу less
    less_pipe = popen("less", "w");
    if (less_pipe == NULL)
    {
        perror("Can't create pipe to less.");
        return;
    }

    // Открываем файл readme и сохраняем стандартный ввод
    int readme_file_fd = open("readme", O_RDONLY);
    int oldinput_fd = dup(0);

    if (readme_file_fd < 0)
    {
        fprintf(stderr, "Cannot open input file!\n");
        if (pclose(less_pipe) == -1)
        {
            perror("Can't close pipe to stream.");
            return;
        }
        return;
    }

    // Переопределяем стандартный ввод
    if (dup2(readme_file_fd, STDIN_FILENO) == -1)
    {
        fprintf(stderr, "Cannot redirect input!\n");
        if (pclose(less_pipe) == -1)
        {
            perror("Can't close pipe to stream.");
            return;
        }
        return;
    }

    char buffer[MAX_INPUT_SIZE];
    size_t bytes_read_count;

    while ((bytes_read_count = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0)
    {
        if (fwrite(buffer, 1, bytes_read_count, less_pipe) != bytes_read_count)
        {
            perror("Can't write data to steam.");
            break;
        }
    }

    fflush(stdin);
    dup2(oldinput_fd, STDIN_FILENO);
    close(readme_file_fd);

    if (pclose(less_pipe) == -1)
    {
        perror("Can't close pipe to stream.");
        return;
    }
}

bool execute_internal_command(char *tokens[], int token_count, char *output_file, bool append_output)
{
    if (token_count == 0)
    {
        return true;
    }

    if (strcmp(tokens[0], "cd") == 0)
    {
        if (token_count == 1)
        { // Отсутствуют аргументы команды
            chdir(".");
        }
        else
        {
            if (chdir(tokens[1]) != 0)
            { // Недействительный путь
                fprintf(stderr, "Path is invalid.\n");
            }
            else
            { // Изменяем рабочую папку
                setenv("PWD", getcwd(NULL, 0), 1);
            }
        }

        return true;
    }
    else if (strcmp(tokens[0], "clr") == 0)
    {
        printf("\x1B[2J"); // Очистка экрана
        printf("\x1B[H");  // Перенос курсора в начало

        return true;
    }
    else if (strcmp(tokens[0], "dir") == 0)
    {                                                                                                                                   // Вывод содержания каталога
        struct dirent *entry;                                                                                                           // Структура, описывающая директорию
        DIR *dir = NULL;
        if ((token_count == 1 || (token_count > 1 && !!output_file && tokens[1] == NULL)))
		{
			dir = opendir(".");
		}
		else 
        {
			dir = opendir(tokens[1]);
		} // Изменяет папку при данных аргументах

        if (dir == NULL)
        { // Недействительный путь
            fprintf(stderr, "Can\'t open directory!\n");
            return true;
        }

        char *filenames[MAX_DIRECTORY_CONTENTS]; // Имена файлов и папок
        int files_cnt = 0;                       // Количество файлов и папок

        // Читаем содержимое папки
        while ((entry = readdir(dir)) != NULL)
        {
            filenames[files_cnt++] = strdup(entry->d_name);
        }

        closedir(dir);

        for (int i = 0; i < files_cnt; i++)
        {
            printf("%s\n", filenames[i]);
            free(filenames[i]);
        }

        return true;
    }
    else if (strcmp(tokens[0], "environ") == 0)
    { // Вывод переменных среды
        char **env = __environ;
        while (*env)
        {
            printf("%s\n", *env);
            env++;
        }

        return true;
    }
    else if (strcmp(tokens[0], "echo") == 0)
    { // Вывод текста
        for (int i = 1; i < token_count; i++)
        {
            if (tokens[i] == NULL)
            {
                break;
            }
			if (i == token_count - 1) {
				printf("%s%s", tokens[i], "\n");
			}
			else {
				printf("%s%s", tokens[i], " ");
			}
            
        }

        return true;
    }
    else if (strcmp(tokens[0], "pause") == 0)
    { // Пауза
        printf("Press Enter to continue...");
        getchar();

        return true;
    }
    else if (strcmp(tokens[0], "quit") == 0)
    { // Выход
        exit(EXIT_SUCCESS);
    }
    if (strcmp(tokens[0], "help") == 0)
    {
        execute_help_command();
        return true;
    }

    return false;
}

char *input_file = NULL;    // Имя файла, на который переопределен ввод
char *output_file = NULL;   // Имя файла, на который переопределен вывод
bool append_output = false; // Флаг для определения того, нужно ли добавлять к файлу данные

void update_input_output(char *tokens[], int token_count)
{
    // Ищем операторы переопределения ввода/вывода
    // Если такие есть, то записываем имя файла и заменяем оператор на NULL
    input_file = NULL;
    output_file = NULL;
    for (int i = 0; i < token_count - 1; i++)
    {
        if (!input_file && strcmp(tokens[i], "<") == 0)
        {
            if (i < token_count - 1)
            {
                input_file = tokens[i + 1];
                tokens[i] = NULL;
            }
            else
            {
                fprintf(stderr, "Input file is missing!\n");
                return;
            }
        }
        else if (!output_file && strcmp(tokens[i], ">") == 0)
        {
            if (i < token_count - 1)
            {
                output_file = tokens[i + 1];
                tokens[i] = NULL;
            }
            else
            {
                fprintf(stderr, "Output file is missing!\n");
                return;
            }
        }
        else if (!output_file && strcmp(tokens[i], ">>") == 0)
        {
            if (i < token_count - 1)
            {
                output_file = tokens[i + 1];
                tokens[i] = NULL;
                append_output = true;
            }
            else
            {
                fprintf(stderr, "Output file is missing!\n");
                return;
            }
        }
    }
}

void proccess_input(char input[], char *tokens[], int token_count)
{                                          // Обрабатываем запрос от пользователя
    tokenize(input, tokens, &token_count); // Делим запрос на отдельные аргументы/токены

    update_input_output(tokens, token_count); // Проверяем запрос на наличие input/output file

    if (token_count > 0)
    {
        int output_fd = -1;
        int oldoutput_fd = dup(STDOUT_FILENO); // Дубликация консольного вывода
        if (output_file)
        {

            if (append_output)
            { // нашли оператор ">>" открываем на добавление текста
                output_fd = open(output_file, O_CREAT | O_RDWR | O_APPEND, 0644);
            }
            else
            {
                output_fd = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            }

            if (output_fd < 0)
            {
                fprintf(stderr, "Cannot open output file!\n");
                exit(EXIT_FAILURE);
            }

            if (dup2(output_fd, STDOUT_FILENO) == -1)
            {
                fprintf(stderr, "Cannot redirect output!\n");
            }
        }

        if (!execute_internal_command(tokens, token_count, output_file, append_output)) // Прошлись по внутренним, если не нашли идём во внешние
        {
            execute_external_command(tokens, token_count, input_file, output_file, append_output);
        }

        if (!!output_file) // Если был output file возвращаем output обратно терминалу
        {
            fflush(stdout);                    // Выводим всё что осталось в буффере
            dup2(oldoutput_fd, STDOUT_FILENO); // Возвращаем терминальный output обратно
            close(output_fd);                  // Закрываем файл
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        FILE *cmd_source = fopen(argv[1], "r");
        if (cmd_source == NULL)
        {
            fprintf(stderr, "Cannot open commands file!\n");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_INPUT_SIZE];
        char *buf_tokens[MAX_TOKENS];
        int buf_token_count = 0;

        while (fgets(buffer, sizeof(buffer), cmd_source) != NULL)
        {
            proccess_input(buffer, buf_tokens, buf_token_count);
        }

        fclose(cmd_source);

        return EXIT_SUCCESS;
    }

    // Получаем текущую рабочую папку
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL)
    {
        fprintf(stderr, "Cannot get current directory!\n");
        exit(EXIT_FAILURE);
    }

    // Выделяем память для строки с путем до файла оболочки
    char *shell_path = (char *)malloc(strlen(cwd) + strlen(SHELL_NAME) + 2);
    if (shell_path == NULL)
    {
        fprintf(stderr, "malloc failed!\n");
        free(cwd);
        exit(EXIT_FAILURE);
    }

    // Формируем путь к файлу оболочки и выставляем переменную окружения
    sprintf(shell_path, "%s/%s", cwd, SHELL_NAME);
    setenv("SHELL", shell_path, 1);

    free(cwd);
    free(shell_path);

    char input[MAX_INPUT_SIZE];
    char *tokens[MAX_TOKENS];
    int token_count = 0;

    while (true)
    {
        char *current_dir = getcwd(NULL, 0);
        printf("%s$ ", current_dir);
        free(current_dir);

        // Если не можем получить входную строку или достигнут конец файла
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            break;
        }

        proccess_input(input, tokens, token_count);
    }

    return EXIT_SUCCESS;
}
