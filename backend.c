#include "backend.h"

void* backendCommandReader(){
    int pid = getpid();
    char command[50], cmd[20], arg[20];

    while(1){
        printf("[%5d] COMMAND: ", pid);
        scanf("%49[^\n]", command);
	    sscanf(command, "%14s %29[^\n]", cmd, arg);

        if(strcmp(cmd, "users") == 0){
            //users(); //Lists users using the platform ATM
        }
        else if(strcmp(cmd, "list") == 0){
            //list(); //Lists available items
        }
        else if(strcmp(cmd, "kick") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' kick '\n");
                printf("Use the following notation: 'kick <username>'\n");
                continue;
            }
            //kick(arg); // Kicks certain user by the username stored in "arg"
        }
        else if(strcmp(cmd, "prom") == 0){
            //prom(); //Lists active promotors
        }
        else if(strcmp(cmd, "reprom") == 0){
            //reprom(); //Refresh active promotors
            //Reads the promotors file again, removes no longer present promotors and adds new ones
        }
        else if(strcmp(cmd, "cancel") == 0){
            if(strcmp(arg, "") == 0){
                printf("\nInvalid notation for command ' cancel '\n");
                printf("Use the following notation: 'cancel <promotors-executable-filename>'\n");
                continue;
            }
            //cancelPromotor(); Cancels promotor
        }
        else if(strcmp(cmd, "close") == 0){
            printf("\nClosing...\n");
            sleep(2);
            //return(0);
            //Just for basic testing, needs to assure closing of possible open pipes and threads 
        }
    }
}

int main(int argc, char **argv){
    char backend_fifo[20];
    pthread_t id;
	int pid = getpid();

    if(access(BACKEND_FIFO, F_OK) != 0){
		if(mkfifo(BACKEND_FIFO, 0777) == -1){
			perror("\nNamed pipe already exists!\n");
			return -1;
		}
	}
	fprintf(stderr, "Backend FIFO created!\n");

    //In order for this to work don't forget to compile varAmbiente.sh first! By doing: "source varAmbiente.sh in terminal"
    if(getenv("MAX_USERS") == NULL){
        printf("\nError! MAX_USERS not defined!\n");
        return(0);
    }
    if(getenv("MAX_PROMOTORS") == NULL){
        printf("\nError! MAX_PROMOTORS not defined!\n");
        return(0);
    }
    if(getenv("MAX_ITEMS") == NULL){
        printf("\nError! MAX_ITEMS not defined!\n");
        return(0);
    }

    char* maxUsersChar = getenv("MAX_USERS");
    int maxUsers = atoi(maxUsersChar);

    char* maxPromotorsChar = getenv("MAX_PROMOTORS");
    int maxPromotors = atoi(maxPromotorsChar);

    char* maxItemsChar = getenv("MAX_ITEMS");
    int maxItems = atoi(maxItemsChar);

    backendCommandReader();
}