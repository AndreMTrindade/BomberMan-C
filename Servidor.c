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

typedef struct Palava {
    char comando[50];
    struct Palavra *p;
} Palavra;

typedef struct {
    Cliente *clientes;
    Objecto *objectos;
    int* Sair;
} PassarThread;

typedef struct {
    Cliente *clientes;
    Objecto *objectos;
    int* Sair;
} PassarThreadJogo;

typedef struct Jogada {
    char ascii;
    int PID;
} Jogada;


static int id = 0;

Cliente* Shell(Cliente *clientes);
Palavra* DevolvePalavras(char* frase);
void LimpaStdin(void);
int ProcessaComando(Palavra *p);
int Size(Palavra *p);
char* UpString(char *s);
Cliente* AdicionaCliente(Palavra *p, Cliente *c);
void Users(Cliente *c);
void *RecebeJogadores(void *dados);
void *EnviaDadosJagador(void *dados);
int VerificaCliente(Cliente *cl, Cliente c);
Cliente *LeClientes();
void GravaClientes(Cliente *clientes);
int Conta(Cliente *c);
Objecto* LeLabirinto();
void gotoxy(int x, int y);
void MostraLabirinto(Objecto *ob);
void *CorpoJogo(void *dados);
void TrataAccao(Objecto *b, Jogada j);
Objecto VerificaMovimento(int tecla, Objecto *lob, Objecto *ob);

int main(int argc, char** argv) {
    Cliente *clientes = LeClientes();
    if (access(FIFOLOGIN, F_OK) == 0) {
        printf("Já está a ser executado um Servidor!..\n");
        return 1;
    }

    clientes = Shell(clientes);
    GravaClientes(clientes);

    return (EXIT_SUCCESS);
}

////FUNÇÃO QUE LE OS COMANDOS INSERIDOS

Cliente* Shell(Cliente *clientes) {
    char comando[50];
    Palavra *it;
    Palavra *p;
    int op;
    int Sair = 0;
    pthread_t recebe;
    Objecto *ob = NULL;
    PassarThread *x = (PassarThread*) malloc(sizeof (PassarThread));
    ob = LeLabirinto();
    x->clientes = clientes;
    x->Sair = &Sair;
    x->objectos = ob;
    pthread_create(&recebe, NULL, &RecebeJogadores, (void *) x);

    while (1) {
        printf("Comando: ");
        scanf("%[^\n]", comando);
        LimpaStdin();

        p = DevolvePalavras(comando);
        op = ProcessaComando(p);

        switch (op) {
            case -2:
                break;
            case -1:
                printf("<ERRO> Comando incorreto\n");
                break;
            case 0:
                clientes = AdicionaCliente(p, clientes);
                break;
            case 1:
                Users(clientes);
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                break;

        }

        it = p;
        while (p != NULL) {
            it = (Palavra*) it->p;
            free(p);
            p = it;
        }
        if (op == -2) {
            Sair = 1;
            unlink(FIFOLOGIN);
            return clientes;
        }
    }
}

///DEVOLVE UMA ARRAY DE PALAVRAS
///// DIVIDE EM PALAVRAS A FRASE INSERIDA

Palavra* DevolvePalavras(char* frase) {
    Palavra *p = NULL;
    Palavra *novo = NULL;
    Palavra *ultimo;
    char str[100];
    strcpy(str, "");
    for (int i = 0; i <= (int) strlen(frase); i++) {
        if (frase[i] != ' ' && frase[i] != '\t' && frase[i] != '\0') {
            //strcat(str,frase[i]);
            sprintf(str, "%s%c", str, frase[i]);
        } else {
            if (strlen(str) != 0) {
                if (p == NULL) {
                    p = (Palavra*) malloc(sizeof (Palavra));
                    strcpy(p->comando, str);
                    p->p = NULL;
                    ultimo = (Palavra*) p;
                } else {
                    novo = (Palavra*) malloc(sizeof (Palavra));
                    strcpy(novo->comando, str);
                    ultimo->p = (struct Palavra*) novo;
                    novo->p = NULL;
                    ultimo = (Palavra*) novo;
                }
            }
            strcpy(str, "");
        }
    }

    return p;
}

