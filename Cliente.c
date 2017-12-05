#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "Estruturas.h"

#define clear printf("\033[H\033[J")

Cliente* Inicio(Cliente *c);
void LimpaStdin(void);
int EnviaDadosLogin(Cliente *c);

int main(int argc, char** argv) {
    Cliente c;
    char str[50];
    int fd;
    Objecto ObInicial;
    
    do
    {
        clear;
        Inicio(&c);
    }while(EnviaDadosLogin(&c) == -1);
    
    printf("Esperando por outros Jogadores..");
    fflush(stdout);
        
    sprintf(str, "../JJJ%d", c.PID);
    mkfifo(str, 0600);
    fd = open(str, O_RDONLY);
    read(fd, &ObInicial, sizeof (ObInicial));
    
    
    return (EXIT_SUCCESS);
}

///FUNÇÃO ONDE OCORRE O LOGIN
/// DO CLIENTE
Cliente* Inicio(Cliente *c) {
    char Nome[50];
    char Pass[50];

    do {
        printf("Nome: ");
        scanf("%[^\n]", Nome);
        LimpaStdin();
    } while (strlen(Nome) <= 0);


    do {
        printf("Palavra-Chave: ");
        scanf("%[^\n]", Pass);
        LimpaStdin();
    } while (strlen(Pass) < 4);

    strcpy(c->nome, Nome);
    strcpy(c->PalavraChave, Pass);
    c->PID = getpid();

    return c;
}

////FUNÇÃO PARA APANHAR OS ESPAÇOS EM BRANCO
void LimpaStdin(void) {
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

///// FUNÇÃO RESPONSAVEL POR ENVIAR OS DADOS
////  PARA O SERVIDOR 
int EnviaDadosLogin(Cliente *c) {
    char str[10];
    int fdres;
    int res;
    int fd = open(FIFOLOGIN, O_WRONLY);
    
    write(fd, c, sizeof (Cliente));

    printf("Esperando Resposta do Servidor...\n");
    fflush(stdout);

    sprintf(str, "../JJJ%d", c->PID);
    mkfifo(str, 0600);
    fdres = open(str, O_RDONLY);

    if (fdres == -1) {
        printf("Erro ao criar fifo JJJ\n");
    }

    read(fdres, &res, sizeof (res));
    fflush(stdout);
    close(fdres);
    unlink(str);
    close(fd);

    if (res == 0) {
        printf("%s Jogador já está online!\n", c->nome);
        sleep(3);
        return -1;
    } else {
        if (res == 1) {
            printf("Login efectuado com sucesso!\n");      
            return 1;
        } else {
            printf("Jogador inexistente!\n");
            sleep(3);
            return -1;
        }
    }
}