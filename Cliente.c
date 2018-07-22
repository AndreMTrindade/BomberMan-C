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
    WINDOW * mainwin;
} PassaThread;


Cliente* Inicio(Cliente *c);
void LimpaStdin(void);
int EnviaDadosLogin(Cliente *c);
Objecto* RecebeObjetosIniciais();
void MostraLabirinto(Objecto *ob);
void gotoxy(int x, int y);
void* AtualizaEcra(void *dados);
void Imprime(Objecto *ob);

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

    WINDOW * mainwin;

    do {
        clear;
        Inicio(&c);
    } while (EnviaDadosLogin(&c) == -1);

    ob = RecebeObjetosIniciais();

    passadadosThread->Sair = &Sair;
    passadadosThread->ob = ob;
    passadadosThread->mainwin = mainwin;

    if ((mainwin = initscr()) == NULL) {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(EXIT_FAILURE);
    }

    noecho();
    keypad(mainwin, TRUE);
    curs_set(0);
    start_color();
    Imprime(ob);

    pthread_create(&envia, NULL, &AtualizaEcra, (void *) passadadosThread);

    do {
        scanf("%c", &tecla);
        fde = open("../MMM", O_WRONLY);
        if (fde == -1) {
            printf("Erro ao Abrir FIFO \n");
            fflush(stdout);
        } else {
            j.PID = getpid() + 10000;
            j.ascii = (int) tecla;
            write(fde, &j, sizeof (j));
            close(fde);
        }

    } while (Sair == 0);

    delwin(mainwin);
    endwin();
    refresh();
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
        printf("Recebeu: %d\n", b.id);
        fflush(stdout);
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

void* AtualizaEcra(void *dados) {
    PassaThread *x = (PassaThread*) dados;
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
                endwin();
                refresh();
                printf("O seu jogador foi kicado!\n");
                sleep(2);
                pthread_exit(0);
            } else {
                if (b.tipo == -1) {
                    clear();
                    delwin(x->mainwin);
                    endwin();
                    refresh();
                    printf("O Servidor foi terminado!\n");
                    fflush(stdout);
                    sleep(2);
                    pthread_exit(0);
                } else {
                    if (b.tipo == -2) {
                        clear();
                        delwin(x->mainwin);
                        endwin();
                        refresh();
                        printf("Perdeu!\n");
                        fflush(stdout);
                        sleep(2);
                        pthread_exit(0);
                    }
                }

            }
            existe = 0;
            while (it != NULL) {
                if (it->id == b.id) {
                    existe = 1;
                    if (b.ativo == 0) {
                        if (ant == NULL) {
                            x->ob = x->ob->p;
                        } else {
                            ant->p = it->p;
                            free(it);

                        }
                    } else {
                        it->x = b.x;
                        it->y = b.y;
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
            }
            Imprime(x->ob);
        }
    }
    pthread_exit(0);
}

///IMPRIME NO ECRA OS OBJECTOSd

void Imprime(Objecto *ob) {
    int contador = 0;
    Objecto *it;
    it = ob;
    clear();

    init_pair(1, COLOR_GREEN, COLOR_GREEN);
    init_pair(2, COLOR_WHITE, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_RED);
    init_pair(5, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(6, COLOR_BLUE, COLOR_BLUE);
    init_pair(7, COLOR_RED, COLOR_BLACK);
    init_pair(8, COLOR_BLACK, COLOR_YELLOW);

    init_pair(9, COLOR_WHITE, COLOR_BLACK);
    init_pair(10, COLOR_YELLOW, COLOR_BLACK);
    init_pair(11, COLOR_RED, COLOR_BLACK);
    init_pair(12, COLOR_CYAN, COLOR_BLACK);
    init_pair(13, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(14, COLOR_BLUE, COLOR_BLACK);
    init_pair(15, COLOR_WHITE, COLOR_BLACK);

    while (it != NULL) {
        if (it->tipo == 1) {
            attron(COLOR_PAIR(1));
            mvprintw(it->y, it->x, "0");
        } else {
            if (it->tipo > 1000) {
                contador++;
                switch (contador) {
                    case 1: attron(COLOR_PAIR(9));
                        break;
                    case 2: attron(COLOR_PAIR(10));
                        break;
                    case 3: attron(COLOR_PAIR(11));
                        break;
                    case 4: attron(COLOR_PAIR(12));
                        break;
                    case 5: attron(COLOR_PAIR(13));
                        break;
                    case 6: attron(COLOR_PAIR(14));
                        break;

                }
                mvprintw(it->y, it->x, "P");
            } else if (it->tipo == 2) {
                attron(COLOR_PAIR(2));
                mvprintw(it->y, it->x, "O");
            } else if (it->tipo == 3) {
                attron(COLOR_PAIR(3));
                mvprintw(it->y, it->x, "x");
            } else if (it->tipo == 4) {
                attron(COLOR_PAIR(3));
                mvprintw(it->y, it->x, "X");
            } else if (it->tipo == 5) {
                attron(COLOR_PAIR(5));
                mvprintw(it->y, it->x, "i");
            } else if (it->tipo == 6) {
                attron(COLOR_PAIR(6));
                mvprintw(it->y, it->x, "U");
            } else if (it->tipo == 7) {
                attron(COLOR_PAIR(7));
                mvprintw(it->y, it->x, "I");
            } else if (it->tipo == 8) {
                attron(COLOR_PAIR(8));
                mvprintw(it->y, it->x, "x");
            } else if (it->tipo == 9) {
                attron(COLOR_PAIR(8));
                mvprintw(it->y, it->x, "X");
            } else if (it->tipo == 10) {
                attron(COLOR_PAIR(8));
                mvprintw(it->y, it->x, "C");
            } else if (it->tipo == 11) {
                attron(COLOR_PAIR(8));
                mvprintw(it->y, it->x, "E");
            } else if (it->tipo == 12) {
                if (it->y == getpid()) {
                    attron(COLOR_PAIR(15));
                    mvprintw(2, 40, "Ponto: %d", it->x);
                }
            }
        }
        it = it->p;
    }
    refresh();

}