////FUNÇÃO PARA APANHAR OS ESPAÇOS EM BRANCO

void LimpaStdin(void) {
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

///FUNÇÃO PARA DEVOLVER O NUMERO CORRESPONDENTE AO COMANDO INSERIDO

int ProcessaComando(Palavra *p) {
    if (p == NULL) {
        printf("aa");
        return -1;
    } else {
        if (strcmp(UpString(p->comando), "EXIT") == 0) {
            return -2;
        } else {
            if (strcmp(UpString(p->comando), "ADD") == 0) {
                if (Size(p) == 3) {
                    return 0;
                }
            } else {
                if (strcmp(UpString(p->comando), "USERS") == 0) {
                    return 1;

                } else {
                    if (strcmp(UpString(p->comando), "KICK") == 0) {
                        if (Size(p) == 2) {
                            return 2;
                        }
                    } else {
                        if (strcmp(UpString(p->comando), "GAME") == 0) {
                            if (Size(p) == 1) {
                                return 3;
                            }
                        } else {
                            if (strcmp(UpString(p->comando), "SHUTDOWN") == 0) {
                                if (Size(p) == 1) {
                                    return 4;
                                }
                            } else {
                                if (strcmp(UpString(p->comando), "MAP") == 0) {
                                    if (Size(p) == 2) {
                                        return 5;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return -1;
}

////TAMANHO DE IMA PALAVRA

int Size(Palavra *p) {
    int contar = 0;
    Palavra *it = p;
    while (it != NULL) {
        contar++;
        it = (Palavra*) it->p;
    }

    return contar;
}

///TRANSFORMA A STRING
//// COLOCA TODOS OS CARACTERES EM MAIUSCULA

char* UpString(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        s[i] = toupper(s[i]);
        i++;
    }
    return s;
}

///FUNÇÃO PARA ADICIONAR CLIENTES

Cliente* AdicionaCliente(Palavra *p, Cliente *c) {

    Palavra *it;
    Palavra *Username, *password;
    pthread_t recebe;

    int i = 0;

    it = p;

    while (it != NULL) {

        if (i == 1) {
            Username = it;
        }
        if (i == 2) {
            password = it;
        }
        i++;
        it = (Palavra*) it->p;
    }

    if (c == NULL) {

        c = (Cliente*) malloc(sizeof (Cliente));

        strcpy(c->nome, Username->comando);
        strcpy(c->PalavraChave, password->comando);
        c->PID = 0;
        c->Ajogar = 0;
        c->p = NULL;
        printf("Primeiro Utilizador Adicionado com Sucesso!\n");

        return c;
    } else {
        Cliente *it;
        Cliente *novo;
        it = c;
        while (it->p != NULL) {
            it = (Cliente*) it->p;
        }
        novo = (Cliente*) malloc(sizeof (Cliente));
        strcpy(novo->nome, Username->comando);
        strcpy(novo->PalavraChave, password->comando);
        novo->PID = 0;
        novo->Ajogar = 0;
        novo->p = NULL;
        it->p = novo;
        printf("Utilizador Adicionado com Sucesso!\n");
    }
    return c;
}

////FUNÇAO PARA LISTAR TODOS OS CLIENTES

void Users(Cliente *c) {
    Cliente *it;
    int i = 1;
    printf("Utilizadores: \n\n");
    it = c;

    while (it != NULL) {
        printf("%d - %d - %s\n", i, it->Ajogar, it->nome);
        fflush(stdout);
        it = (Cliente*) it->p;
        i++;
    }
}

/// LE OS CLIENTES DO FICHEIRO DE TEXTOl

Cliente* LeClientes() {
    FILE *fd = fopen("clientes.txt", "rt");
    Cliente *clientes = malloc(sizeof (Cliente));
    strcpy(clientes->nome, "Admin");
    strcpy(clientes->PalavraChave, "Admin");
    clientes->PID = -1;
    clientes->p = NULL;

    if (fd == NULL) {
        printf("<AVISO> Ainda Nao tem Clientes\n");
        fflush(stdout);
        return NULL;
    }

    char nome[50];
    char pass[50];
    Cliente *it;
    Cliente *novo;



    it = clientes;

    while (fscanf(fd, "%s %s", nome, pass) > 0) {
        novo = (Cliente*) malloc(sizeof (Cliente));
        strcpy(novo->nome, nome);
        strcpy(novo->PalavraChave, pass);
        novo->PID = 0;
        novo->Ajogar = 0;
        novo->p = NULL;
        it->p = novo;
        it = novo;
    }

    it->p = NULL;
    fclose(fd);
    return clientes;
}

///GRAVA CLIENTES NO FICHEIRO DE TEXTO

void GravaClientes(Cliente *clientes) {
    unlink("clientes.txt");
    FILE *fd = fopen("clientes.txt", "wt");
    Cliente *it;

    it = clientes;
    it = it->p;
    while (it != NULL) {
        fprintf(fd, "%s %s\n", it->nome, it->PalavraChave);
        it = (Cliente*) it->p;
    }
    fclose(fd);
    return;
}

///RECEBE O JOGADORES (CLIENTES)

void *RecebeJogadores(void *dados) {
    char str[80];
    int fd, fd_resp, i;
    int res;
    int *Sair;
    Cliente c;
    Objecto *ob;
    PassarThread *x = (PassarThread*) dados;
    PassarThreadJogo *d = (PassarThreadJogo*) malloc(sizeof (PassarThreadJogo));
    pthread_t envia;
    const char* s = getenv("NMAXPLAY");
    int nPlayers = 0;

    if (s == NULL) {
        srand((unsigned) time(NULL));
        nPlayers = 1 + (rand() % 20);
    } else {
        nPlayers = atoi(s);
    }

    printf("Players: %d\n", nPlayers);
    fflush(stdout);

    Cliente *Clientes = x->clientes;
    ob = x->objectos;

    d->Sair = x->Sair;
    d->objectos = ob;
    d->clientes = Clientes;
    ob = x->objectos;
    Sair = x->Sair;

    mkfifo(FIFOLOGIN, 0600);

    fd = open(FIFOLOGIN, O_RDWR);

    while (*Sair == 0) {
        
        printf("ENTROU");
        fflush(stdout);
        i = read(fd, &c, sizeof (c));

        if (i == sizeof (c)) {
            sprintf(str, "../JJJ%d", c.PID);

            fd_resp = open(str, O_WRONLY);
            if (fd_resp == -1) {
                printf("Erro %d\n", c.PID);
                fflush(stdout);
            } else {
                res = VerificaCliente(Clientes, c);
                write(fd_resp, &res, sizeof (res));
                close(fd_resp);
                if (Conta(Clientes) == 1) {//ALTERAR
                    printf("\nLimite de Clientes atingido\n");
                    fflush(stdout);
                    close(fd);
                    unlink(FIFOLOGIN);
                    sleep(1);
                    pthread_create(&envia, NULL, &EnviaDadosJagador, (void *) d);
                    pthread_exit(0);
                }
            }
        }

    }

    close(fd);
    unlink(FIFOLOGIN);
    pthread_exit(0);

}

///THREAD DE CADA JOGADOR PARA ENVIAR OS DADOS DO JOGO

void *EnviaDadosJagador(void *dados) {
    PassarThreadJogo *x = (PassarThreadJogo*) dados;
    Cliente *it;
    Objecto *itb;
    Objecto final;
    final.id = -1;
    char str[50];
    int fd;
    pthread_t atualiza;

    itb = x->objectos;
    it = x->clientes;
    it = it->p;

    while (it != NULL) {
        if (it->Ajogar == 1) {
            sprintf(str, "../JJJ%d", it->PID);
            fd = open(str, O_WRONLY);
            if (fd == -1) {
                printf("<ERRO> Nao foi possivel abrir o FIFO <%s>\n", str);
                fflush(stdout);
            } else {
                itb = x->objectos;
                while (itb != NULL) {
                    printf("ESCREVEU: %d\n", itb->id);
                    fflush(stdout);
                    write(fd, itb, sizeof (Objecto));
                    itb = itb->p;
                }
                write(fd, &final, sizeof (Objecto));
                close(fd);
            }
        }

        it = it->p;
    }
    pthread_create(&atualiza, NULL, &CorpoJogo, (void *) x);
    pthread_exit(0);
}

/// VERIFICA SE O CLIENTE EXISTE E SE JÁ ESTÁ ONLINE

int VerificaCliente(Cliente *cl, Cliente c) {
    Cliente *it;
    it = cl;
    while (it != NULL) {
        if (strcmp(c.nome, it->nome) == 0 && strcmp(c.PalavraChave, it->PalavraChave) == 0) {
            if (it->Ajogar == 0) {
                it->Ajogar = 1;
                it->PID = c.PID;
                return 1;
            } else {
                return 0;
            }
        }
        it = (Cliente*) it->p;
    }
    return -1;
}

/// CONTA O NUMERO DE CLIENTES 

int Conta(Cliente *c) {
    int i = 0;

    c = c->p;

    while (c != NULL) {
        if (c->Ajogar == 1) {
            i++;
        }
        c = c->p;
    }

    return i;
}

///FUNÇÃO QUE LE DE UM FICHEIRO O LABIRINTO

Objecto* LeLabirinto() {
    FILE *fd = fopen("Labirinto.txt", "rt");
    char c;
    Objecto *ob = NULL;
    Objecto *ultimo = NULL;
    int x, y = 0;

    while ((c = getc(fd)) != EOF) {
        if (c == '\n') {
            x = 0;
            y++;
        } else {
            if (c == '1') {
                if (ob == NULL) {
                    ob = (Objecto*) malloc(sizeof (Objecto));
                    ob->ativo = 1;
                    ob->id = ++id;
                    ob->tipo = 0;
                    ob->x = x;
                    ob->y = y;
                    ob->p = NULL;
                    ultimo = ob;
                } else {
                    ultimo->p = (Objecto*) malloc(sizeof (Objecto));
                    ultimo->p->ativo = 1;
                    ultimo->p->id = ++id;
                    ultimo->p->x = x;
                    ultimo->p->y = y;
                    ultimo->p->tipo = 1;
                    ultimo->p->p = NULL;
                    ultimo = ultimo->p;
                }
            } else {
                if (c == 'S') {
                    if (ob == NULL) {
                        ob = (Objecto*) malloc(sizeof (Objecto));
                        ob->ativo = 1;
                        ob->id = ++id;
                        ob->tipo = 3;
                        ob->x = x;
                        ob->y = y;
                        ob->p = NULL;
                        ultimo = ob;
                    } else {
                        ultimo->p = (Objecto*) malloc(sizeof (Objecto));
                        ultimo->p->ativo = 3;
                        ultimo->p->id = ++id;
                        ultimo->p->x = x;
                        ultimo->p->y = y;
                        ultimo->p->tipo = 1;
                        ultimo->p->p = NULL;
                        ultimo = ultimo->p;
                    }
                }
            }
        }
        x++;
    }
    return ob;
}

//FUNÇÃO QUE MOSTRA AS POSIÇOES DOS OBJETOS

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

void *CorpoJogo(void *dados) {
    PassarThreadJogo *x = (PassarThreadJogo*) dados;
    Jogada j;
    int i, fd;

    mkfifo("../MMM", 0600);
    fd = open("../MMM", O_RDWR);

    if (fd == -1) {
        printf("Erro ao abrir FIFOJOGO\n");
        pthread_exit(0);
    }

    while (*(x->Sair) == 0) {
        i = read(fd, &j, sizeof (j));
        printf("LEU JOGADA: %c\n", j.ascii);
        fflush(stdout);
        if (i == sizeof (j)) {
            TrataAccao(x->objectos, j);
        }
    }
}

////TRATA AS ACÇOES!!

void TrataAccao(Objecto *b, Jogada j) {

    Objecto * it = b;
    Objecto novo;
    char tecla;
    int fd;
    char str[50];

    while (it != NULL) {
        if (it->tipo == j.PID * 10000) {
            tecla = j.ascii;
            sprintf(str, "../JJJ%d", j.PID);
            fd = open(str, O_WRONLY);

            if (fd == -1) {
                printf("Erro ao Abrir FIFO CLIENTE %d", j.PID);
                fflush(stdout);
                break;
            } else {
                if (toupper(tecla) == 'W' || tecla == 30) {
                    novo = VerificaMovimento(1, b, it);
                    if (novo.y != it->y) {
                        write(fd, &novo, sizeof (novo));
                        close(fd);
                    }

                } else {
                    if (toupper(tecla) == 'S' || tecla == 31) {
                        novo = VerificaMovimento(2, b, it);
                        if (novo.y != it->y) {
                            write(fd, &novo, sizeof (novo));
                            close(fd);
                        }

                    } else {
                        if (toupper(tecla) == 'D' || tecla == 16) {
                            novo = VerificaMovimento(3, b, it);
                            if (novo.x != it->x) {
                                write(fd, &novo, sizeof (novo));
                                close(fd);
                            }

                        } else {
                            if (toupper(tecla) == 'A' || tecla == 17) {
                                novo = VerificaMovimento(4, b, it);
                                if (novo.y != it->y) {
                                    write(fd, &novo, sizeof (novo));
                                    close(fd);
                                }

                            } else {
                                if (toupper(tecla) == 'A' || tecla == 32) {

                                }
                            }
                        }
                    }
                }
                break;
            }
        }
        it = it->p;
    }
}

//VERIFICA SE A ACÇAO PERTENDIDA É POSSIVEL

Objecto VerificaMovimento(int tecla, Objecto *lob, Objecto *ob) {
    if (tecla == 1) {
        int y = ob->y - 1;

        if (y <= 0) {
            return *ob;
        } else {
            Objecto *it;

            while (it != NULL) {
                if (it->y == y) {
                    return *ob;
                }
                it = it->p;
            }
            ob->y = y;
            return *ob;
        }

    } else {
        if (tecla == 2) {
            int y = ob->y + 1;

            if (y >= 20) {
                return *ob;
            } else {
                Objecto *it;

                while (it != NULL) {
                    if (it->y == y) {
                        return *ob;
                    }
                    it = it->p;
                }
                ob->y = y;
                return *ob;
            }
        } else {
            if (tecla == 3) {
                int x = ob->x + 1;

                if (x >= 30) {
                    return *ob;
                } else {
                    Objecto *it;

                    while (it != NULL) {
                        if (it->x == x) {
                            return *ob;
                        }
                        it = it->p;
                    }
                    ob->x = x;
                    return *ob;
                }
            } else {
                if (tecla == 4) {
                    int x = ob->x - 1;

                    if (x <= 0) {
                        return *ob;
                    } else {
                        Objecto *it;

                        while (it != NULL) {
                            if (it->x == x) {
                                return *ob;
                            }
                            it = it->p;
                        }
                        ob->x = x;
                        return *ob;

                    }
                }
            }
        }
    }

    return *ob;
}