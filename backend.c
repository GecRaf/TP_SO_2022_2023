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
        updateUserBalance(usr->username, usr->balance - 1);
        usr = usr->next;
    }
    saveUsersFile(usersFile);
    fclose(f);
}
void loggedIn(void *user)
{
    User *usr = (User *)user;

    printf("\n\t[~] List of users using the platform ATM:\n");

    while (usr != NULL)
    {
        if (usr->loggedIn == 1)
            printf("\n\t\t[~] %s\n", usr->username);
        usr = usr->next;
    }
    printf("\n");
}
void list(void *item)
{
    Item *it = (Item *)item;
    printf("\n\t[~] List of items:\n");

    while (it != NULL)
    {
        if (strcmp(it->name, "") != 0)
            printf("\n\t\t[~] ID: %d\n\t\t[~] Name: %s\n\t\t[~] Category: %s\n\t\t[~] Base Price: %d\n\t\t[~] Buy Now Price: %d\n\t\t[~] Selling User: %s\n\t\t[~] Highest Bidder: %s\n", it->id, it->name, it->category, it->basePrice, it->buyNowPrice, it->sellingUser, it->highestBidder);
        it = it->next;
    }
    printf("\n");
}
void kick(char username[], void *user){
    User *usr = (User *)user;
    while (usr != NULL)
    {
        if (strcmp(usr->username, username) == 0)
        {
            if (usr->loggedIn == 1)
            {
                kill(usr->PID, SIGINT);
                printf("\n\t[~] User '%s' kicked\n\n", username);
                // Put the user loggedIn to 0
                usr->loggedIn = 0;
                return;
            }
            else
            {
                printf("\n\t[!] User '%s' is not logged in\n\n", username);
                return;
            }
        }
        usr = usr->next;
    }
    printf("\n\t[!] User '%s' does not exist\n\n", username);
}
void *promotorComms(void *vargp)
{
    if (strcmp(vargp, "") == 0)
    {
        pthread_exit((void *)NULL);
    }

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
                // printf("\n\n[~] Promotor '%s' message: %s\n", promotorsExecutablesPath, promotorMSG);
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
                // printf("\n\nTesting struct: %s %d %d\n\n", pr->category, pr->discount, pr->duration); // Testing
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
void readPromoters(void *prt)
{
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

    for (int i = 0; i < maxPromoters; i++)
    {
        if (fscanf(f, "%s", promotor_ptr->path) == 1)
        {
            threadCounter++;
        }
        strcpy(promotor_ptr->category, "");
        promotor_ptr->discount = 0;
        promotor_ptr->duration = 0;
        promotor_ptr->next = (Promotor *)malloc(sizeof(Promotor));
        promotor_ptr = promotor_ptr->next;
    }

    fclose(f);

    promotor_ptr = (Promotor *)prt;

    for (int j = 0; j < maxPromoters; j++)
    {
        if (strcmp(promotor_ptr->path, "") != 0)
        {
            char path[100] = "txt/";
            strcat(path, promotor_ptr->path);
            strcpy(promotor_ptr->path, path);
            promotor_ptr->active = 0;
        }
        promotor_ptr = promotor_ptr->next;
    }
}
void launchPromotersThreads(void *structThreadCredentials, pthread_t *threadPromotor)
{
    StructThreadCredentials *mainStruct = (StructThreadCredentials *)structThreadCredentials;
    Promotor *prt = (Promotor *)mainStruct->promotor;

    while (threadCounter != 0)
    {
        if (strcmp(prt->path, "") != 0 && prt->active == 0)
        {
            if (pthread_create(&threadPromotor[threadCounter], NULL, promotorComms, &prt->path) != 0)
                perror("Error creating thread");
            printf("\n[+] Promotor thread created '%s'\n", prt->path);
            prt->active = 1;
            prt->threadID = threadCounter;
        }
        else if (strcmp(prt->path, "") != 0 && prt->active == 1)
        {
            // Kill thread
            if (pthread_kill(threadPromotor[prt->threadID], SIGUSR1) != 0)
                perror("Error killing thread");
            printf("\n[+] Promotor thread killed '%s'\n", prt->path);
        }
        prt = prt->next;
        threadCounter--;
    }
}
void prom(void *promotor)
{
    // Show all the active promoters
    Promotor *prt = (Promotor *)promotor;
    printf("\n\t[~] List of active promoters:\n");

    while (prt != NULL)
    {
        if (prt->active == 1)
            printf("\n\t\t[~] %s\n", prt->path);
        prt = prt->next;
    }
    printf("\n");
}
void reprom(void *structThreadCredentials, pthread_t *threadPromotor)
{
    StructThreadCredentials *mainStruct = (StructThreadCredentials *)structThreadCredentials;
    Promotor *prt = (Promotor *)mainStruct->promotor;
    int maxPromotors = atoi(getenv("MAX_PROMOTORS"));
    char *promotersFile = getenv("FPROMOTERS");

    FILE *f;
    f = fopen(promotersFile, "r");
    if (f == NULL)
    {
        printf("\n[!] Error while opening file %s\n", promotersFile);
        exit(EXIT_FAILURE);
    }

    // Print file line by line
    char line[100];
    printf("\n\t[~] List of promoters:\n");
    while (fgets(line, sizeof(line), f))
    {
        prt = (Promotor *)mainStruct->promotor;
        char prefix[100] = "txt/";
        strcat(prefix, line);
        strcpy(line, prefix);
        printf("\n\t\t[~] %s", line);

        while(prt != NULL)
        {
            if (strcmp(prt->path, line) == 0)
            {
                printf("\t\t\t[~] Já existe\n");
                break;
            }
            else if(strcmp(prt->path, "") == 0)
            {
                strcpy(prt->path, line);
                prt->active = 1;
                if (pthread_create(&threadPromotor[maxPromotors], NULL, promotorComms, &prt->path) != 0)
                    perror("Error creating thread");
                printf("\n[+] Promotor thread created '%s'\n", prt->path);
                prt->threadID = maxPromotors;
                break;
            }
            prt = prt->next;
        }
    }
    printf("\n");
}
void cancelPromotor(char path[])
{
}
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

    while (usr != NULL)
    {
        if (usr->loggedIn == 1)
        {
            kill(usr->PID, SIGUSR1);
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
void backendCommandReader(void *structThreadCredentials, pthread_t *threadPromotor)
{
    StructThreadCredentials *mainStruct = (StructThreadCredentials *)structThreadCredentials;
    Backend *backend_ptr = (Backend *)mainStruct->backend;
    User *usr = (User *)mainStruct->user;
    Item *it = (Item *)mainStruct->item;
    Promotor *prt = (Promotor *)mainStruct->promotor;
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
            loggedIn(usr); // Lists users using the platform ATM
        }
        else if (strcmp(cmd, "list") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' list '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            list(it); // Lists available items
        }
        else if (strcmp(cmd, "kick") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n[!] Invalid notation for command ' kick '\n");
                printf("[~] Use the following notation: 'kick <username>'\n");
                continue;
            }
            kick(arg, usr); // Kicks certain user by the username stored in "arg"
            *arg = 0;       // Clears arg char array
        }
        else if (strcmp(cmd, "prom") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' prom '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            prom(prt); // Lists active promotors
        }
        else if (strcmp(cmd, "reprom") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' reprom '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            reprom(mainStruct, threadPromotor); // Refresh active promotors
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
            cancelPromotor(arg); // Cancels promotor
        }
        else if (strcmp(cmd, "close") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n[!] Invalid notation for command ' close '\n");
                printf("[!] No arguments needed\n");
                continue;
            }
            quit(usr); // Needs to assure closing of possible open pipes and threads
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
void readItemsFile(void *item)
{
    Item *it = (Item *)item;
    FILE *f;

    char *maxItemsChar = getenv("MAX_ITEMS");
    int maxItems = atoi(maxItemsChar);

    char *itemsFile = getenv("FITEMS");

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

    for (int i = 0; i < maxItems; i++)
    {
        // 1 sapatilhas desporto 50 30 100
        if (fscanf(f, "%d %s %s %d %d %d", &it->id, it->name, it->category, &it->basePrice, &it->buyNowPrice, &it->duration) == 6)
            count++;
        strcpy(it->highestBidder, "N/A");
        strcpy(it->sellingUser, "N/A");
        it->next = (Item *)malloc(sizeof(Item));
        it = it->next;
    }

    printf("\n[~] Successuflly read %d items\n", count);

    fclose(f);
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
    Item it;

    int fd = open(BACKEND_FIFO_FRONTEND, O_RDONLY);
    if (fd == -1)
    {
        printf("\n[!] Error while opening pipe BACKEND_FIFO_FRONTEND\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        User *user_ptr = structThreadCredentials_ptr->user;
        Item *item_ptr = structThreadCredentials_ptr->item;
        memset(&comms, 0, sizeof(comms));

        int size = read(fd, &comms, sizeof(comms));
        if (size > 0)
        {
            if (strcmp(comms.message, "exit") == 0)
            {

                while (user_ptr != NULL)
                {
                    if (user_ptr->PID == comms.PID)
                    {
                        printf("\n\n[~] User %s logged out\n", user_ptr->username);
                        pthread_mutex_lock(backend_ptr->mutex);
                        user_ptr->loggedIn = 0;
                        pthread_mutex_unlock(backend_ptr->mutex);
                        break;
                    }
                    user_ptr = user_ptr->next;
                }
            }
            else if (strcmp(comms.message, "cash") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'", comms.message, comms.username);
                while (user_ptr != NULL)
                {
                    if (user_ptr->PID == comms.PID)
                    {
                        comms.balance = user_ptr->balance;
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n[!] Error while opening pipe FRONTEND_FIFO\n");
                            exit(EXIT_FAILURE);
                        }
                        int size2 = write(fd, &comms, sizeof(comms));
                        if (size2 == -1)
                        {
                            printf("\n[!] Error while writing to pipe FRONTEND_FIFO\n");
                            exit(EXIT_FAILURE);
                        }
                        close(fd);
                        break;
                    }
                    user_ptr = user_ptr->next;
                }
            }
            else if (strcmp(comms.message, "add") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                while (user_ptr != NULL)
                {
                    if (user_ptr->PID == comms.PID)
                    {
                        pthread_mutex_lock(backend_ptr->mutex);
                        user_ptr->balance += comms.balance;
                        comms.balance = user_ptr->balance;
                        pthread_mutex_unlock(backend_ptr->mutex);
                        break;
                    }
                    user_ptr = user_ptr->next;
                }
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd == -1)
                {
                    printf("\n[!] Error while opening pipe FRONTEND_FIFO\n");
                    exit(EXIT_FAILURE);
                }
                // Write comms to frontend
                int size2 = write(fd, &comms, sizeof(comms));
                if (size2 == -1)
                {
                    printf("\n[!] Error while writing to pipe FRONTEND_FIFO\n");
                    exit(EXIT_FAILURE);
                }
                close(fd);
            }
            else if (strcmp(comms.message, "sell") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                int fd = open(BACKEND_FIFO_FRONTEND, O_RDONLY);
                if (fd == -1)
                {
                    printf("\n[!] Error while opening pipe BACKEND_FIFO_FRONTEND\n");
                    exit(EXIT_FAILURE);
                }
                int size = read(fd, &it, sizeof(it));
                if (size == -1)
                {
                    printf("\n[!] Error while reading from pipe BACKEND_FIFO_FRONTEND\n");
                    exit(EXIT_FAILURE);
                }
                close(fd);
                pthread_mutex_lock(backend_ptr->mutex);
                int id = 0;
                while (item_ptr != NULL)
                {
                    if (item_ptr->id == 0)
                    {
                        item_ptr->id = id + 1;
                        it.id = item_ptr->id;
                        strcpy(item_ptr->name, it.name);
                        strcpy(item_ptr->category, it.category);
                        item_ptr->basePrice = it.basePrice;
                        item_ptr->buyNowPrice = it.buyNowPrice;
                        item_ptr->duration = it.duration;
                        strcpy(item_ptr->sellingUser, it.sellingUser);
                        strcpy(item_ptr->highestBidder, it.highestBidder);
                        item_ptr->bought = it.bought;

                        // Send int with the id of the item to the frontend
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd2 = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd2 == -1)
                        {
                            printf("\n[!] Error while opening pipe FRONTEND_FIFO\n");
                            exit(EXIT_FAILURE);
                        }
                        printf("\t[~] Item '%s' added to the database with ID %d\n", it.name, it.id);
                        int size2 = write(fd2, &it, sizeof(it));
                        if (size2 == -1)
                        {
                            printf("\n[!] Error while writing to pipe FRONTEND_FIFO\n");
                            exit(EXIT_FAILURE);
                        }
                        close(fd2);
                        pthread_mutex_unlock(backend_ptr->mutex);

                        break;
                    }
                    else
                    {
                        id = item_ptr->id;
                    }
                    item_ptr = item_ptr->next;
                }
            }
            else if (strcmp(comms.message, "list") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                //pthread_mutex_lock(backend_ptr->mutex);
                // While item_ptr->id != 0, send the item to the frontend. Last item will have id = -1
                while (item_ptr != NULL)
                {
                    printf("\t[~] Item '%s' with ID %d sent to the frontend\n", item_ptr->name, item_ptr->id);
                    if (item_ptr->id != 0)
                    {
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n[!] Error while opening pipe FRONTEND_FIFO\n");
                            exit(EXIT_FAILURE);
                        }
                        int size2 = write(fd, item_ptr, sizeof(Item));
                        if (size2 == -1)
                        {
                            printf("\n[!] Error while writing to pipe FRONTEND_FIFO\n");
                            exit(EXIT_FAILURE);
                        }
                        close(fd);
                    }
                    item_ptr = item_ptr->next;
                }
                // Something is wrong here. Sometimes the last item is not sent to the frontend
                Item end;
                end.id = -1;
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd == -1)
                {
                    printf("\n[!] Error while opening pipe FRONTEND_FIFO\n");
                    exit(EXIT_FAILURE);
                }
                int size2 = write(fd, &end, sizeof(Item));
                if (size2 == -1)
                {
                    printf("\n[!] Error while writing to pipe FRONTEND_FIFO\n");
                    exit(EXIT_FAILURE);
                }
                close(fd);
                //pthread_mutex_unlock(backend_ptr->mutex);
            }
        }
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
        // pthread_mutex_lock(backend_ptr->mutex); // Review this later
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

                while (user_ptr != NULL)
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
                    }
                    else
                    {
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
        // pthread_mutex_unlock(backend_ptr->mutex);
    }
    close(fd);
    pthread_exit((void *)NULL);
}
void *verifyUserAlive(void *structThreadCredentials)
{
    StructThreadCredentials *structThreadCredentials_ptr = (StructThreadCredentials *)structThreadCredentials;
    User user;
    Comms comms;
    printf("\n[~] Function VerifyUserAlive\n");
    int heartbeat = atoi(getenv("HEARTBEAT"));
    printf("\n[~] Heartbeat: %d\n", heartbeat);
    
    // Send signal SIGURS2 to frontend and wait for signal SIGURS1
    // If signal SIGURS1 is not received in 5 seconds, the user is considered dead
    // If the user is dead, the user is removed from the list of connected users

    while (1)
    {
        sleep(heartbeat);
        User *user_ptr = (User *)structThreadCredentials_ptr->user;
        while (user_ptr != NULL)
        {
            if (user_ptr->loggedIn == 1)
            {
                printf("\n[~] Sending signal to frontend\n");
                kill(user_ptr->PID, SIGUSR2);
                // If the user is alive we will receive a struct comms through the pipe BACKEND_FIFO
                // If the user is dead we will not receive a struct comms through the pipe BACKEND_FIFO
                int fd = open(BACKEND_FIFO, O_RDONLY);
                if (fd == -1)
                {
                    printf("\n[!] Error while opening pipe BACKEND_FIFO\n");
                    exit(EXIT_FAILURE);
                }
                int size = read(fd, &comms, sizeof(comms));
                if(size > 0)
                {
                    printf("\n[~] User %s is alive\n", comms.username);
                }
                else
                {
                    printf("\n[~] User %s is dead\n", user_ptr->username);
                    user_ptr->loggedIn = 0;
                    user_ptr->PID = 0;
                    structThreadCredentials_ptr->backend->connectedClients--;
                }
            }
            user_ptr = user_ptr->next;
        }
    }
    
    pthread_exit((void *)NULL);
}
void sendSignal(int s, siginfo_t *info, void *v)
{
    int n = info->si_value.sival_int;
    kill(info->si_pid, SIGUSR1);
}
void crtlCSignal()
{
    // TODO: How to send signal to every frontend?
    quit(NULL);
}

