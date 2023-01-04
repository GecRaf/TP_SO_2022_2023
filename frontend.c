#include "utils.h"

int sell(char itemName[], char category[], int basePrice, int buyNowPrice, int duration, char sellerUsername[])
{
    Item item;
    strcpy(item.name, itemName);
    strcpy(item.category, category);
    item.basePrice = basePrice;
    item.buyNowPrice = buyNowPrice;
    item.duration = duration;
    strcpy(item.sellingUser, sellerUsername);
    strcpy(item.highestBidder, "N/A");
    item.bought = 0;

    int fd = open(BACKEND_FIFO_FRONTEND, O_WRONLY);
    if (fd == -1)
    {
        printf("\n[!] Error opening backend fifo BACKEND_FIFO_FRONTEND\n\n");
        exit(EXIT_FAILURE);
    }
    int size = write(fd, &item, sizeof(Item));
    if (size == -1)
    {
        printf("\n[!] Error writing to backend fifo BACKEND_FIFO_FRONTEND\n\n");
        exit(EXIT_FAILURE);
    }
    close(fd);

    sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, getpid());
    fd = open(FRONTEND_FINAL_FIFO, O_RDONLY);
    if (fd == -1)
    {
        printf("\n[!] Error opening frontend fifo FRONTEND_FINAL_FIFO\n\n");
        exit(EXIT_FAILURE);
    }
    size = read(fd, &item, sizeof(Item));
    if (size == -1)
    {
        printf("\n[!] Error reading from frontend fifo FRONTEND_FINAL_FIFO\n\n");
        exit(EXIT_FAILURE);
    }
    close(fd);

    if (item.id != -1)
        return item.id; // In case of success
    else
        return -1; // In case of insuccess
}
void list()
{
    // Receive item struct from backend and print it until its NULL
    Item item;
    sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, getpid());
    int fd = open(FRONTEND_FINAL_FIFO, O_RDONLY);
    if (fd == -1)
    {
        printf("\n[!] Error opening backend fifo BACKEND_FIFO_FRONTEND\n\n");
        exit(EXIT_FAILURE);
    }
    int size = read(fd, &item, sizeof(Item));
    if (size == -1)
    {
        printf("\n[!] Error reading from backend fifo BACKEND_FIFO_FRONTEND\n\n");
        exit(EXIT_FAILURE);
    }
    close(fd);

    printf("\n\t[~] ----------------------------\n");
    while (item.id != -1)
    {
        printf("\n\t[~] Item %d\n", item.id);
        printf("\t[~] Name: %s\n", item.name);
        printf("\t[~] Category: %s\n", item.category);
        printf("\t[~] Base Price: %d\n", item.basePrice);
        printf("\t[~] Buy Now Price: %d\n", item.buyNowPrice);
        printf("\t[~] Duration: %d\n", item.duration);
        printf("\t[~] Seller: %s\n", item.sellingUser);
        if (strcmp(item.highestBidder, "N/A") != 0)
            printf("\t[~] Highest Bidder: %s\n", item.highestBidder);
        printf("\n\t[~] ----------------------------\n");

        memset(&item, 0, sizeof(Item));

        fd = open(FRONTEND_FINAL_FIFO, O_RDONLY);
        if (fd == -1)
        {
            printf("\n[!] Error opening backend fifo BACKEND_FIFO_FRONTEND\n\n");
            exit(EXIT_FAILURE);
        }
        size = read(fd, &item, sizeof(Item));
        if (size == -1)
        {
            printf("\n[!] Error reading from backend fifo BACKEND_FIFO_FRONTEND\n\n");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    printf("\n");
}
void licat(char category[])
{
    printf("\ncategory: %s\n\n", category);
}
void lisel(char sellerUsername[])
{
    printf("\nsellerUsername: %s\n\n", sellerUsername);
}
void lival(int maxPrice)
{
    printf("\nmaxPrice: %d\n\n", maxPrice);
}
void litime(int duration)
{
    printf("\nduration: %d\n\n", duration);
}
void currentTime(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("\n\t[~] Current time: %d-%d-%d %d:%d:%d\n\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}
void buy(int id, int value)
{
    printf("\nid: %d\nvalue: %d\n\n", id, value);
}
void add(int value)
{
    printf("\nvalue: %d\n\n", value);
}
void quit()
{
    Comms comms;
    strcpy(comms.message, "exit");
    comms.PID = getpid();

    int fd = open(BACKEND_FIFO_FRONTEND, O_WRONLY);
    if (fd == -1)
    {
        printf("\n[!] Error opening backend fifo BACKEND_FIFO_FRONTEND\n\n");
        exit(EXIT_FAILURE);
    }
    int size = write(fd, &comms, sizeof(Comms));
    if (size == -1)
    {
        printf("\n[!] Error writing to backend fifo BACKEND_FIFO_FRONTEND\n\n");
        exit(EXIT_FAILURE);
    }
    close(fd);

    printf("\n[!] Exiting...\n\n");
    unlink(FRONTEND_FINAL_FIFO);
    sleep(1);
    exit(EXIT_SUCCESS);
}
void quit2()
{
    printf("\n[!] Exiting...\n\n");
    unlink(FRONTEND_FINAL_FIFO);
    sleep(1);
    exit(EXIT_SUCCESS);
}
void clear()
{
    system("clear");
}
void imAlive()
{

    printf("hello?");

    Comms comms;
    comms.PID = getpid();
    sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, getpid());
    int fd = open(FRONTEND_FINAL_FIFO, O_WRONLY);
    if (fd == -1)
    {
        printf("\nError communicating it's alive");
        quit();
    }
    int answer = 1;
    int size = write(fd, &answer, sizeof(int));
    if (size == -1)
    {
        printf("\nError communicating it's alive");
        quit();
    }
    close(fd);
}
void *threadAlive(void *user_ptr)
{
    int heartbeat = atoi(getenv("HEARTBEAT"));
    sleep(heartbeat);
    // Waiting for signal SIGUSR2 from backend and then sending alive signal SIGUSR1
    while (1)
    {
        signal(SIGUSR2, imAlive);
    }
    pthread_exit((void *)NULL);
}
void *frontendCommandReader(void *user_ptr)
{
    User *usr = (User *)user_ptr;

    Comms comms;
    comms.PID = getpid();
    strcpy(comms.username, usr->username);

    int fd = open(BACKEND_FIFO_FRONTEND, O_WRONLY);
    if (fd == -1)
    {
        printf("\n[!] Error opening backend fifo BACKEND_FIFO_FRONTEND\n\n");
        exit(EXIT_FAILURE);
    }
    clear();
    int pid = getpid();
    char command[50], cmd[15], arg[30];

    printf("\nfrontendCommandReader @ type help for command list\n\n");

    while (1)
    {
        *arg = 0;
        *cmd = 0;
        printf("[%5d] COMMAND: ", pid);
        scanf(" %49[^\n]", command);
        sscanf(command, "%14s %29[^\n]", cmd, arg);

        if (strcmp(cmd, "sell") == 0)
        {
            char itemName[15] = "", category[15] = "", basePriceChar[10] = "", buyNowPriceChar[10] = "", durationChar[10] = "";
            if (sscanf(arg, "%14s %14s %9s %9s %9[^\n]", itemName, category, basePriceChar, buyNowPriceChar, durationChar) == 5)
            {
                int basePrice = atoi(basePriceChar), buyNowPrice = atoi(buyNowPriceChar), duration = atoi(durationChar);
                if (strcmp(itemName, "") == 0)
                {
                    printf("\n\t[!] Invalid notation for command ' sell ' in <item-name>\n\n");
                    continue;
                }
                else if (strcmp(category, "") == 0)
                {
                    printf("\n\t[!] Invalid notation for command ' sell ' in <category>\n\n");
                    continue;
                }
                else if (basePrice == 0)
                {
                    printf("\n\t[!] Invalid notation for command ' sell '! Base Price must be > 0 \n\n");
                    continue;
                }
                else if (buyNowPrice == 0)
                {
                    printf("\n\t[!] Invalid notation for command ' sell '! Buy Now Price must be > 0 \n\n");
                    continue;
                }
                else if (duration == 0)
                {
                    printf("\n\t[!] Invalid notation for command ' sell '! Duration must be > 0 \n\n");
                    continue;
                }
                strcpy(comms.message, cmd);
                int size = write(fd, &comms, sizeof(comms));
                if (size == -1)
                {
                    printf("\n\t[!] Error writing to backend fifo\n\n");
                    exit(EXIT_FAILURE);
                }
                int success = sell(itemName, category, basePrice, buyNowPrice, duration, usr->username); // Puts item to sell
                if (success == -1)
                    printf("\n\t[!] Error creating new item!\n");
                else
                    printf("\n\t[~] New item added successfully! ID: %d\n\n", success);
            }
            else
            {
                printf("\n\t[!] Invalid notation for command ' sell '\n");
                printf("\t[~] Use the following notation: 'sell <item-name> <category> <base-price> <buy-now-price> <duration>'\n\n");
                continue;
            }
        }
        else if (strcmp(cmd, "list") == 0)
        {
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            list(); // Lists available items
        }
        else if (strcmp(cmd, "licat") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n\t[!] Invalid notation for command ' licat '\n");
                printf("\t[~] Use the following notation: 'licat <category-name>'\n\n");
                continue;
            }
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            licat(arg); // Lists available per category
        }
        else if (strcmp(cmd, "lisel") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n\t[!] Invalid notation for command ' lisel '\n");
                printf("\t[~] Use the following notation: 'lisel <seller-username>'\n\n");
                continue;
            }
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            lisel(arg); // Lists specific seller items
        }
        else if (strcmp(cmd, "lival") == 0)
        {
            int value = atoi(arg);
            if (strcmp(arg, "") == 0 || value == 0)
            {
                printf("\n\t[!] Invalid notation for command ' lival '\n");
                printf("\t[~] Use the following notation: 'lival <max-price>'\n\n");
                continue;
            }
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            lival(value); // Lists items until a certain price range
        }
        else if (strcmp(cmd, "litime") == 0)
        {
            int duration = atoi(arg);
            if (strcmp(arg, "") == 0 || duration == 0)
            {
                printf("\n\t[!] Invalid notation for command ' litime '\n");
                printf("\t[~] Use the following notation: 'litime <hour-in-seconds>'\n\n");
                continue;
            }
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            litime(duration); // Lists available until a certain hour
        }
        else if (strcmp(cmd, "time") == 0)
        {
            currentTime(); // Displays current hour in seconds
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(cmd, "buy") == 0)
        {
            char idChar[10], valueChar[10];
            sscanf(arg, "%9s %9[^\n]", idChar, valueChar);
            int id = atoi(idChar), value = atoi(valueChar);
            if (strcmp(arg, "") == 0 || id == 0 || value == 0)
            {
                printf("\n\t[!] Invalid notation for command ' buy '\n");
                printf("\t[~] Use the following notation: 'buy <id> <value>'\n\n");
                continue;
            }
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            buy(id, value); // Lists active promotors
        }
        else if (strcmp(cmd, "cash") == 0)
        {
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            // Open frontend fifo
            sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, pid);
            int fd2 = open(FRONTEND_FINAL_FIFO, O_RDONLY);
            if (fd2 == -1)
            {
                printf("\n\t[!] Error opening frontend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            // Read from frontend fifo
            int size2 = read(fd2, &comms, sizeof(comms));
            if (size2 == -1)
            {
                printf("\n\t[!] Error reading from frontend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            printf("\n\t[~] Your current balance is: %d euros \n\n", comms.balance);
            close(fd2);
        }
        else if (strcmp(cmd, "add") == 0)
        {
            int value = atoi(arg);
            if (strcmp(arg, "") == 0 || value == 0)
            {
                printf("\n\t[!] Invalid notation for command ' add '\n");
                printf("\t[~] Use the following notation: 'add <value>'\n\n");
                continue;
            }
            strcpy(comms.message, cmd);
            // Store the value to be added to the user's balance
            comms.balance = value;
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            // Read from frontend fifo the new balance
            sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, pid);
            int fd2 = open(FRONTEND_FINAL_FIFO, O_RDONLY);
            if (fd2 == -1)
            {
                printf("\n\t[!] Error opening frontend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            int size2 = read(fd2, &comms, sizeof(comms));
            if (size2 == -1)
            {
                printf("\n\t[!] Error reading from frontend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            printf("\n\t[+] %d euros added to your balance\n", value);
            printf("\n\t[~] Your new balance is: %d euros \n\n", comms.balance);
            close(fd2);
        }
        else if (strcmp(cmd, "exit") == 0)
        {
            strcpy(comms.message, cmd);
            int size = write(fd, &comms, sizeof(comms));
            if (size == -1)
            {
                printf("\n\t[!] Error writing to backend fifo\n\n");
                exit(EXIT_FAILURE);
            }
            quit();
        }
        else if (strcmp(cmd, "clear") == 0)
        {
            clear();
        }
        else if (strcmp(cmd, "help") == 0)
        {
            printf("\n\t\t[!] Available commands:\n");
            printf("\t[~] sell <item-name> <category> <base-price> <buy-now-price> <duration> || Place item for sale\n");
            printf("\t[~] list || Lists available items\n");
            printf("\t[~] licat <category-name> || Lists items by category\n");
            printf("\t[~] lisel <seller-username> || Lists items by seller\n");
            printf("\t[~] lival <max-price> || Lists items by maximum price\n");
            printf("\t[~] litime <duration> || Lists items by limited time\n");
            printf("\t[~] time || Displays current hour in seconds\n");
            printf("\t[~] buy <id> <value> || Bid on item by specified id and value\n");
            printf("\t[~] cash || Displays users available balance\n");
            printf("\t[~] add <value> || Add specified value to users balance\n");
            printf("\t[~] exit || Exit user interface\n");
            printf("\t[~] clear || Console clear\n\n");
        }
        else
        {
            printf("\n\t[!] Command not found: '%s' (For help type: 'help')\n\n", cmd);
        }
    }
    pthread_exit((void *)NULL);
}
int backendOn()
{
    int backend_fd = open(BACKEND_FIFO, O_RDONLY | O_NONBLOCK);
    close(backend_fd);
    if (backend_fd == -1)
    {
        return 1;
    }
    return 0;
}
void receiveSignal(int n)
{
    printf("\n\n[!] Backend program is closing...\n");
    sleep(1);
    printf("\n[!] Exiting...\n\n");
    unlink(FRONTEND_FINAL_FIFO);
    sleep(1);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    User user;
    signal(SIGINT, quit);
    signal(SIGUSR1, receiveSignal);
    pthread_t threadBackendComms;
    pthread_t threadImAlive;

    if (backendOn())
    {
        printf("\n[!] Backend program isn't running yet!\n");
        return 0;
    }

    if (argc < 3 || argc >= 4)
    {
        printf("\n[!] Invalid number of arguments\n");
        printf("[~] Use the following notation: '$./frontend <username> <username-password>'\n\n");
        return 0;
    }

    sprintf(FRONTEND_FINAL_FIFO, FRONTEND_FIFO, getpid());
    if (mkfifo(FRONTEND_FINAL_FIFO, 0666) == -1)
    {
        printf("\n[!] Error creating frontend fifo!\n");
        return 0;
    }

    // Aqui envia para o backend as credenciais e retorna se são válidas ou não
    strcpy(user.username, argv[1]);
    strcpy(user.password, argv[2]);
    user.PID = getpid();

    int fd = open(BACKEND_FIFO, O_WRONLY);
    if (fd == -1)
    {
        printf("\n[!] Error opening frontend fifo\n");
        return 1;
    }
    int size = write(fd, &user, sizeof(user));
    if (size == -1)
    {
        printf("\n[!] Error writing to backend fifo\n");
        return 1;
    }
    close(fd);

    // Receives response from backend (1 = Logged in, -1 = Wrong password, 0 = User doesn't exist)
    int fd2 = open(FRONTEND_FINAL_FIFO, O_RDONLY);
    if (fd2 == -1)
    {
        printf("\n[!] Error opening backend fifo\n");
        return 1;
    }
    int success;
    read(fd2, &success, sizeof(int));
    sleep(2);
    close(fd2);

    if (success == 1)
    {
        printf("\n[!] Logged in successfully!\n");
        sleep(1);
    }
    else if (success == -1)
    {
        printf("\n[!] Wrong password!\n");
        sleep(1);
        quit2();
    }
    else if (success == 0)
    {
        printf("\n[!] User doesn't exist!\n");
        sleep(1);
        quit2();
    }
    else if (success == 2)
    {
        printf("\n[!] User already logged in!\n");
        sleep(1);
        quit2();
    }
    else if (success == -2)
    {
        printf("\n[!] Max number of users reached!\n");
        sleep(1);
        quit2();
    }

    if (pthread_create(&threadBackendComms, NULL, frontendCommandReader, &user) != 0)
    {
        perror("Error creating thread");
    }

    /*if (pthread_create(&threadImAlive, NULL, threadAlive, &user) != 0)
    {
        perror("Error creating thread to communicate its alive");
    }*/

    pthread_join(threadBackendComms, NULL);
    //pthread_join(threadImAlive, NULL);

    return 0;
}
