#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int wish_cd(char **tokens,int num_tokens);
int wish_exit(char **tokens,int num_tokens);
int wish_pwd(char **tokens,int num_tokens);
int wish_path(char **tokens,int num_tokens);
char *search_exe(char *command);


char **path;
int MAX_PATH_LENGTH = 1024;
int is_batch_mode = 0;
int MAX_INPUT_LENGTH = 1024;
int max_tokens = 20;

void printError() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

char *remove_newline(const char *str) {
    size_t length = strcspn(str, "\n");
    char *result = malloc((length + 1) * sizeof(char));
    strncpy(result, str, length);
    result[length] = '\0';
    return result;
}

char *builtin_str[] = {
    "cd",
    "exit",
    "pwd",
    "path"};

int (*builtin_func[])(char **,int) = {
    &wish_cd,
    &wish_exit,
    &wish_pwd,
    &wish_path};

int builtin_num()
{
    return sizeof(builtin_str) / sizeof(char *); // 字符指针数组的大小除以每个指针（地址值）得到指针个数，即字符串个数
}

int wish_cd(char **tokens,int num_tokens)
{
    if(num_tokens>2 || num_tokens==1)
    {
        printError();
        return 1;
    }

    
    
    if (chdir(tokens[1]) != 0)
    {
            printError();
            return 1;
            //perror("wish"); // 前缀加系统错误
    }
    
    return 0;
}

int wish_exit(char **tokens,int num_tokens)
{
    if(num_tokens==1)
    {exit(0);}
    else{
        printError();
    }
    
}

int wish_pwd(char **tokens,int num_tokens)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current working directory: %s\n", cwd);
    }
    else
    {
        printError();
        //perror("wish");
    }
    return 1;
}

int wish_path(char **tokens,int num_tokens)
{
    int new_path_count = 0;
    while (tokens[new_path_count + 1] != NULL)
    {
        new_path_count++;
    }

    int old_path_count = 0;
    while (path[old_path_count] != NULL)
    {
        old_path_count++;
    }

    int total_path_count = old_path_count + new_path_count;

    if (new_path_count == 0) {//用户输入空格时，path清除
        for (int i = 0; i < old_path_count; i++) {
            free(path[i]);
        }
        free(path);
        path = NULL;
        path = malloc(sizeof(char *));
        path[0] = NULL;

        return 1;
    }

    path = realloc(path, (total_path_count + 1) * sizeof(char *));
    
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    for (int i = 0; i < new_path_count; i++)
{
    if(tokens[i+1][0]!='/')
    {
    char *combined_path = malloc((strlen(cwd) + strlen(tokens[i + 1]) + 1) * sizeof(char));
    sprintf(combined_path, "%s/%s", cwd, tokens[i + 1]);

    path[old_path_count + i] = malloc((strlen(combined_path) + 1) * sizeof(char));
    strcpy(path[old_path_count + i], combined_path);

    free(combined_path);
    }
    else{
        path[old_path_count + i] = malloc((strlen(tokens[i + 1]) + 1) * sizeof(char));
        strcpy(path[old_path_count + i], tokens[i + 1]);
    }
}

    

    path[total_path_count] = NULL;

    return 1;
}

char *search_exe(char *command)
{
    int i = 0;
    while (path[i] != NULL)
    {
        char *exe = malloc(MAX_PATH_LENGTH * sizeof(char));
        snprintf(exe, MAX_PATH_LENGTH, "%s/%s", path[i], command);

        if (access(exe, X_OK) == 0)
        {
            return exe;
        }
        

        free(exe);
        i++;
    }

    

    return NULL;
}

int launch_process(char **tokens)
{
    char *command = tokens[0];
    char *exe = search_exe(command);
    if (exe == NULL)
    {
        printError();
        //printf("Command not found: %s\n", command);
        return 1;
    }

    pid_t pid, wpid;
    int status;

    pid = fork();

    if (pid < 0)
    {
        printError();
        //perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        char *output_file = NULL;
        int output_fd = -1;

        // 是否需要重定向输出
        int i = 0;
        while (tokens[i] != NULL)
        {
            if (strcmp(tokens[i], ">") == 0)
            {
                if (tokens[i + 1] != NULL && tokens[i + 2] == NULL)
                {
                    output_file = tokens[i + 1];
                    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                    if (output_fd == -1)
                    {
                        printError();
                        //perror("open");
                        exit(1);
                    }

                    tokens[i] = NULL;
                    tokens[i + 1] = NULL;
                    break;
                }
                else
                {
                    printError();
                    //fprintf(stderr, "Invalid usage of output redirection\n");
                    exit(1);
                }
            }
            i++;
        }

        // 若有>则重定向
        if (output_fd != -1)
        {
            if (dup2(output_fd, STDOUT_FILENO) == -1) // 将文件描述符复制到标准输出流和标准错误流，实现重定向
            {
                printError();
                //perror("dup2");
                exit(1);
            }
            if (dup2(output_fd, STDERR_FILENO) == -1)
            {
                printError();
                //perror("dup2");
                exit(1);
            }
        }

        // printf("1\n");
        // printf("2\n");
        execv(exe, tokens);
        // printf("3\n");

       
       
        //perror("ls");
        //perror("execv");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        free(exe);
        return 1;
    }
}
    