int main(int argc, char **argv)
{
    clear();
    printf("\n\nbackend @ SOBAY\n\n");

    // Check if another instance of the backend is already running

    if (!instanceController())
    {
        printf("\n[!] Another instance of the backend is already running\n");
        return 0;
    }

    // Environment variables check

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

    StructThreadCredentials structThreadCredentials;
    Backend backend;
    User usr;
    Promotor prt;
    Item itm;

    // Struct initialization

    listUsers(&usr);
    readPromoters(&prt);
    readItemsFile(&itm);

    backend.connectedClients = 0;
    backend.maxItems = atoi(getenv("MAX_ITEMS"));
    backend.maxUsers = atoi(getenv("MAX_USERS"));
    backend.maxPromoters = atoi(getenv("MAX_PROMOTORS"));
    structThreadCredentials.backend = &backend;
    structThreadCredentials.promotor = &prt;

    pthread_t threadPromotor[threadCounter];
    pthread_t threadCredentials;
    pthread_t threadFrontendComms;
    pthread_t verifyAlive;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    backend.mutex = &mutex;

    structThreadCredentials.user = &usr;
    structThreadCredentials.item = &itm;

    // Thread creation

    launchPromotersThreads(&structThreadCredentials, threadPromotor);

    if (pthread_create(&threadCredentials, NULL, verifyCredentials, &structThreadCredentials) != 0)
        perror("Error creating thread");

    if (pthread_create(&threadFrontendComms, NULL, frontendComms, &structThreadCredentials) != 0)
        perror("Error creating thread");

    /*if(pthread_create(&verifyAlive, NULL, verifyUserAlive, &structThreadCredentials)!=0)
    {
        perror("Error creating thread");
    }*/

    // Signal handler

    struct sigaction sa;
    sa.sa_sigaction = crtlCSignal;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &sa, NULL);

    sleep(3);

    backendCommandReader(&structThreadCredentials, threadPromotor);

    // Thread join and mutex destroy

    while (threadCounter != 0)
    {
        pthread_join(threadPromotor[threadCounter], NULL);
        --threadCounter;
    }

    pthread_join(threadCredentials, NULL);
    pthread_join(threadFrontendComms, NULL);
    //pthread_join(verifyAlive, NULL);
    pthread_mutex_destroy(&mutex);

    pthread_exit((void *)NULL);
    unlink(BACKEND_FIFO);
    unlink(BACKEND_FIFO_FRONTEND);
    return 0;
}