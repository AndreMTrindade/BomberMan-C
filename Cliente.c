#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ncurses.h>

#include "Estruturas.h"


#define clear printf("\033[H\033[J")

typedef struct Jogada{
    char ascii;
    int PID; 
}Jogada;

Cliente* Inicio(Cliente *c);
void LimpaStdin(void);
int EnviaDadosLogin(Cliente *c);
Objecto* RecebeObjetosIniciais();
void MostraLabirinto(Objecto *ob);
void gotoxy(int x, int y);

int main(int argc, char** argv) {
    Cliente c;
    char str[50];
    int fd;
    pthread_t recebe;
    int Sair = 0;
    int envia = 0;
    char tecla;
    Objecto *ob;
    Jogada j;
    
    sprintf(str, "../JJJ%d", getpid());
    mkfifo(str, 0600);
    do {
        clear;
        Inicio(&c);
    } while (EnviaDadosLogin(&c) == -1);

    ob = RecebeObjetosIniciais();
    //MostraLabirinto(ob);
    //pthread_create(&recebe, NULL, &RecebeObjetos, (void*) &Sair);

    initscr();
    while (1) {
        envia = 0;
        tecla = getch();
        if (toupper(tecla) == 'W' || tecla == 30) {
            envia = 1;
        } else {
            if (toupper(tecla) == 'S' || tecla == 31) {
                envia = 1;
            } else {
                if (toupper(tecla) == 'D' || tecla == 16) {
                    envia = 1;
                } else {
                    if (toupper(tecla) == 'A' || tecla == 17) {
                        envia = 1;
                    } else {
                        if (toupper(tecla) == 'A' || tecla == 17) {
                            envia = 1;
                        }
                    }
                }
            }
        }
        
        if(envia == 1)
        {
            fd = open("../MMM",O_WRONLY);
            if(fd == -1)
            {
                printf("Erro ao Abrir FIFO \n");
                fflush(stdout);
            }
            else
            {
                j.PID = getpid();
                j.ascii = (int)tecla;
                write(fd,&j,sizeof(j));
                close(fd);
            }
            
        }


    }
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

    sprintf(str, "../JJJ%d", c->PID);

    write(fd, c, sizeof (Cliente));
    close(fd);
    printf("Esperando Resposta do Servidor...\n");
    fflush(stdout);


    fdres = open(str, O_RDONLY);

    if (fdres == -1) {
        printf("Erro ao criar fifo JJJ\n");
    }

    read(fdres, &res, sizeof (res));
    fflush(stdout);
    close(fdres);


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

///THREAD  QUE RECEBE OS OBJETOS DO SERVIDOR

Objecto* RecebeObjetosIniciais() {
    int fd, i;
    char str[50];
    Objecto *arrayb = NULL;
    Objecto *ul;
    Objecto b;

    sprintf(str, "../JJJ%d", getpid());

    fd = open(str, O_RDONLY);
    if (fd == -1) {
        printf("<ERRO> Erro ao abrir o Ficheiro FIFO\n");
        fflush(stdout);
        pthread_exit(0);
    }

    while (1) {
        i = read(fd, &b, sizeof (b));
        printf("Recebeu: %d\n", b.id);
        fflush(stdout);
        if (i == sizeof (b)) {

            if (b.id == -1) {
                break;
                break;
            }
            if (arrayb == NULL) {
                arrayb = (Objecto*) malloc(sizeof (b));
                arrayb->ativo = b.ativo;
                arrayb->id = b.id;
                arrayb->tipo = b.tipo;
                arrayb->x = b.x;
                arrayb->y = b.y;
                arrayb->p = NULL;
                ul = arrayb;
            } else {
                ul->p = (Objecto*) malloc(sizeof (b));
                ul->p->ativo = b.ativo;
                ul->p->id = b.id;
                ul->p->tipo = b.tipo;
                ul->p->x = b.x;
                ul->p->y = b.y;
                ul->p->p = NULL;
                ul = ul->p;
            }
        }
    }
    close(fd);
    printf("ACABOU");
    return arrayb;
    return arrayb;
}

void MostraLabirinto(Objecto *ob) {
    Objecto *it;
    it = ob;

    while (it != NULL) {
        gotoxy(it->x, it->y);
        printf("%d", it->tipo);
        it = it->p;
    }
}

////GOTOXY

void gotoxy(int x, int y) {
    printf("%c[%d;%df", 0x1B, y, x);
}