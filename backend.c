#include "backend.h"

void users(){}
void list(){}
void kick(char username[]){}
void prom(){}
void reprom(){}
void cancelPromotor(){}
void quit(){
    printf("\n[!] Closing...\n\n");
    sleep(1);
    exit(EXIT_SUCCESS);
}
void clear(){
    system("clear");
}

void backendCommandReader()
{
    clear();
    int pid = getpid();
    char command[50];
    char cmd[15], arg[30];

    printf("\nbackendCommandReader @ type help for command list\n\n");

    while (1)
    {
        *arg = 0;
        *cmd = 0;
        fflush(stdout);
        printf("[%5d] COMMAND: ", pid);
        scanf(" %49[^\n]", command);
        sscanf(command, "%14s %29[^\n]", cmd, arg); // Usar isto novamente para repartir o arg nos vários argumentos!!

        if (strcmp(cmd, "users") == 0)
        {
            users(); //Lists users using the platform ATM
        }
        else if (strcmp(cmd, "list") == 0)
        {
            list(); //Lists available items
        }
        else if (strcmp(cmd, "kick") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n[!] Invalid notation for command ' kick '\n");
                printf("[~] Use the following notation: 'kick <username>'\n");
                continue;
            }
            kick(arg); // Kicks certain user by the username stored in "arg"
            *arg = 0; // Clears arg char array
        }
        else if (strcmp(cmd, "prom") == 0)
        {
            prom(); //Lists active promotors
        }
        else if (strcmp(cmd, "reprom") == 0)
        {
            reprom(); //Refresh active promotors
            // Reads the promotors file again, removes no longer present promotors and adds new ones
        }
        else if (strcmp(cmd, "cancel") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n[!] Invalid notation for command ' cancel '\n");
                printf("[~] Use the following notation: 'cancel <promotors-executable-filename>'\n");
                continue;
            }
            cancelPromotor(); // Cancels promotor
        }
        else if (strcmp(cmd, "close") == 0)
        {
            quit(); // Needs to assure closing of possible open pipes and threads
        }
        else if (strcmp(cmd, "clear") == 0)
        {
            clear();
        }
        else if (strcmp(cmd, "help") == 0){
            printf("\t\n[!] Available commands:\n");
            printf("[~] users || Lists users using the platform\n");
            printf("[~] list || Lists available items\n");
            printf("[~] kick <username> || Kicks a certain user\n");
            printf("[~] prom || Lists active promotors\n");
            printf("[~] reprom || Refresh active promotors\n");
            printf("[~] cancel <promotors-executable-filename> || Cancels promotor\n");
            printf("[~] close || Closes platform\n");
            printf("[~] clear || Console clear\n\n");
        }
        else
        {
            printf("\t[!] Command not found: '%s' (For help type: 'help')\n\n", cmd);
        }
    }
}

int main(int argc, char **argv)
{
    char backend_fifo[20];
    int backend_fd;
    pthread_t id;
    int pid = getpid();

    if (access(BACKEND_FIFO, F_OK) != 0)
    {
        if (mkfifo(BACKEND_FIFO, S_IRUSR | S_IWUSR) == -1)
        {
            perror("\n[!] Error creating pipe!\nError ");
            exit(EXIT_FAILURE);
        }
    } // Not working as supposed, instance controller?

    // In order for this to work don't forget to compile varAmbiente.sh first! By doing: "source varAmbiente.sh in terminal"
    if (getenv("MAX_USERS") == NULL)
    {
        printf("\n[!] Error! MAX_USERS not defined!\n");
        return (0);
    }
    if (getenv("MAX_PROMOTORS") == NULL)
    {
        printf("\n[!] Error! MAX_PROMOTORS not defined!\n");
        return (0);
    }

    char *maxUsersChar = getenv("MAX_USERS");
    int maxUsers = atoi(maxUsersChar);

    char *maxPromotorsChar = getenv("MAX_PROMOTORS");
    int maxPromotors = atoi(maxPromotorsChar);

    backendCommandReader();
}