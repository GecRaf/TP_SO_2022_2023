#include "frontend.h"

Item* items;
int continua = 1;

int sell(char itemName[], char category[], int basePrice, int buyNowPrice, int duration)
{
    printf("\nitemName: %s\ncategory: %s\nbasePrice: %d\nbuyNowPrice: %d\nduration: %d\n\n",
           itemName, category, basePrice, buyNowPrice, duration);

    int i = 0;
    while (items[i].id != 0)
        i++;

    items[i].id = i + 1;

    if(items[i].id != 0){
        strcpy(items[i].name, itemName);
        strcpy(items[i].category, category);
        items[i].basePrice = basePrice;
        items[i].buyNowPrice = buyNowPrice;
        
        return items[i].id; // In case of success
    }
    else
        return -1; // In case of insuccess
    // TODO later change void to int function
}
void list() {}
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
void currentTime() {}
void buy(int id, int value) {
    printf("\nid: %d\nvalue: %d\n\n", id, value);
}
void userBalance(){}
void add(int value)
{
    printf("\nvalue: %d\n\n", value);
}
void quit() {
    printf("\n[!] Exiting...\n\n");
    sleep(1);
    exit(EXIT_SUCCESS);
}
void clear()
{
    system("clear");
}

void frontendCommandReader()
{
   // clear();
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
            if(sscanf(arg, "%14s %14s %9s %9s %9[^\n]", itemName, category, basePriceChar, buyNowPriceChar, durationChar)==5){
                int basePrice = atoi(basePriceChar), buyNowPrice = atoi(buyNowPriceChar), duration = atoi(durationChar);
            if(strcmp(itemName, "") == 0)
            {
                printf("\nInvalid notation for command ' sell ' in <item-name>\n");
                continue;
            }
            else if(strcmp(category, "") == 0)
            {
                printf("\nInvalid notation for command ' sell ' in <category>\n");
                continue;
            }
            else if(basePrice == 0 ){
                 printf("\nInvalid notation for command ' sell '! Base Price must be > 0 \n");
                continue;
            }
            else if(buyNowPrice == 0)
            {
                printf("\nInvalid notation for command ' sell '! Buy Now Price must be > 0 \n");
                continue;
            }
            else if(duration == 0)
            {
                printf("\nInvalid notation for command ' sell '! Duration must be > 0 \n");
                continue;
            }
            int success = sell(itemName, category, basePrice, buyNowPrice, duration); // Puts item to sell
            if(success == -1)
                printf("\n[!] Error creating new item!\n");
            else
                printf("\n[~] New item added successfully!\nID: %d\n\n", success);
            // Should return the ID from the platform or -1 in case of insuccess
            }
            
            else 
            {
                printf("\nInvalid notation for command ' sell '\n");
                printf("Use the following notation: 'sell <item-name> <category> <base-price> <buy-now-price> <duration>'\n");
                continue;
            }
            
        }
        else if (strcmp(cmd, "list") == 0)
        {
            list(); // Lists available items
        }
        else if (strcmp(cmd, "licat") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n[!] Invalid notation for command ' licat '\n");
                printf("[~] Use the following notation: 'licat <category-name>'\n");
                continue;
            }
            licat(arg); // Lists available per category
        }
        else if (strcmp(cmd, "lisel") == 0)
        {
            if (strcmp(arg, "") == 0)
            {
                printf("\n[!] Invalid notation for command ' lisel '\n");
                printf("[~] Use the following notation: 'lisel <seller-username>'\n");
                continue;
            }
            lisel(arg); // Lists specific seller items
        }
        else if (strcmp(cmd, "lival") == 0)
        {
            int value = atoi(arg);
            if (strcmp(arg, "") == 0 || value == 0)
            {
                printf("\n[!] Invalid notation for command ' lival '\n");
                printf("[~] Use the following notation: 'lival <max-price>'\n");
                continue;
            }
            lival(value); // Lists items until a certain price range
        }
        else if (strcmp(cmd, "litime") == 0)
        {
            int duration = atoi(arg);
            if (strcmp(arg, "") == 0 || duration == 0)
            {
                printf("\n[!] Invalid notation for command ' litime '\n");
                printf("[~] Use the following notation: 'litime <hour-in-seconds>'\n");
                continue;
            }
            litime(duration); // Lists available until a certain hour
        }
        else if (strcmp(cmd, "time") == 0)
        {
            currentTime(); // Displays current hour in seconds
        }
        else if (strcmp(cmd, "buy") == 0)
        {
            char idChar[10], valueChar[10];
            sscanf(arg, "%9s %9[^\n]", idChar, valueChar);
            int id = atoi(idChar), value = atoi(valueChar);
            if (strcmp(arg, "") == 0 || id == 0 || value == 0)
            {
                printf("\n[!] Invalid notation for command ' buy '\n");
                printf("[~] Use the following notation: 'buy <id> <value>'\n");
                continue;
            }
            buy(id, value); // Lists active promotors
        }
        else if (strcmp(cmd, "cash") == 0)
        {
            userBalance(); // Displays user's available balance
        }
        else if (strcmp(cmd, "add") == 0)
        {
            int value = atoi(arg);
            if (strcmp(arg, "") == 0 || value == 0)
            {
                printf("\n[!] Invalid notation for command ' add '\n");
                printf("[~] Use the following notation: 'add <value>'\n");
                continue;
            }
            add(value); // Adds certain value to user's balance
        }
        else if (strcmp(cmd, "exit") == 0)
        {
            quit();
            // Just for basic testing, needs to assure closing of possible open pipes and threads
            // Needs to warn "backend" that is closing (signals)
        }
        else if (strcmp(cmd, "clear") == 0)
        {
            clear();
        }
        else if (strcmp(cmd, "help") == 0)
        {
            printf("\t\n[!] Available commands:\n");
            printf("[~] sell <item-name> <category> <base-price> <buy-now-price> <duration> || Place item for sale\n");
            printf("[~] list || Lists available items\n");
            printf("[~] licat <category-name> || Lists items by category\n");
            printf("[~] lisel <seller-username> || Lists items by seller\n");
            printf("[~] lival <max-price> || Lists items by maximum price\n");
            printf("[~] litime <duration> || Lists items by limited time\n");
            printf("[~] time || Displays current hour in seconds\n");
            printf("[~] buy <id> <value> || Bid on item by specified id and value\n");
            printf("[~] cash || Displays users available balance\n");
            printf("[~] add <value> || Add specified value to users balance\n");
            printf("[~] exit || Exit user interface\n");
            printf("[~] clear || Console clear\n\n");
        }
        else
        {
            printf("\t[!] Command not found: '%s' (For help type: 'help')\n\n", cmd);
        }
    }
} 

int sendCredentials(char username[], char password[])
{
    // Named pipes
}

void finish(int s){
    continua = 0;
}

int main(int argc, char **argv)
{
    int fd;
    char username[20], password[20];

    struct sigaction act;
    act.sa_handler = finish;
    sigaction(SIGINT, &act, NULL);
    
    if (argc < 3 || argc >= 4)
    {
        printf("\n[!] Invalid number of arguments\n");
        printf("[~] Use the following notation: '$./frontend <username> <username-password>'\n\n");
        return 0;
    }

    strcpy(username, argv[1]);
    strcpy(password, argv[2]);

    int success = sendCredentials(username, password);
    if(success == -1)
        return 0; //TODO later

    if (getenv("MAX_ITEMS") == NULL)
    {
        printf("\n[!] Error! MAX_ITEMS not defined!\n");
        return (0);
    }

    char *maxItemsChar = getenv("MAX_ITEMS");
    int maxItems = atoi(maxItemsChar);
    
    items = (Item*)malloc(sizeof(Item)*maxItems);
    // Aqui envia para o backend as credenciais e retorna se são válidas ou não
    frontendCommandReader();
}
