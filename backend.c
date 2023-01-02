#include "utils.h"
#include "users_lib.h"

int pipeBP[2], pipePB[2];
int threadCounter = 0;

void listUsers(void *user)
{
    FILE *f;

    char *usersFile = getenv("FUSERS");
    if (loadUsersFile(usersFile) == -1)
    {
        printf("\n[!] Error while loading users file\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n[~] Successuflly loaded file ' users '\n");
        printf("\n[~] Successuflly read %d users\n", loadUsersFile(usersFile));
    }
    // Each time this function is called the users loose 1 balance
    int maxUsers = atoi(getenv("MAX_USERS"));

    User *usr = (User *)user;

    f = fopen(usersFile, "r");
    if (f == NULL)
    {
        printf("\n[!] Error while opening file ' users '\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < maxUsers; i++)
    {
        fscanf(f, "%s %s %d", usr->username, usr->password, &usr->balance);
        usr->loggedIn = 0;
        usr->PID = 0;
        usr->next = (User *)malloc(sizeof(User));
        usr = usr->next;
    }

    saveUsersFile(usersFile);
    fclose(f);
}
void loggedIn(void *user)
{
    User *usr = (User *)user;

    while (usr->loggedIn == 1)
    {
        printf("\n%s %s %d", usr->username, usr->password, usr->balance);
        ++usr;
    }
}
void list() {}
void kick(char username[]) {}
void prom() {}
void reprom(void *backend)
{
}
void cancelPromotor() {}
void quitPromotor()
{
    close(pipePB[0]);
    // pthread_exit((void*) NULL);
    // pthread_join(thread, NULL);
    //  kill(pid, SIGUSR2); // Not understood
}
void quit(void *user)
{
    User *usr = (User *)user;
    printf("\n[!] Closing...\n\n");

    while(usr != NULL)
    {
        if(usr->loggedIn == 1)
        {
            kill(usr->PID, SIGINT);
        }
        usr = usr->next;
    }

    unlink(BACKEND_FIFO);
    unlink(BACKEND_FIFO_FRONTEND);
    quitPromotor();
    sleep(1);
    exit(EXIT_SUCCESS);
}
void clear()
{
    system("clear");
}
void backendCommandReader(void *backend, void *user)
{
    Backend *backend_ptr = (Backend *)backend;
    User *usr = (User *)user;
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
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' users '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            loggedIn(user); // Lists users using the platform ATM
        }
        else if (strcmp(cmd, "list") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' list '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            list(); // Lists available items
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
            *arg = 0;  // Clears arg char array
        }
        else if (strcmp(cmd, "prom") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' prom '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            prom(); // Lists active promotors
        }
        else if (strcmp(cmd, "reprom") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' reprom '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            reprom(backend_ptr); // Refresh active promotors
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
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' close '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            quit(user); // Needs to assure closing of possible open pipes and threads
        }
        else if (strcmp(cmd, "clear") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' clear '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            clear();
        }
        else if (strcmp(cmd, "help") == 0)
        {
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
void readItemsFile()
{
    FILE *f;

    char *maxItemsChar = getenv("MAX_ITEMS");
    int maxItems = atoi(maxItemsChar);

    char *itemsFile = getenv("FITEMS");

    Item *it = (Item *)malloc(sizeof(Item) * maxItems);

    f = fopen(itemsFile, "r");
    if (f == NULL)
    {
        printf("\n[!] Error while opening the file ' items '\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n[~] Successuflly loaded file ' items '\n");
    }

    int count = 0;

    while (fscanf(f, "%d %s %s %d %d %d %s %s[^\n]", &it->id, it->name, it->category, &it->basePrice, &it->buyNowPrice, &it->duration, it->sellingUser, it->highestBidder) == 8)
    {
        // printf("\n\n%d %s %s %d %d %d %s %s\n\n", it->id, it->name, it->category, it->basePrice, it->buyNowPrice, it->duration, it->sellingUser, it->highestBidder); // Testing
        ++it;
        ++count;
    }

    printf("\n[~] Successuflly read %d items\n", count);

    fclose(f);
}
void *promotorComms(void *vargp)
{
    char *promotorsFile = getenv("FPROMOTORS");
    char *maxPromotorsChar = getenv("MAX_PROMOTORS");
    int maxPromotors = atoi(maxPromotorsChar);

    char *promotorsExecutablesPath = vargp;

    printf("\n[~] Promotor '%s' launched!\n", promotorsExecutablesPath);

    int estado, num;
    pipe(pipeBP);
    pipe(pipePB);
    int pid = fork();

    if (pid == 0)
    {
        close(0);         // Close stdin
        dup(pipeBP[0]);   // Duplicate pipeBP[0] to read from pipe BackEnd -> Promotor
        close(pipeBP[0]); // Close pipeBP[0]
        close(pipeBP[1]); // Close pipeBP[1]
        close(1);         // Close stdout
        dup(pipePB[1]);   // Duplicate pipePB[1] to write to pipe Promotor -> BackEnd
        close(pipePB[0]); // Close pipePB[0]
        close(pipePB[1]); // Close pipePB[1]
        if (execl(promotorsExecutablesPath, promotorsExecutablesPath, NULL) == -1)
        {
            printf("\n[!] Error while executing ' promotor_oficial '\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        int flag = 1;
        int maxPromotors = atoi(getenv("MAX_PROMOTORS"));
        Promotor *pr = (Promotor *)malloc(sizeof(Promotor) * maxPromotors);
        while (flag) // Review
        {
            close(pipeBP[0]); // Close pipeBP[0]
            close(pipePB[1]); // Close pipePB[1]
            char promotorMSG[50];
            int tam = read(pipePB[0], promotorMSG, sizeof(promotorMSG));
            if (tam > 0)
            {
                promotorMSG[tam - 1] = '\0';
                //printf("\n\n[~] Promotor '%s' message: %s\n", promotorsExecutablesPath, promotorMSG);
                char *token = strtok(promotorMSG, " ");
                int i = 0;
                while (token != NULL)
                {
                    if (i == 0)
                    {
                        strcpy(pr->category, token);
                    }
                    else if (i == 1)
                    {
                        pr->discount = atoi(token);
                    }
                    else if (i == 2)
                    {
                        pr->duration = atoi(token);
                    }
                    token = strtok(NULL, " ");
                    ++i;
                }
                //printf("\n\nTesting struct: %s %d %d\n\n", pr->category, pr->discount, pr->duration); // Testing
            }
            if (tam == -1)
            {
                flag = 0;
            }
        }
    }
    pthread_exit((void *)NULL);
    exit(EXIT_SUCCESS);
}
int instanceController()
{
    int backend_fd = open(BACKEND_FIFO, O_RDONLY | O_NONBLOCK);
    close(backend_fd);
    if (backend_fd == -1)
    {
        return 1;
    }
    return 0;
}
void *frontendComms(void *structThreadCredentials)
{
    StructThreadCredentials *structThreadCredentials_ptr = (StructThreadCredentials *)structThreadCredentials;
    Backend *backend_ptr = structThreadCredentials_ptr->backend;
    User user;
    Comms comms;
    int fd = open(BACKEND_FIFO_FRONTEND, O_RDONLY);
    if (fd == -1)
    {
        printf("\n[!] Error while opening pipe BACKEND_FIFO_FRONTEND\n");
        exit(EXIT_FAILURE);
    }

    //TODO: Check this later. It's only verifying if the user logged out once.

    while (1)
    {
        User *user_ptr = structThreadCredentials_ptr->user;
        memset(&comms, 0, sizeof(comms));
        char mensagem[100];
        //pthread_mutex_lock(backend_ptr->mutex);
        int size = read(fd, &comms, sizeof(comms));
        if (size > 0)
        {
            printf("\n\n[~] Message from %s: %s\nPID: %d\n", comms.username, comms.message, comms.PID);
            if(strcmp(comms.message, "exit") == 0)
            {
                printf("\n\n[~] User %s logged out\n", comms.username);
                while(user_ptr != NULL)
                {
                    if(user_ptr->PID == comms.PID)
                    {
                        user_ptr->loggedIn = 0;
                        break;
                    }
                    user_ptr = user_ptr->next;
                }
                break;
            }
            printf("\n\n[~] Message from %s: %s\n", comms.username, comms.message);
        }
        //pthread_mutex_unlock(backend_ptr->mutex);
    }
    pthread_exit((void *)NULL);
    close(fd);
}
void *verifyCredentials(void *structThreadCredentials)
{
    FILE *f;
    StructThreadCredentials *structThreadCredentials_ptr = (StructThreadCredentials *)structThreadCredentials;
    Backend *backend_ptr = (Backend *)structThreadCredentials_ptr->backend;
    User user;
    int maxUsers = atoi(getenv("MAX_USERS"));

    int fd = open(BACKEND_FIFO, O_RDONLY);
    if (fd == -1)
    {
        printf("\n[!] Error while opening pipe BACKEND_FIFO\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        User *user_ptr = (User *)structThreadCredentials_ptr->user;
        memset(&user, 0, sizeof(user));
        int size = read(fd, &user, sizeof(user));
        //pthread_mutex_lock(backend_ptr->mutex); // Review this later
        if (size > 0)
        {
            if (backend_ptr->connectedClients < maxUsers)
            {
                backend_ptr->connectedClients++;
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, user.PID);
                int fd2 = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd2 == -1)
                {
                    printf("\n[!] Error while opening pipe\n");
                    exit(EXIT_FAILURE);
                }

                int result = 0;

                while(user_ptr != NULL)
                {
                    if (strcmp(user_ptr->username, user.username) == 0)
                    {
                        if (user_ptr->loggedIn == 1)
                        {
                            result = 2;
                            printf("\n\n[~] User %s is already logged in\n", user_ptr->username);
                            break;
                        }
                        // Check if password is correct
                        if (strcmp(user_ptr->password, user.password) == 0)
                        {
                            result = 1;
                            printf("\n\n[~] User %s logged in\n", user_ptr->username);
                            user_ptr->loggedIn = 1;
                            user_ptr->PID = user.PID;
                            break;
                        }
                        else
                        {
                            result = -1;
                            break;
                        }
                    }else{
                        result = 0;
                    }
                    user_ptr = user_ptr->next;
                }

                int size2 = write(fd2, &result, sizeof(result));
                if (size2 == -1)
                {
                    printf("\n[!] Error while writing to pipe\n");
                    exit(EXIT_FAILURE);
                }
                close(fd2);
            }
            else
            {
                printf("\n[!] Client[%5d] connection refused!\n[!] Max number of users reached!\n", user.PID);
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, user.PID);
                int fd2 = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd2 == -1)
                {
                    printf("\n[!] Error while opening pipe\n");
                    exit(EXIT_FAILURE);
                }
                int result = -2;
                int size2 = write(fd2, &result, sizeof(result));
                if (size2 == -1)
                {
                    printf("\n[!] Error while writing to pipe\n");
                    exit(EXIT_FAILURE);
                }
                close(fd2);
            }
        }
        //pthread_mutex_unlock(backend_ptr->mutex);
    }
    close(fd);
    pthread_exit((void *)NULL);
}
void sendSignal(int s, siginfo_t *info, void *v)
{
    int n = info->si_value.sival_int;
    kill(info->si_pid, SIGUSR1);
}
void crtlCSignal()
{
    printf("\nCtrl+C activated");
    // SEND KILL SIGNAL TO FRONTEND... HOW
    quit(NULL);
}
void readPromoters(void *prt){
    FILE *f;
    char *promotersFile = getenv("FPROMOTERS");
    int maxPromoters = atoi(getenv("MAX_PROMOTORS"));

    f = fopen(promotersFile, "r");
    if (f == NULL)
    {
        printf("\n[!] Error while opening file %s\n", promotersFile);
        exit(EXIT_FAILURE);
    }

    Promotor *promotor_ptr = (Promotor *)prt;

    for(int i=0; i < maxPromoters; i++){
        if(fscanf(f, "%s", promotor_ptr->path) == 1){
            threadCounter++;
        }
        strcpy(promotor_ptr->category, "");
        promotor_ptr->discount = 0;
        promotor_ptr->duration = 0;
        promotor_ptr->next = (Promotor *)malloc(sizeof(Promotor));
        promotor_ptr = promotor_ptr->next;
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    clear();
    printf("\n\nbackend @ SOBAY\n\n");
    if (!instanceController())
    {
        printf("\n[!] Another instance of the backend is already running\n");
        return 0;
    } // Checks if there is already an instance of the backend running

    // In order for this to work don't forget to compile varAmbiente.sh first! By doing: "source varAmbiente.sh in terminal"
    if (getenv("MAX_USERS") == NULL)
    {

        printf("\n[!] Error! MAX_USERS not defined!\n");
        return (0);
    }
    if (getenv("MAX_ITEMS") == NULL)
    {
        printf("\n[!] Error! MAX_ITEMS not defined!\n");
        return (0);
    }
    if (getenv("FUSERS") == NULL)
    {
        printf("\n[!] Error! FUSERS not defined!\n");
        return (0);
    }
    if (getenv("FITEMS") == NULL)
    {
        printf("\n[!] Error! FITEMS not defined!\n");
        return (0);
    }
    if (getenv("MAX_PROMOTORS") == NULL)
    {
        printf("\n[!] Error! MAX_PROMOTORS not defined!\n");
        return (0);
    }

    // FIFO creation
    if (mkfifo(BACKEND_FIFO, 0666) == -1)
    {
        printf("\n[!] Error while creating the backend FIFO\n");
        return 0;
    }

    if (mkfifo(BACKEND_FIFO_FRONTEND, 0666) == -1)
    {
        printf("\n[!] Error while creating the backendFrontend FIFO\n");
        return 0;
    }

    readItemsFile();

    StructThreadCredentials structThreadCredentials;
    Backend backend;
    User usr;
    Promotor prt;
    listUsers(&usr);
    readPromoters(&prt);
    backend.connectedClients = 0;
    backend.maxItems = atoi(getenv("MAX_ITEMS"));
    backend.maxUsers = atoi(getenv("MAX_USERS"));
    backend.maxPromoters = atoi(getenv("MAX_PROMOTORS"));
    structThreadCredentials.backend = &backend;

    pthread_t threadPromotor[threadCounter];
    pthread_t threadCredentials;
    pthread_t threadFrontendComms;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    backend.mutex = &mutex;

    char promotersPath[threadCounter][100];

    for(int i=0; i < backend.maxPromoters; i++){
        if(strcmp(prt.path, "") != 0){
            char path[100] = "txt/";
            strcat(path, prt.path);
            strcpy(promotersPath[i], path);
        }
        prt = *prt.next;
    }
    
    while(threadCounter != 0){
        if(pthread_create(&threadPromotor[threadCounter], NULL, promotorComms, promotersPath[threadCounter-1]) != 0)
            perror("Error joining thread");
        threadCounter--;
    }

    structThreadCredentials.user = &usr;

    if (pthread_create(&threadCredentials, NULL, verifyCredentials, &structThreadCredentials) != 0)
        perror("Error creating thread");

    if (pthread_create(&threadFrontendComms, NULL, frontendComms, &structThreadCredentials) != 0)
        perror("Error creating thread");


    struct sigaction sa;
     sa.sa_sigaction = crtlCSignal;
     sa.sa_flags = SA_SIGINFO;
     sigaction(SIGINT, &sa, NULL);
    
    sleep(3);
    backendCommandReader(&backend, &usr); // Needs to integrate a thread

    while (threadCounter != 0)
    {
        pthread_join(threadPromotor[threadCounter], NULL);
        --threadCounter;
    }

    pthread_join(threadCredentials, NULL);
    pthread_join(threadFrontendComms, NULL);
    pthread_mutex_destroy(&mutex);

    pthread_exit((void *)NULL);
    unlink(BACKEND_FIFO);
    unlink(BACKEND_FIFO_FRONTEND);
    return 0;
}