int launch_parallel_commands(char **cmds[], int cmds_index)
{
    pid_t pids[cmds_index];
    int status;

    for (int i = 0; i < cmds_index; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printError();
            //perror("fork");
            return 1;
        }
        else if (pid == 0)
        {
            char *exe = search_exe(cmds[i][0]);
            if (exe == NULL)
            {
                printError();
                //printf("Command not found: %s\n", cmds[i][0]);
                exit(1);
            }

            char *output_file = NULL;
            int output_fd = -1;

            // 是否需要重定向输出
            int j = 0;
            while (cmds[i][j] != NULL)
            {
                if (strcmp(cmds[i][j], ">") == 0)
                {
                    if (cmds[i][j + 1] != NULL && cmds[i][j + 2] == NULL)
                    {
                        output_file = cmds[i][j + 1];
                        output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                        if (output_fd == -1)
                        {
                            printError();
                            //perror("open");
                            exit(1);
                        }

                        cmds[i][j] = NULL;
                        cmds[i][j + 1] = NULL;
                        break;
                    }
                    else
                    {
                        printError();
                        //fprintf(stderr, "Invalid usage of output redirection\n");
                        exit(1);
                    }
                }
                j++;
            }

            // 若有>则重定向
            if (output_fd != -1)
            {
                if (dup2(output_fd, STDOUT_FILENO) == -1)
                {
                    printError();
                    //perror("dup2");
                    exit(1);
                }
                if (dup2(output_fd, STDERR_FILENO) == -1)
                {
                    printError();
                    //perror("dup2");
                    exit(1);
                }
            }

            execvp(exe, cmds[i]);

            // 若 execvp 执行失败，才会执行到这里
            printError();
            //perror("execvp");
            exit(1);
        }
        else
        {
            pids[i] = pid;
        }
    }

    for (int i = 0; i < cmds_index; i++)
    {
        waitpid(pids[i], &status, 0);
    }

    return 1;
}

int action(char **tokens, int i)
{

    if (tokens[0] == NULL)
    {
        return 1;
    }

    int cmds_index = 0;
    int num_cmds = 0;
    char ***cmds = malloc(10 * sizeof(char **));         // 最多10个并行命令
    cmds[cmds_index] = malloc((i + 1) * sizeof(char *)); // 为\0留位置

    for (int j = 0; j < i; j++)
    {
        if (strcmp(tokens[j], "&") == 0)
        {
            if(j==0){return 0;}
            if(j==i-1){break;}
            cmds[cmds_index][num_cmds] = NULL;
            cmds_index++;
            cmds[cmds_index] = malloc((i + 1) * sizeof(char *));
            num_cmds = 0;
        }
        else
        {
            cmds[cmds_index][num_cmds] = malloc(strlen(tokens[j]) * sizeof(char));
            strcpy(cmds[cmds_index][num_cmds], tokens[j]);
            num_cmds++;
        }
    }
    cmds[cmds_index][num_cmds] = NULL;

    if (cmds_index >= 1)
    {
        return launch_parallel_commands(cmds, cmds_index + 1);
    }
    else
    {
        for (int k = 0; k < builtin_num(); k++)
        {
            if (strcmp(tokens[0], builtin_str[k]) == 0)
            {
                return (*builtin_func[k])(tokens,i);
            }
        }

        return launch_process(tokens);
        return 1;
    }
}
    


