#include "promotor.h"

// Implement function that runs file text in order to execute promotion
// Runs in a thread so that multiple promotions can be ran at the same time

int main(int argc, char **argv)
{

    if (getenv("MAX_PROMOTORS") == NULL)
    {
        printf("\n[!] Error! MAX_PROMOTORS not defined!\n");
        return (0);
    }
    if (getenv("FPROMOTERS") == NULL)
    {
        printf("\n[!] Error! FPROMOTERS not defined!\n");
        return (0);
    }
    FILE *f;
    char *maxPromotorsChar = getenv("FPROMOTERS");
    int maxPromotors = atoi(maxPromotorsChar);

    char *promotorsFile = getenv("FPROMOTERS");

    Promotor *p = (Promotor *)malloc(sizeof(Promotor) * maxPromotors);

    f = fopen(promotorsFile, "r");
    if (f == NULL)
    {
        printf("\n[!] Error while opening the file ' promoters '\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n[~] Successuflly loaded file ' promoters '\n");
    }

    while (fscanf(f, "%s %d %d[^\n]", p->category, &p->discount, &p->duration) == 3)
    {
        printf("\n\n%s %d %d\n\n", p->category, p->discount, p->duration); // Just for testing purposes, remove later
        ++p;
    }

    int xpto, fd_pai_filho[2], size;
    scanf("%d", &xpto);
    pipe(fd_pai_filho);

    int pid = fork();
    if (pid == fork())
    {
        char str[100];
        close(fd_pai_filho[1]);
        size = read(fd_pai_filho[0], str, sizeof(str));
        printf("filho - pid %d leu %s com tamanho %d", getpid(), str, size);
    }
    else if (pid > 0)
    {
        close(fd_pai_filho[0]);
        scanf("%d", &xpto);

        size = write(fd_pai_filho[1], "ola", 4);
        printf("pai- pid %d, com tamanho %d\n", getpid(), size);
    }

    fclose(f);
}

/* EXEMPLO DE COMO USAR O SINAIS

static int r = 20;

void sig_handler(int signal) {
    if (signal != SIGALRM)
        return;

    printf("\nPassaram %i segundos.", r);
    alarm(r--);
}

int main (void) {
    char a = 0;

    if (signal(SIGALRM, sig_handler) == SIG_ERR)
        printf("\nErro ao colocar a função ao serviço do sinal.");
    
    printf("\nIndique o tempo de espera: ");
    fflush(stdout);
    scanf("%i", &r);

    alarm(r--);

    do {
        printf("\nVou aguardar aqui! Pode pressionar uma tecla para acelerar o timer.");
        scanf("%c", &a);

        printf("\nVou chamar o timer!");
        alarm(n--);//sig_handler(SIGALRM);
    } while(1);

    return 0;
}*/