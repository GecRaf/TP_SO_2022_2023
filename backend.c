#include "utils.h"
#include "users_lib.h"

// Notes:
// 1. 'List' command keeps failing sometimes, randomly

int pipeBP[2], pipePB[2];
int threadCounter = 0;
int mustContinue = 1;

// Exclusivly to kill frontend processes in case of SIGINT
int frontendPIDArrayIndex = 0;
int frontendPIDArray[10];

// Exclusivly to kill promotor processes in case of quit()
int promotorPIDArrayIndex = 0;
int promotorPIDArray[10];

void quit(void *user)
{
    printf("\n\t[!] Closing...\n\n");
    
    if (user != NULL)
    {
        User *usr = (User *)user;
        while (usr != NULL)
        {
            if (usr->loggedIn == 1)
            {
                kill(usr->PID, SIGUSR1);
            }
            usr = usr->next;
        }
    }

    sleep(1);

    close(pipePB[0]);
    for (int i = 0; i < promotorPIDArrayIndex; i++)
    {
        kill(promotorPIDArray[i], SIGUSR1);
    }

    close(STDOUT_FILENO);
    close(STDIN_FILENO);
    fflush(stdout);
    mustContinue = 0;
}
void listUsers(void *user)
{
    FILE *f;

    char *usersFile = getenv("FUSERS");
    if (loadUsersFile(usersFile) == -1)
    {
        printf("\n\t[!] Error while loading users file [func: listUsers]\n");
        quit(NULL);
    }
    else
    {
        printf("\n\t[~] Successuflly loaded file ' users '\n");
        printf("\n\t[~] Successuflly read %d users\n", loadUsersFile(usersFile));
    }

    int maxUsers = atoi(getenv("MAX_USERS"));

    User *usr = (User *)user;

    f = fopen(usersFile, "r");
    if (f == NULL)
    {
        printf("\n\t[!] Error while opening file ' users ' [func: listUsers]\n");
        quit(NULL);
    }
    for (int i = 0; i < maxUsers; i++)
    {
        fscanf(f, "%s %s %d", usr->username, usr->password, &usr->balance);
        usr->loggedIn = 0;
        usr->PID = 0;
        usr->heartbeating = 1;
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
            printf("\n\t\t[~] ID: %d\n\t\t[~] Name: %s\n\t\t[~] Category: %s\n\t\t[~] Base Price: %d\n\t\t[~] Buy Now Price: %d\n\t\t[~] Higgest Bid: %d\n\t\t[~] Selling User: %s\n\t\t[~] Highest Bidder: %s\n", it->id, it->name, it->category, it->basePrice, it->buyNowPrice, it->highestBid, it->sellingUser, it->highestBidder);
        it = it->next;
    }
    printf("\n");
}
void kick(char username[], void *user)
{
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
void *promotorComms(void *prom)
{
    Promotor *promotor = (Promotor *)prom;
    // Estrutura esta a chegar vazia
    printf("\n\t[~] Promotor '%s' started\n", promotor->path);

    if (strcmp(promotor->path, "") == 0)
    {
        pthread_exit((void *)NULL);
    }

    char *promotorsFile = getenv("FPROMOTORS");
    char *maxPromotorsChar = getenv("MAX_PROMOTORS");
    int maxPromotors = atoi(maxPromotorsChar);

    char *promotorsExecutablesPath = promotor->path;

    int estado, num;
    pipe(pipeBP);
    pipe(pipePB);
    int pid = fork();

    if (pid == 0)
    {
        close(1);         // Close stdout
        dup(pipePB[1]);   // Duplicate pipePB[1] to write to pipe Promotor -> BackEnd
        close(pipePB[0]); // Close pipePB[0]
        close(pipePB[1]); // Close pipePB[1]
        if (execl(promotorsExecutablesPath, promotorsExecutablesPath, NULL) == -1)
        {
            printf("\n\t[!] Error while executing ' %s ' [func: promotorComms]\n", promotorsExecutablesPath);
            quit(NULL);
        }
    }
    else
    {
        promotor->PID = pid;
        promotorPIDArray[promotorPIDArrayIndex] = pid;
        promotorPIDArrayIndex++;
        // printf("\n\t[~] Promotor '%s' PID '%5d'\n", promotorsExecutablesPath, pid);
        int flag = 1;
        while (flag || mustContinue) // Review
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
                        strcpy(promotor->category, token);
                    }
                    else if (i == 1)
                    {
                        promotor->discount = atoi(token);
                    }
                    else if (i == 2)
                    {
                        promotor->duration = atoi(token);
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
}
void readPromoters(void *prt)
{
    FILE *f;
    char *promotersFile = getenv("FPROMOTERS");
    int maxPromoters = atoi(getenv("MAX_PROMOTORS"));

    f = fopen(promotersFile, "r");
    if (f == NULL)
    {
        printf("\n\t[!] Error while opening file '%s' [func: readPromoters]\n", promotersFile);
        quit(NULL);
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
        if (strcmp(prt->path, "") != 0 && (prt->active == 0 || prt->active == 2))
        {
            // Create thread for each promotor and send the exact promotor struct pointer
            if (pthread_create(&threadPromotor[threadCounter], NULL, (void *)promotorComms, (void *)prt) != 0)
                perror("Error creating thread promotor");
            printf("\n\t[+] Promotor thread created '%s'\n", prt->path);
            prt->active = 1;
            prt->threadID = threadCounter;
        }
        prt = prt->next;
        threadCounter--;
    }
}
void debugPromotores(void *structThreadCredentials)
{
    // Print all existing promotors and details
    StructThreadCredentials *mainStruct = (StructThreadCredentials *)structThreadCredentials;
    Promotor *prt = (Promotor *)mainStruct->promotor;
    int maxPromotors = atoi(getenv("MAX_PROMOTORS"));

    for (int i = 0; i < maxPromotors; i++)
    {
        if (strcmp(prt->path, "") != 0)
        {
            printf("\n[~] Promotor %d: %s\n", i, prt->path);
            printf("\t[~] Category: %s\n", prt->category);
            printf("\t[~] Discount: %d\n", prt->discount);
            printf("\t[~] Duration: %d\n", prt->duration);
            printf("\t[~] Active: %d\n", prt->active);
            printf("\t[~] Thread ID: %d\n", prt->threadID);
        }
        prt = prt->next;
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
    // Sometimes it's working, sometimes it's not
    StructThreadCredentials *mainStruct = (StructThreadCredentials *)structThreadCredentials;
    Promotor *prt = (Promotor *)mainStruct->promotor;
    int maxPromotors = atoi(getenv("MAX_PROMOTORS"));
    char *promotersFile = getenv("FPROMOTERS");

    FILE *f;
    f = fopen(promotersFile, "r");
    if (f == NULL)
    {
        printf("\n[!] Error while opening file '%s' [func: reprom]\n", promotersFile);
        quit(NULL);
    }

    // Print file line by line
    char line[100];
    int newPromotors = 0;

    printf("\n\t[~] List of new promoters:\n");

    while (fgets(line, 100, f) == line)
    {
        prt = (Promotor *)mainStruct->promotor;
        // printf("\n\t\t[~] %s\n", line);
        //  Check if line as \n and remove it
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        // Concatenate txt/ to the path
        char path[100] = "txt/";
        strcat(path, line);
        strcpy(line, path);

        int flag = 0;
        for (int i = 0; i < maxPromotors; i++)
        {
            if (strcmp(prt->path, line) == 0)
            {
                printf("\n\t\t[+] Promotor already exists '%s' and is active\n", prt->path);
                flag = 1;
                prt->active = 3;
                break;
            }
            prt = prt->next;
        }
        if (flag == 0)
        {
            prt = (Promotor *)mainStruct->promotor;
            for (int j = 0; j < maxPromotors; j++)
            {
                if (strcmp(prt->path, "") == 0)
                {
                    strcpy(prt->path, line);
                    prt->active = 2;
                    printf("\n\t\t[+] Promotor added '%s'\n", prt->path);
                    threadCounter++;
                    newPromotors++;
                    break;
                }
                prt = prt->next;
            }
        }
    }
    prt = (Promotor *)mainStruct->promotor;
    for (int i = 0; i < maxPromotors; i++)
    {
        if (strcmp(prt->path, "") != 0 && prt->active != 3 && prt->active != 2)
        {
            printf("\n\t\t[+] Promotor '%s' existed and now it's gone\n", prt->path);
            kill(prt->PID, SIGUSR1);
            waitpid(prt->PID, NULL, 0);
            printf("\n\t\t[+] Promotor thread killed '%s'\n\n", prt->path);
            prt->active = 0;
        }
        prt = prt->next;
    }
    prt = (Promotor *)mainStruct->promotor;
    for (int i = 0; i < maxPromotors; i++)
    {
        if (strcmp(prt->path, "") != 0 && prt->active == 3)
        {
            prt->active = 1;
        }
        prt = prt->next;
    }
    prt = (Promotor *)mainStruct->promotor;
    launchPromotersThreads(structThreadCredentials, threadPromotor);
    if (newPromotors == 0)
        printf("\n\t\t[!] No new promoters found\n");
    fclose(f);
    printf("\n");
}
void cancelPromotor(void *structThreadCredentials, pthread_t *threadPromotor, char path[])
{
    StructThreadCredentials *mainStruct = (StructThreadCredentials *)structThreadCredentials;
    Promotor *prt = (Promotor *)mainStruct->promotor;

    int notFound = 0;

    while (prt != NULL)
    {
        if (strcmp(prt->path, path) == 0)
        {
            // TODO: Kill thread without crashing the program
            kill(prt->PID, SIGUSR1);
            waitpid(prt->PID, NULL, 0);
            printf("\n\t[+] Promotor thread killed '%s'\n\n", prt->path);
            prt->active = 0;
            notFound = 1;
            break;
        }
        prt = prt->next;
    }
    if (notFound == 0)
    {
        printf("\n\t[!] Path not found\n");
        printf("\n\t[!] Use command <prom> to see the active promoters\n\n");
    }
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

    while (mustContinue)
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
                printf("\n\t[!] Invalid notation for command ' users '\n");
                printf("\t[!] No arguments needed\n");
                continue;
            }
            loggedIn(usr); // Lists users using the platform ATM
        }
        else if (strcmp(cmd, "list") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n\t[!] Invalid notation for command ' list '\n");
                printf("\t[!] No arguments needed\n");
                continue;
            }
            list(it); // Lists available items
        }
        else if (strcmp(cmd, "kick") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n\t[!] Invalid notation for command ' kick '\n");
                printf("\t[~] Use the following notation: 'kick <username>'\n");
                continue;
            }
            kick(arg, usr); // Kicks certain user by the username stored in "arg"
            *arg = 0;       // Clears arg char array
        }
        else if (strcmp(cmd, "prom") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n\t[!] Invalid notation for command ' prom '\n");
                printf("\t[!] No arguments needed\n");
                continue;
            }
            prom(prt); // Lists active promotors
        }
        else if (strcmp(cmd, "reprom") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n\t[!] Invalid notation for command ' reprom '\n");
                printf("\t[!] No arguments needed\n");
                continue;
            }
            reprom(mainStruct, threadPromotor);
        }
        else if (strcmp(cmd, "cancel") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n\t[!] Invalid notation for command ' cancel '\n");
                printf("\t[~] Use the following notation: 'cancel <promotors-executable-filename>'\n\n");
                continue;
            }
            cancelPromotor(mainStruct, threadPromotor, arg); // Cancels promotor
        }
        else if (strcmp(cmd, "close") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n\t[!] Invalid notation for command ' close '\n");
                printf("\t[!] No arguments needed\n");
                continue;
            }
            quit(usr);
        }
        else if (strcmp(cmd, "clear") == 0)
        {
            if (!(strcmp(arg, "") == 0))
            {
                printf("\n\t[!] Invalid notation for command ' clear '\n");
                printf("\t[!] No arguments needed\n");
                continue;
            }
            clear();
        }
        else if (strcmp(cmd, "help") == 0)
        {
            printf("\n\t\t[!] Available commands:\n");
            printf("\t[~] users || Lists users using the platform\n");
            printf("\t[~] list || Lists available items\n");
            printf("\t[~] kick <username> || Kicks a certain user\n");
            printf("\t[~] prom || Lists active promotors\n");
            printf("\t[~] reprom || Refresh active promotors\n");
            printf("\t[~] cancel <promotors-executable-filename> || Cancels promotor\n");
            printf("\t[~] close || Closes platform\n");
            printf("\t[~] clear || Console clear\n\n");
        }
        else
        {
            printf("\n\t[!] Command not found: '%s' (For help type: 'help')\n\n", cmd);
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
        printf("\n[!] Error while opening the file 'items' [func: readItemsFile]\n");
        quit(NULL);
    }
    else
    {
        printf("\n\t[~] Successuflly loaded file 'items' [func: readItemsFile]\n");
    }

    int count = 0;

    for (int i = 0; i < maxItems; i++)
    {
        if (fscanf(f, "%d %s %s %d %d %d", &it->id, it->name, it->category, &it->basePrice, &it->buyNowPrice, &it->duration) == 6)
            count++;
        strcpy(it->highestBidder, "N/A");
        strcpy(it->sellingUser, "N/A");
        it->highestBid = 0;
        it->next = (Item *)malloc(sizeof(Item));
        it = it->next;
    }

    printf("\n\t[~] Successuflly read %d items\n", count);

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
        printf("\n\r[!] Error while opening pipe 'BACKEND_FIFO_FRONTEND' [func: frontendComms]\n");
        quit(NULL);
    }

    while (mustContinue)
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
            else if (strcmp(comms.message, "buy") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'", comms.message, comms.username);
                while (user_ptr != NULL)
                {
                    if (user_ptr->PID == comms.PID)
                    {
                        if (user_ptr->balance < comms.balance)
                        {
                            printf("\n\n\t[!] User '%s' doesn't have enough balance to buy item with ID '%d'", comms.username, comms.buyID);
                            comms.buyID = 1;
                            sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                            int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                            if (fd == -1)
                            {
                                printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            int size2 = write(fd, &comms, sizeof(comms));
                            if (size2 == -1)
                            {
                                printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            close(fd);
                            break;
                        }
                        else
                        {
                            // If the value offered by the user is equal to the buy now price, the item is sold
                            // If the value offered by the user is equal or greater than the base price and the highest bidder, the bid is accepted and he becomes the highest bidder\

                            while (item_ptr != NULL)
                            {
                                if (item_ptr->id == comms.buyID)
                                {
                                    if (comms.balance == item_ptr->buyNowPrice)
                                    {

                                        printf("\n\n\t[~] User '%s' bought item with ID '%d' for '%d' euros", comms.username, comms.buyID, comms.balance);
                                        user_ptr->balance -= comms.balance;
                                        strcpy(item_ptr->sellingUser, comms.username);
                                        item_ptr->bought = 1;
                                        comms.buyID = 2;
                                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                                        if (fd == -1)
                                        {
                                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                            quit(NULL);
                                        }
                                        int size2 = write(fd, &comms, sizeof(comms));
                                        if (size2 == -1)
                                        {
                                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                            quit(NULL);
                                        }
                                        close(fd);
                                        break;
                                    }
                                    else if (comms.balance >= item_ptr->basePrice && comms.balance > item_ptr->highestBid)
                                    {
                                        printf("\n\n\t[~] User '%s' bid '%d' euros for item with ID '%d'", comms.username, comms.balance, comms.buyID);
                                        item_ptr->highestBid = comms.balance;
                                        strcpy(item_ptr->highestBidder, comms.username);
                                        comms.buyID = 3;
                                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                                        if (fd == -1)
                                        {
                                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                            quit(NULL);
                                        }
                                        int size2 = write(fd, &comms, sizeof(comms));
                                        if (size2 == -1)
                                        {
                                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                            quit(NULL);
                                        }
                                        close(fd);
                                        break;
                                    }
                                    else
                                    {
                                        if (item_ptr->highestBid > item_ptr->basePrice)
                                        {
                                            comms.balance = item_ptr->highestBid;
                                        }
                                        else
                                        {
                                            comms.balance = item_ptr->basePrice;
                                        }
                                        printf("\n\n\t[!] User '%s' bid '%d' euros for item with ID '%d' but it was rejected", comms.username, comms.balance, comms.buyID);
                                        comms.buyID = 4;
                                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                                        if (fd == -1)
                                        {
                                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                            quit(NULL);
                                        }
                                        int size2 = write(fd, &comms, sizeof(comms));
                                        if (size2 == -1)
                                        {
                                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: buy]\n", FRONTEND_FINAL_FIFO);
                                            quit(NULL);
                                        }
                                        close(fd);
                                        break;
                                    }
                                }
                                item_ptr = item_ptr->next;
                            }
                        }
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
                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: cash]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        int size2 = write(fd, &comms, sizeof(comms));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: cash]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
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
                    printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: add]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                // Write comms to frontend
                int size2 = write(fd, &comms, sizeof(comms));
                if (size2 == -1)
                {
                    printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: add]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                close(fd);
            }
            else if (strcmp(comms.message, "sell") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                int fd = open(BACKEND_FIFO_FRONTEND, O_RDONLY);
                if (fd == -1)
                {
                    printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: sell]\n", BACKEND_FIFO_FRONTEND);
                    quit(NULL);
                }
                int size = read(fd, &it, sizeof(it));
                if (size == -1)
                {
                    printf("\n\t[!] Error while reading from pipe '%s' [func: frontendComms | command: sell]\n", BACKEND_FIFO_FRONTEND);
                    quit(NULL);
                }
                close(fd);

                int id = 0;
                while (item_ptr != NULL)
                {
                    if (item_ptr->id == 0)
                    {
                        pthread_mutex_lock(backend_ptr->mutex);
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
                        pthread_mutex_unlock(backend_ptr->mutex);
                        // Send int with the id of the item to the frontend
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd2 = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd2 == -1)
                        {
                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: sell]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        printf("\t[~] Item '%s' added to the database with ID %d\n", it.name, it.id);
                        int id = it.id;
                        int size2 = write(fd2, &id, sizeof(id));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: sell]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        close(fd2);

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
                while (item_ptr != NULL)
                {
                    if (item_ptr->id != 0 && item_ptr->bought != 1 && item_ptr->duration > 0)
                    {
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: list]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        int size2 = write(fd, item_ptr, sizeof(Item));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: list]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        close(fd);
                    }
                    item_ptr = item_ptr->next;
                }
                Item end;
                end.id = -1;
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd == -1)
                {
                    printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: list]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                int size2 = write(fd, &end, sizeof(Item));
                if (size2 == -1)
                {
                    printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: list]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                close(fd);
            }
            else if (strcmp(comms.message, "licat") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                while (item_ptr != NULL)
                {
                    if (item_ptr->id != 0 && strcmp(item_ptr->category, comms.argument) == 0 && item_ptr->duration > 0)
                    {
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: licat]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        int size2 = write(fd, item_ptr, sizeof(Item));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: licat]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        close(fd);
                    }
                    item_ptr = item_ptr->next;
                }
                Item end;
                end.id = -1;
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd == -1)
                {
                    printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: licat]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                int size2 = write(fd, &end, sizeof(Item));
                if (size2 == -1)
                {
                    printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: licat]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                close(fd);
            }
            else if (strcmp(comms.message, "lisel") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                while (item_ptr != NULL)
                {
                    if (item_ptr->id != 0 && strcmp(item_ptr->sellingUser, comms.argument) == 0 && item_ptr->duration > 0)
                    {
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: lisel]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        int size2 = write(fd, item_ptr, sizeof(Item));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: lisel]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        close(fd);
                    }
                    item_ptr = item_ptr->next;
                }
                Item end;
                end.id = -1;
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd == -1)
                {
                    printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: lisel]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                int size2 = write(fd, &end, sizeof(Item));
                if (size2 == -1)
                {
                    printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: lisel]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                close(fd);
            }
            else if (strcmp(comms.message, "lival") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                while (item_ptr != NULL)
                {
                    if (item_ptr->id != 0 && item_ptr->buyNowPrice <= comms.balance && item_ptr->duration > 0)
                    {
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: lival]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        int size2 = write(fd, item_ptr, sizeof(Item));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: lival]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        close(fd);
                    }
                    item_ptr = item_ptr->next;
                }
                Item end;
                end.id = -1;
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd == -1)
                {
                    printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: lival]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                int size2 = write(fd, &end, sizeof(Item));
                if (size2 == -1)
                {
                    printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: lival]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                close(fd);
            }
            else if (strcmp(comms.message, "litime") == 0)
            {
                printf("\n\n\t[~] '%s' Action taken by '%s'\n", comms.message, comms.username);
                while (item_ptr != NULL)
                {
                    time_t actual_time = time(NULL);
                    struct tm *actual_time_tm = localtime(&actual_time);
                    int actual_time_seconds = actual_time_tm->tm_hour * 3600 + actual_time_tm->tm_min * 60 + actual_time_tm->tm_sec;

                    if (item_ptr->id != 0 && (actual_time_seconds + item_ptr->duration) <= (comms.balance + actual_time_seconds) && item_ptr->duration > 0)
                    {
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: litime]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        int size2 = write(fd, item_ptr, sizeof(Item));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: litime]\n", FRONTEND_FINAL_FIFO);
                            quit(NULL);
                        }
                        close(fd);
                    }
                    item_ptr = item_ptr->next;
                }
                Item end;
                end.id = -1;
                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, comms.PID);
                int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                if (fd == -1)
                {
                    printf("\n\t[!] Error while opening pipe '%s' [func: frontendComms | command: litime]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                int size2 = write(fd, &end, sizeof(Item));
                if (size2 == -1)
                {
                    printf("\n\t[!] Error while writing to pipe '%s' [func: frontendComms | command: litime]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                close(fd);
            }
        }
    }
    pthread_exit((void *)NULL);
    close(fd);
}
void *removeUserNotAlive(void *structThreadCredentials)
{
    StructThreadCredentials *structThreadCredentials_ptr = (StructThreadCredentials *)structThreadCredentials;
    User *user_ptr = (User *)structThreadCredentials_ptr->user;
    int heartbeat = atoi(getenv("HEARTBEAT"));

    while (mustContinue)
    {
        sleep(heartbeat + 10); // This was working with 10 seconds here and 3 seconsds in the frontend [IN CASE OF FAILING, TRY TO CHANGE THIS]
        User *user_ptr = (User *)structThreadCredentials_ptr->user;
        while (user_ptr != NULL)
        {
            if (user_ptr->heartbeating == 0 && user_ptr->loggedIn == 1)
            {
                user_ptr->loggedIn = 0;
                printf("\n\t[~] User '%s' has been removed from the system\n", user_ptr->username);
                user_ptr->heartbeating = 1;
            }
            user_ptr->heartbeating = 0;
            user_ptr = user_ptr->next;
        }
    }
    pthread_exit((void *)NULL);
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
        printf("\n\t[!] Error while opening pipe '%s' [func: verifyCredentials]\n", BACKEND_FIFO);
        quit(NULL);
    }

    while (mustContinue)
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
                    printf("\n\t[!] Error while opening pipe '%s' [func: verifyCredentials]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
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
                            user_ptr->heartbeating = 1;
                            user_ptr->loggedIn = 1;
                            user_ptr->PID = user.PID;
                            // Store PID in the global variable
                            frontendPIDArray[frontendPIDArrayIndex] = user.PID;
                            frontendPIDArrayIndex++;
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
                    printf("\n\t[!] Error while writing to pipe '%s' [func: verifyCredentials]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
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
                    printf("\n\t[!] Error while opening pipe '%s' [func: verifyCredentials]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                int result = -2;
                int size2 = write(fd2, &result, sizeof(result));
                if (size2 == -1)
                {
                    printf("\n\t[!] Error while writing to pipe '%s' [func: verifyCredentials]\n", FRONTEND_FINAL_FIFO);
                    quit(NULL);
                }
                close(fd2);
            }
        }
        // pthread_mutex_unlock(backend_ptr->mutex);
    }
    close(fd);
    pthread_exit((void *)NULL);
}
void crtlCSignal()
{
    for (int i = 0; i < frontendPIDArrayIndex; i++)
    {
        kill(frontendPIDArray[i], SIGUSR1);
    }
    quit(NULL);
}
void *itemActions(void *structThreadCredentials)
{
    StructThreadCredentials *structThreadCredentials_ptr = (StructThreadCredentials *)structThreadCredentials;
    User *user_ptr = (User *)structThreadCredentials_ptr->user;
    Item *item_ptr = (Item *)structThreadCredentials_ptr->item;
    Backend *backend_ptr = (Backend *)structThreadCredentials_ptr->backend;
    // Function to simply decrease the duration of the items evert second
    // If the duration of an item is 0, the item is removed from the list
    while (mustContinue)
    {
        item_ptr = (Item *)structThreadCredentials_ptr->item;
        sleep(1);
        pthread_mutex_lock(backend_ptr->mutex);
        while (item_ptr != NULL)
        {
            // Check if the struct name is not empty
            if (strcmp(item_ptr->name, "") != 0)
            {
                // Check if the duration is greater than 0
                if (item_ptr->duration > 0)
                {
                    item_ptr->duration--;
                }
                else if (item_ptr->duration == 0)
                {
                    item_ptr->duration = -1;
                    // Print this only once
                    if (item_ptr->duration == -1)
                    {
                        printf("\n\n\t[~] Item '%s' with ID '%d' timed out\n\n", item_ptr->name, item_ptr->id);
                        while (user_ptr != NULL)
                        {
                            if (user_ptr->loggedIn == 1)
                            {
                                char FRONTEND_FINAL_FIFO[100];
                                sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, user_ptr->PID);
                                int fd = open(FRONTEND_FINAL_FIFO, O_RDWR);
                                if (fd == -1)
                                {
                                    printf("\n\t[!] Error while opening pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                    quit(NULL);
                                }
                                Comms comms;
                                strcpy(comms.username, item_ptr->name);
                                comms.buyID = item_ptr->id;
                                strcpy(comms.message, "ItemTimedOut");
                                int size = write(fd, &comms, sizeof(comms));
                                if (size == -1)
                                {
                                    printf("\n\t[!] Error while writing to pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                    quit(NULL);
                                }
                                close(fd);
                            }
                            user_ptr = user_ptr->next;
                        }
                    }
                }
                else if (item_ptr->bought == 1)
                {
                    item_ptr->bought = -1;
                    printf("\n\n\t[~] Item '%s' with ID '%d' bought by '%s'\n\n", item_ptr->name, item_ptr->id, item_ptr->sellingUser);
                    while (user_ptr != NULL)
                    {
                        if (user_ptr->loggedIn == 1)
                        {
                            char FRONTEND_FINAL_FIFO[100];
                            sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, user_ptr->PID);
                            int fd = open(FRONTEND_FINAL_FIFO, O_RDWR);
                            if (fd == -1)
                            {
                                printf("\n\t[!] Error while opening pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            Comms comms;
                            strcpy(comms.message, "Bought");
                            int size = write(fd, &comms, sizeof(comms));
                            if (size == -1)
                            {
                                printf("\n\t[!] Error while writing to pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            // Write the item struct to frontend
                            int size2 = write(fd, item_ptr, sizeof(Item));
                            if (size2 == -1)
                            {
                                printf("\n\t[!] Error while writing to pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            close(fd);
                        }
                        user_ptr = user_ptr->next;
                    }
                }
            }
            else if (strcmp(item_ptr->sellingUser, "N/A") != 0){
                if(item_ptr->bought != 1){
                    // Send new item to frontend
                    while (user_ptr != NULL)
                    {
                        if (user_ptr->loggedIn == 1)
                        {
                            char FRONTEND_FINAL_FIFO[100];
                            sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, user_ptr->PID);
                            int fd = open(FRONTEND_FINAL_FIFO, O_RDWR);
                            if (fd == -1)
                            {
                                printf("\n\t[!] Error while opening pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            Comms comms;
                            strcpy(comms.message, "NewItem");
                            int size = write(fd, &comms, sizeof(comms));
                            if (size == -1)
                            {
                                printf("\n\t[!] Error while writing to pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            // Write the item struct to frontend
                            int size2 = write(fd, item_ptr, sizeof(Item));
                            if (size2 == -1)
                            {
                                printf("\n\t[!] Error while writing to pipe '%s' [func: itemActions]\n", FRONTEND_FINAL_FIFO);
                                quit(NULL);
                            }
                            close(fd);
                        }
                        user_ptr = user_ptr->next;
                    }
                }
            }
            item_ptr = item_ptr->next;
        }
        pthread_mutex_unlock(backend_ptr->mutex);
    }
    pthread_exit((void *)NULL);
}
void *verifyUserAlive(void *structThreadCredentials)
{
    StructThreadCredentials *structThreadCredentials_ptr = (StructThreadCredentials *)structThreadCredentials;
    User *user_ptr = (User *)structThreadCredentials_ptr->user;
    int rec = 0;
    char message[100];
    int fd = open(ALIVE_FIFO, O_RDWR);
    if (fd == -1)
    {
        printf("\n\t[!] Error while opening pipe '%s' [func: veriyUserAlive]\n", ALIVE_FIFO);
        quit(NULL);
    }
    while (mustContinue)
    {
        user_ptr = (User *)structThreadCredentials_ptr->user;
        int size = read(fd, &rec, sizeof(rec));
        if (size > 0)
        {
            while (user_ptr != NULL)
            {
                if (user_ptr->PID == rec)
                {
                    if (user_ptr->loggedIn == 1)
                    {
                        // printf("\n[~] User '%s' is alive\n", user_ptr->username);
                        user_ptr->heartbeating = 1;
                    }
                }
                user_ptr = user_ptr->next;
            }
        }
    }
    close(fd);
    pthread_exit((void *)NULL);
}
void *promotorMessages(void *structThreadCredentials){
    // Check through all the promotor structs and send messages to the frontend
    StructThreadCredentials *structThreadCredentials_ptr = (StructThreadCredentials *)structThreadCredentials;
    Backend *backend_ptr = (Backend *)structThreadCredentials_ptr->backend;
    Promotor *promotor_ptr = (Promotor *)structThreadCredentials_ptr->promotor;
    User *user_ptr = (User *)structThreadCredentials_ptr->user;

    while (mustContinue)
    {
        promotor_ptr = structThreadCredentials_ptr->promotor;
        pthread_mutex_lock(backend_ptr->mutex);
        while (promotor_ptr != NULL)
        {
            if(strcmp(promotor_ptr->category, "") != 0 && promotor_ptr->duration == 0){
                promotor_ptr->listed = 2;
            }
            else if (strcmp(promotor_ptr->category, "") != 0 && promotor_ptr->listed != 1)
            {
                promotor_ptr->listed = 1;
                user_ptr = structThreadCredentials_ptr->user;
                while (user_ptr != NULL)
                {
                    if (user_ptr->loggedIn == 1)
                    {
                        char FRONTEND_FINAL_FIFO[100];
                        sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, user_ptr->PID);
                        int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
                        if (fd == -1)
                        {
                            printf("\n\t[!] Error while opening pipe '%s' [func: promotorMessages]\n", FRONTEND_FINAL_FIFO);
                            exit(EXIT_FAILURE);
                        }
                        Comms comms;
                        strcpy(comms.message, "Promotor");
                        int size = write(fd, &comms, sizeof(comms));
                        if (size == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: promotorMessages]\n", FRONTEND_FINAL_FIFO);
                            exit(EXIT_FAILURE);
                        }
                        // Write the promotor struct to frontend
                        int size2 = write(fd, promotor_ptr, sizeof(Promotor));
                        if (size2 == -1)
                        {
                            printf("\n\t[!] Error while writing to pipe '%s' [func: promotorMessages]\n", FRONTEND_FINAL_FIFO);
                            exit(EXIT_FAILURE);
                        }
                        close(fd);
                    }
                    user_ptr = user_ptr->next;
                }
            }
            promotor_ptr = promotor_ptr->next;
        }
        pthread_mutex_unlock(backend_ptr->mutex);
        sleep(1);
    }
    pthread_exit((void *)NULL);
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

    if (mkfifo(COMMS_FIFO, 0666) == -1)
    {
        printf("\n[!] Error while creating the backendFrontend FIFO\n");
        return 0;
    }
    if (mkfifo(ALIVE_FIFO, 0666) == -1)
    {
        printf("\n[!] Error while creating the alive FIFO\n");
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
    pthread_t threadItemActions;
    pthread_t verifyAlive;
    pthread_t removeUser;
    pthread_t threadPromotorMessages;
    pthread_mutex_t mutex;
    pthread_t threads[6] = {threadCredentials, threadFrontendComms, threadItemActions, verifyAlive, removeUser, threadPromotorMessages};
    pthread_mutex_init(&mutex, NULL);
    backend.mutex = &mutex;

    structThreadCredentials.user = &usr;
    structThreadCredentials.item = &itm;

    // Thread creation

    launchPromotersThreads(&structThreadCredentials, threadPromotor);

    if (pthread_create(&threadCredentials, NULL, verifyCredentials, &structThreadCredentials) != 0)
        perror("Error creating thread 'threadCredentials'");

    if (pthread_create(&threadFrontendComms, NULL, frontendComms, &structThreadCredentials) != 0)
        perror("Error creating thread 'threadFrontendComms'");

    if (pthread_create(&threadItemActions, NULL, itemActions, &structThreadCredentials) != 0)
        perror("Error creating thread 'threadItemActions'");

    if (pthread_create(&verifyAlive, NULL, verifyUserAlive, &structThreadCredentials) != 0)
    {
        perror("Error creating thread 'verifyAlive'");
    }
    if (pthread_create(&removeUser, NULL, removeUserNotAlive, &structThreadCredentials) != 0)
    {
        perror("Error creating thread 'removeUser'");
    }
    if (pthread_create(&threadPromotorMessages, NULL, promotorMessages, &structThreadCredentials) != 0)
    {
        perror("Error creating thread 'promotorMessages'");
    }

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
    int i = 0;
    for (i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);

    // FIFO unlink

    unlink(BACKEND_FIFO);
    unlink(BACKEND_FIFO_FRONTEND);
    unlink(COMMS_FIFO);
    unlink(ALIVE_FIFO);
    return 0;
}