char **tokenize_input(char *input,int *num_tokens)
{
    char **tokens = NULL;
    tokens = malloc(max_tokens * sizeof(char *));
    if (tokens != NULL) {
    memset(tokens, 0, max_tokens * sizeof(char *));
    // 继续使用 tokens
    }
    int i = 0;
    int token_start = 0;
    int token_end = -1;
    int in_quotes = 0;
    int in_token = 0;


    for(int j = 0;input[j]!='\0';j++ )
    {
        if(input[j]==' ')
        {
            if(token_end>=token_start && input[token_start]!=' ')
            {tokens[i]=malloc((token_end-token_start+2)*sizeof(char));
            strncpy(tokens[i],input+token_start,token_end-token_start+1);
            tokens[i][token_end-token_start+1] ='\0';
            i++;
            token_start=j+1;
            token_end=j+1;}
            else
            {
                token_start=j+1;
                token_end=j+1;
            }
        }
        else if(input[j]=='>')
        {
            if(token_end>token_start)
            {
            tokens[i]=malloc((token_end-token_start+2)*sizeof(char));
            strncpy(tokens[i],input+token_start,token_end-token_start+1);
            tokens[i][token_end-token_start+1] = '\0';
            i++;           
            tokens[i]=malloc(2*sizeof(char));
            tokens[i][0]='>';
            tokens[i][1]='\0';
            i++;
            token_start=j+1;
            token_end=j+1;
            }
            else
            {
            tokens[i]=malloc(2*sizeof(char));
            tokens[i][0]='>';
            tokens[i][1]='\0';
            i++;
            token_start=j+1;
            token_end=j+1;
            }
        }
        else if(input[j]=='&')
        {
            if(token_end>token_start)
            {
            tokens[i]=malloc((token_end-token_start+2)*sizeof(char));
            strncpy(tokens[i],input+token_start,token_end-token_start+1);
            tokens[i][token_end-token_start+1] = '\0';
            i++;           
            tokens[i]=malloc(2*sizeof(char));
            tokens[i][0]='&';
            tokens[i][1]='\0';
            i++;
            token_start=j+1;
            token_end=j+1;
            }
            else
            {
            tokens[i]=malloc(2*sizeof(char));
            tokens[i][0]='&';
            tokens[i][1]='\0';
            i++;
            token_start=j+1;
            token_end=j+1;
            }
        }
        else 
        {
            token_end=j;
        }
    }
    if(input[token_start]!='\0')
    {
    tokens[i]=malloc((token_end-token_start+2)*sizeof(char));
    strncpy(tokens[i],input+token_start,token_end-token_start+1);
    tokens[i][token_end-token_start+1]='\0';
    for(int k=i+1;k<max_tokens;k++)
    {
        if(tokens[k]!=NULL)
        {
            free(tokens[k]);
            tokens[k]=NULL;
        }

    }
    *num_tokens = i+1;
    return tokens;
    }

    for(int k=i;k<max_tokens;k++)
    {
        if(tokens[k]!=NULL)
        {
            free(tokens[k]);
            tokens[k]=NULL;
        }

    }

    *num_tokens = i;
    return tokens;
}


int main(int argc, char **argv)
{
    if (argc > 1)
    {
        is_batch_mode = 1;
    }
    FILE *batch_file = NULL;
    if (is_batch_mode)
    {
            batch_file = fopen(argv[1], "r");
            if (batch_file == NULL)
            {
                printError();
                //fprintf(stderr, "Failed to open batch file.\n");
                return 1;
            }

        fseek(batch_file, 0, SEEK_END);
        if (ftell(batch_file) == 0)
        {
            fclose(batch_file);
            printError();
            return 1;
        }
        fseek(batch_file, 0, SEEK_SET);
    }
        path = malloc(sizeof(char *));
        path[0] = NULL;
        path = malloc(3*sizeof(char *));
        path[0] = malloc((strlen("/bin")+1)*sizeof(char));
        strcpy(path[0],"/bin");
        path[1] = malloc((strlen("/usr/bin") + 1)*sizeof(char));
        strcpy(path[1],"/usr/bin");
        path[2] = NULL;

        char *line = NULL;
        size_t len = 0;
        char **tokens = NULL;
        int i = 0;
        int max_tokens = 100;
        char *token = NULL;
        char *line_copy = NULL;
        char *line_without_newline = NULL;
        
        do
        {
            //free(line);
            line = NULL;
            line_copy = NULL;
            line_without_newline = NULL;
            line = malloc(MAX_INPUT_LENGTH * sizeof(char));
            //free(token);
            //token = NULL;

            if(is_batch_mode==0)
            {printf("wish> ");
            getline(&line, &len, stdin);}
            if(is_batch_mode)
            {
                if(fgets(line, MAX_INPUT_LENGTH, batch_file)==NULL)
                {
                    break;
                }
            }
            
            line_copy = strdup(line); // 复制输入行到另一个变量
            
            line_without_newline = remove_newline(line_copy);
            
            i = 0;

            
            tokens=tokenize_input(line_without_newline,&i);

            action(tokens, i);
            

            for (int j = 0; j < i; j++)
            {
                free(tokens[j]);
            }
            free(tokens);

            
            free(line_without_newline);
            free(line_copy);
            

            // printf("%s", line);

            if (strcmp(line, "exit\n") == 0)
            {
                break;
            }
            free(line);
        } while (1);

        if(is_batch_mode)
        {
            fclose(batch_file);
        }

        free(line);
        exit(0);
    }