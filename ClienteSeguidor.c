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

typedef struct {
    char ascii;
    int PID;
} Jogada;

typedef struct {
    Objecto *ob;
    int *Sair;
} PassaThread;


void LimpaStdin(void);
int EnviaDadosLogin(Cliente *c);
Objecto* RecebeObjetosIniciais();
void MostraLabirinto(Objecto *ob);
void gotoxy(int x, int y);
void AtualizaEcra(PassaThread *x);

int main(int argc, char** argv) {
    Cliente c;
    char str[50];
    int fd, fde;
    int Sair = 0;
    char tecla;
    Objecto *ob;
    Jogada j;
    PassaThread *passadadosThread = (PassaThread*) malloc(sizeof (PassaThread));
    pthread_t envia;

    c.Ajogar = 3;
    c.PID = getpid();


    EnviaDadosLogin(&c);

    ob = RecebeObjetosIniciais();
    printf("Jogo Comecou: \n\n");

    passadadosThread->Sair = &Sair;
    passadadosThread->ob = ob;


    AtualizaEcra(passadadosThread);
    return (EXIT_SUCCESS);
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
    mkfifo(str, 0600);
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
        if (i == sizeof (b)) {
            if (b.id == -1) {
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
    return arrayb;
}

////GOTOXY

void gotoxy(int x, int y) {
    printf("%c[%d;%df", 0x1B, y, x);
}

///RECEBE OS DADOS

void AtualizaEcra(PassaThread *x) {
    char str[50];
    int fd, i, existe = 0;
    Objecto *it, *temp, *ant = NULL;
    Objecto b;

    sprintf(str, "../JJJ%d", getpid());

    fd = open(str, O_RDWR);
    if (fd == -1) {
        printf("<ERRO> Erro ao abrir o Ficheiro FIFO\n");
        fflush(stdout);
        pthread_exit(0);
    }

    while (*(x->Sair) == 0) {
        i = read(fd, &b, sizeof (b));
        if (i == sizeof (b)) {

            it = x->ob;
            if (b.id == -5) {
                printf("O seu jogador foi kicado!\n");
                sleep(2);
                return;
            } else {
                if (b.tipo == -1) {
                    endwin();
                    refresh();
                    printf("O Servidor foi terminado!\n");
                    sleep(2);
                    pthread_exit(0);
                }
            }
            existe = 0;
            while (it != NULL) {
                if (it->id == b.id) {
                    existe = 1;
                    if (b.ativo == 0) {
                        if (b.tipo == 3 || b.tipo == 4)
                            printf("Bomba Explodiu\n");
                        if (ant == NULL) {
                            x->ob = x->ob->p;
                        } else {
                            ant->p = it->p;
                            free(it);

                        }
                    } else {
                        it->x = b.x;
                        it->y = b.y;
                        printf("Jogador Moveu X: %d Y: %d\n", b.x, b.y);
                        fflush(stdout);
                    }
                    break;
                }
                ant = it;
                it = it->p;
            }
            if (existe == 0) {
                it = x->ob;
                while (it->p != NULL) {
                    it = it->p;
                }
                temp = (Objecto*) malloc(sizeof (Objecto));
                it->p = temp;
                temp->id = b.id;
                temp->ativo = b.ativo;
                temp->tipo = b.tipo;
                temp->x = b.x;
                temp->y = b.y;
                temp->p = NULL;
                if (temp->tipo == 3 || temp->tipo == 4) {
                    printf("Nova Boma Y: %d X: %d\n", temp->x, temp->y);
                }

                fflush(stdout);
            }
        }
    }

}