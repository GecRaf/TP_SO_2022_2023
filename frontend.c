#include "frontend.h"


void frontendCommandReader(){
    int pid = getpid();
    char command[50], cmd[20], arg[20];

    while(1){
        printf("[%5d] COMMAND: ", pid);
        scanf("%49[^\n]", command);
	    sscanf(command, "%14s %29[^\n]", cmd, arg);

        if(strcmp(cmd, "sell") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' sell '\n");
                printf("Use the following notation: 'sell <item-name> <category> <base-price> <buy-now-price> <duration>'\n");
                continue;
            }
            //sell(arg); //Puts item to sell
            //Should return the ID from the platform or -1 in case of insuccess
        }
        else if(strcmp(cmd, "list") == 0){
            //list(); //Lists available items
        }
        else if(strcmp(cmd, "licat") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' licat '\n");
                printf("Use the following notation: 'licat <category-name>'\n");
                continue;
            }
            //licat(); //Lists available per category
        }
        else if(strcmp(cmd, "lisel") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' lisel '\n");
                printf("Use the following notation: 'lisel <seller-username>'\n");
                continue;
            }
            //lisel(); //Lists specific seller items
        }
        else if(strcmp(cmd, "lival") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' lival '\n");
                printf("Use the following notation: 'lival <max-price>'\n");
                continue;
            }
            //lival(); //Lists items until a certain price range
        }
        else if(strcmp(cmd, "litime") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' litime '\n");
                printf("Use the following notation: 'litime <hour-in-seconds>'\n");
                continue;
            }
            //litime(); //Lists available until a certain hour
        }
        else if(strcmp(cmd, "time") == 0){
            //time(); // Displays current hour in seconds
        }
        else if(strcmp(cmd, "buy") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' buy '\n");
                printf("Use the following notation: 'buy <id> <value>'\n");
                continue;
            }
            //prom(); //Lists active promotors
        }
        else if(strcmp(cmd, "cash") == 0){
            //cash(); //Displays user's available balance
        }
        else if(strcmp(cmd, "add") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' add '\n");
                printf("Use the following notation: 'add <value>'\n");
                continue;
            }
            //add(); //Adds certain value to user's balance
        }
        else if(strcmp(cmd, "exit") == 0){
            printf("\nExiting...\n");
            sleep(2);
            //exit();
            //Just for basic testing, needs to assure closing of possible open pipes and threads 
            //Needs to warn "backend" that is closing (signals)
        }
    }
}


int main(int argc, char **argv){

    if(argc < 3 || argc >= 4){
        printf("\nInvalid number of arguments\n");
        printf("Use the following notation: '$./frontend <username> <username-password>'\n\n");
        return 0;
    }
    //Aqui envia para o backend as credenciais e retorna se são válidas ou não
    frontendCommandReader();
}
