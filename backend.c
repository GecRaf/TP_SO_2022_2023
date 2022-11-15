#include "backend.h"
#include "users_lib.h"
void users(){}
void list(){}
void kick(char username[]){}
void prom(){}
void reprom(){}
void cancelPromotor(){}

void quit(){
    printf("\n[!] Closing...\n\n");
    unlink(BACKEND_FIFO);
    unlink(BACKEND_FIFO_FRONTEND);
    killpromotor();
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
void readItemsFile()
{
    FILE *f;

    char *maxItemsChar = getenv("MAX_ITEMS");
    int maxItems = atoi(maxItemsChar);

    char *itemsFile = getenv("FITEMS");

    Item* it = (Item*)malloc(sizeof(Item)*maxItems);

    f = fopen(itemsFile, "r");
    if(f==NULL)
    {
        printf("\n[!] Error while opening the file ' items '\n");
        exit(EXIT_FAILURE);
    }
    else{
        printf("\n[~] Successuflly loaded file ' items '\n");
    }

    while(fscanf(f, "%d %s %s %d %d %d %s %s[^\n]", &it->id, it->name, it->category, &it->basePrice, &it->buyNowPrice, &it->duration, it->sellingUser, it->highestBidder) == 8){
        printf("\n\n%d %s %s %d %d %d %s %s\n\n", it->id, it->name, it->category, it->basePrice, it->buyNowPrice, it->duration, it->sellingUser, it->highestBidder); // Just for testing purposes, remove later
        ++it;
    }
    
    fclose(f);
    
}
void readUsersFile()
{
    FILE *f;
    char *maxUsersChar = getenv("MAX_USERS");
    int maxUsers = atoi(maxUsersChar);

    char *usersFile = getenv("FUSERS");

    User* u = (User*)malloc(sizeof(User)*maxUsers);

    f= fopen(usersFile, "r");
    if(f==NULL)
    {
        printf("\n[!] Error while opening the file ' users '\n");
        exit(EXIT_FAILURE);
    }
    else{
        printf("\n[~] Successuflly loaded file ' users '\n");
    }

    while(fscanf(f, "%s %s %d[^\n]", u->username, u->password, &u->balance) == 3)  
    {
        printf("\n\n%s %s %d\n\n", u->username, u->password, u->balance); // Just for testing purposes, remove later
        ++u;
    }

    fclose(f);
}
void readCredentials(){}
void promotorComms()
{
    int pipeBP[2], pipePB[2];
    int estado, num;
    pipe(pipeBP);
    pipe(pipePB);
    int pid = fork();
    if(pid == 0){
        close(STDIN_FILENO); // Close stdin
        close(STDOUT_FILENO); // Close stdout
        dup(pipeBP[0]); // Duplicate pipeBP[0] to read from pipe BackEnd -> Promotor
        dup(pipePB[1]); // Duplicate pipePB[1] to write to pipe Promotor -> BackEnd
        execl("./promotor_oficial", "./promotor_oficial", (char*)NULL); // Add eventual arguments
    }else{
        close(pipeBP[0]);
        close(pipePB[1]);
    }
}

void killpromotor(){
    close(pipePB[0]);
    kill(filho, SIGUSR2); 
}

int instanceController(){
    int backend_fd = open(BACKEND_FIFO, O_RDONLY | O_NONBLOCK);
    close(backend_fd);
    if(backend_fd == -1){
        return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if(!instanceController()){
        printf("\n[!] Another instance of the backend is already running\n");
        return 0;
    }// Checks if there is already an instance of the backend running

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

    // FIFO creation
    if(mkfifo(BACKEND_FIFO, 0666) == -1){
        printf("\n[!] Error while creating the backend FIFO\n");
        return 0;
    }
    if(mkfifo(BACKEND_FIFO_FRONTEND, 0666) == -1){
        printf("\n[!] Error while creating the backend_frontend FIFO\n");
        return 0;
    }
   
    promotorComms(); // Needs to integrate a thread
    readItemsFile(); 
    readUsersFile();
    readCredentials(); // Needs to integrate a thread
    backendCommandReader(); // Needs to integrate a thread

    return 0;
}