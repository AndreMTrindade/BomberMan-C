#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

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

typedef struct PassaThreadBomba {
    Cliente *clientes;
    Objecto *objectos;
    Objecto *bomba;
} PassaThreadBomba;


static int id = 0;

Cliente* Shell(Cliente *clientes);
Palavra* DevolvePalavras(char* frase);
void LimpaStdin(void);
int ProcessaComando(Palavra *p);
int Size(Palavra *p);
char* UpString(char *s);
Cliente* AdicionaCliente(Palavra *p, Cliente *c);
void KickPlayer(Palavra *p, Cliente *c);
void Users(Cliente *c);
void Shutdown(Cliente *c);
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
void TrataAccao(Objecto *b, Jogada j, Cliente *c);
Objecto VerificaMovimento(int tecla, Objecto *lob, Objecto *ob, Cliente *c);
void CriarPerso(Cliente *clientes, Objecto *bjectos);
void ColocaJogador(Objecto *novo, Objecto *objectos);
void ColocaInimigo(Objecto *novo, Objecto *objectos);
void EnviaNovopTodos(Objecto novo, Cliente *c);
void *TrataBomba(void *dados);
void *TrataMegaBomba(void *dados);
void CriarFogo(Objecto *objectos, Objecto *bomba);
void CriarFogoMega(Objecto *objectos, Objecto *bomba);
void verificaColisao(Objecto *bomba, Objecto *ob, Cliente *c);
void *MoveInimigo(void *dados);
void ApanhaItem(int id, Objecto *ObjectoLargado, Objecto *lob, Cliente *c);
void jogadorPerdeu(Objecto jogador, Cliente *c);
void largarItem(Objecto *ob, Objecto it2, Cliente *c);
void ColocaPantano(Objecto *novo, Objecto *objectos);

pthread_mutex_t bloqueiaBomba;

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
                KickPlayer(p, clientes);
                break;
            case 3:
                break;
            case 4:
                Shutdown(clientes);
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
            pthread_mutex_destroy(&bloqueiaBomba);
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

    fd = open(FIFOLOGIN, O_RDONLY);

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
    CriarPerso(x->clientes, x->objectos);

    while (it != NULL) {
        if (it->Ajogar == 1 || it->Ajogar == 3) {
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
    close(fd);

    pthread_create(&atualiza, NULL, &CorpoJogo, (void *) x);
    pthread_exit(0);
}

/// VERIFICA SE O CLIENTE EXISTE E SE JÁ ESTÁ ONLINE

int VerificaCliente(Cliente *cl, Cliente c) {
    Cliente *it, *temp;
    it = cl;
    if (c.Ajogar == 3) {
        while (it->p != NULL) {
            it = it->p;
        }

        temp = (Cliente*) malloc(sizeof (Cliente));
        temp->PID = c.PID;
        temp->Ajogar = 3;
        temp->p = NULL;
        it->p = temp;
        return 1;
    }
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
    int x = 0, y = 0;

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
                if (c == '0') {
                    if (ob == NULL) {
                        ob = (Objecto*) malloc(sizeof (Objecto));
                        ob->ativo = 1;
                        ob->id = ++id;
                        ob->tipo = 2;
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
                        ultimo->p->tipo = 2;
                        ultimo->p->p = NULL;
                        ultimo = ultimo->p;
                    }
                } else {
                    if (c == 'o') {
                        if (ob == NULL) {
                            ob = (Objecto*) malloc(sizeof (Objecto));
                            ob->ativo = 1;
                            ob->id = ++id;
                            ob->tipo = 8;
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
                            ultimo->p->tipo = 8;
                            ultimo->p->p = NULL;
                            ultimo = ultimo->p;
                        }
                    } else {
                        if (c == 'O') {
                            if (ob == NULL) {
                                ob = (Objecto*) malloc(sizeof (Objecto));
                                ob->ativo = 1;
                                ob->id = ++id;
                                ob->tipo = 9;
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
                                ultimo->p->tipo = 9;
                                ultimo->p->p = NULL;
                                ultimo = ultimo->p;
                            }
                        }
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

///CORPO DO JOGO
/// TRATA DE RECEBER E ENVIAR AAS JOGADAS

void *CorpoJogo(void *dados) {
    PassarThreadJogo *x = (PassarThreadJogo*) dados;
    Jogada j;
    Objecto novo;
    int i, fd;
    pthread_t recebe;

    mkfifo("../MMM", 0600);
    fd = open("../MMM", O_RDWR);

    if (fd == -1) {
        printf("Erro ao abrir FIFOJOGO\n");
        fflush(stdout);
        pthread_exit(0);
    }

    if (pthread_mutex_init(&bloqueiaBomba, NULL) != 0) {
        printf("\n mutex init failed\n");
        fflush(stdout);
        pthread_exit(0);
    }


    pthread_create(&recebe, NULL, &MoveInimigo, (void *) x);

    while (*(x->Sair) == 0) {
        i = read(fd, &j, sizeof (j));
        if (i == sizeof (j)) {
            printf("RECEBEU Tecla: %c\n", j.ascii);
            fflush(stdout);
            TrataAccao(x->objectos, j, x->clientes);
        }
    }
    pthread_exit(0);
}

////TRATA AS ACÇOES!!

void TrataAccao(Objecto *b, Jogada j, Cliente *c) {

    Objecto * it = b, *it2 = b;
    Objecto novo, *novaBomba;
    Cliente *itc = c;
    char tecla;
    int fd;
    char str[50];
    pthread_t envia;
    PassaThreadBomba *x = (PassaThreadBomba*) malloc(sizeof ( PassaThreadBomba));

    x->clientes = c;
    x->objectos = b;

    while (it != NULL) {
        if (it->tipo == j.PID) {
            tecla = j.ascii;
            if (toupper(tecla) == 'W' || tecla == 30) {
                novo = VerificaMovimento(1, b, it, c);
                if (novo.y != (it->y - 1)) {
                    pthread_mutex_lock(&bloqueiaBomba);
                    EnviaNovopTodos(novo, c);
                    pthread_mutex_unlock(&bloqueiaBomba);
                }

            } else {
                if (toupper(tecla) == 'S' || tecla == 31) {
                    novo = VerificaMovimento(2, b, it, c);
                    if (novo.y != (it->y + 1)) {
                        pthread_mutex_lock(&bloqueiaBomba);
                        EnviaNovopTodos(novo, c);
                        pthread_mutex_unlock(&bloqueiaBomba);
                    }

                } else {
                    if (toupper(tecla) == 'D' || tecla == 16) {
                        novo = VerificaMovimento(3, b, it, c);
                        if (novo.x != (it->x + 1)) {
                            pthread_mutex_lock(&bloqueiaBomba);
                            EnviaNovopTodos(novo, c);
                            pthread_mutex_unlock(&bloqueiaBomba);
                        }

                    } else {
                        if (toupper(tecla) == 'A' || tecla == 17) {
                            novo = VerificaMovimento(4, b, it, c);
                            if (novo.x != (it->x - 1)) {
                                pthread_mutex_lock(&bloqueiaBomba);
                                EnviaNovopTodos(novo, c);
                                pthread_mutex_unlock(&bloqueiaBomba);
                            }
                        } else {
                            if (tecla == 32) {
                                itc = c;
                                while (itc != NULL) {
                                    if (itc->PID == (j.PID - 10000)) {
                                        if (itc->nBomba > 0) {
                                            itc->nBomba--;
                                            break;
                                        } else {
                                            return;
                                        }

                                    }
                                    itc = itc->p;
                                }

                                it2 = b;
                                while (it2->p != NULL) {
                                    it2 = it2->p;
                                }
                                novaBomba = (Objecto*) malloc(sizeof (Objecto));
                                it2->p = novaBomba;

                                id++;
                                novaBomba->id = id;
                                novaBomba->tipo = 3;
                                novaBomba->explosao = NULL;
                                novaBomba->x = it->x;
                                novaBomba->y = it->y;
                                x->bomba = novaBomba;

                                pthread_create(&envia, NULL, &TrataBomba, (void *) x);
                            } else {
                                if (toupper(tecla) == 'B') {
                                    itc = c;
                                    while (itc != NULL) {
                                        if (itc->PID == j.PID - 10000) {
                                            if (itc->nMegaBomba > 0) {
                                                itc->nMegaBomba--;
                                            } else {
                                                return;
                                            }

                                        }
                                        itc = itc->p;
                                    }

                                    it2 = b;
                                    while (it2->p != NULL) {
                                        it2 = it2->p;
                                    }

                                    novaBomba = (Objecto*) malloc(sizeof (Objecto));
                                    it2->p = novaBomba;

                                    id++;
                                    novaBomba->id = id;
                                    novaBomba->tipo = 4;
                                    novaBomba->explosao = NULL;
                                    novaBomba->x = it->x;
                                    novaBomba->y = it->y;
                                    x->bomba = novaBomba;
                                    pthread_create(&envia, NULL, &TrataMegaBomba, (void *) x);
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        it = it->p;
    }
}

//VERIFICA SE A ACÇAO PERTENDIDA É POSSIVEL

Objecto VerificaMovimento(int tecla, Objecto *lob, Objecto *ob, Cliente *c) {

    Objecto *it = lob;
    Cliente *itc;
    int conta = 0;

    if (tecla == 1) {
        int y = ob->y - 1;

        if (y <= 0) {
            return *ob;
        } else {
            while (it != NULL) {
                if (ob->id != it->id && it->y == y && it->x == ob->x) {
                    if ((it->tipo == 6 || it->tipo == 8 || it->tipo == 9 || it->tipo == 5 || it->tipo == 10 || it->tipo == 11) && ob->tipo > 10000) {
                        if (it->tipo == 8 || it->tipo == 9 || it->tipo == 10 || it->tipo == 11) {
                            ApanhaItem(ob->tipo, it, lob, c);
                        }
                        ob->y = y;
                    }
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
                while (it != NULL) {
                    if (ob->id != it->id && it->y == y && it->x == ob->x) {
                        if ((it->tipo == 6 || it->tipo == 8 || it->tipo == 9 || it->tipo == 5 || it->tipo == 10 || it->tipo == 11) && ob->tipo > 10000) {
                            if (it->tipo == 8 || it->tipo == 9 || it->tipo == 10 || it->tipo == 11) {
                                ApanhaItem(ob->tipo, it, lob, c);
                            }
                            ob->y = y;
                        }
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

                    while (it != NULL) {
                        if (ob->id != it->id && it->y == ob->y && it->x == x) {
                            if ((it->tipo == 6 || it->tipo == 8 || it->tipo == 9 || it->tipo == 5 || it->tipo == 10 || it->tipo == 11) && ob->tipo > 10000) {
                                if (it->tipo == 8 || it->tipo == 9 || it->tipo == 10 || it->tipo == 11) {
                                    ApanhaItem(ob->tipo, it, lob, c);
                                }
                                ob->x = x;
                            }
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
                        while (it != NULL) {
                            if (ob->id != it->id && it->y == ob->y && it->x == x) {
                                if ((it->tipo == 6 || it->tipo == 8 || it->tipo == 9 || it->tipo == 5 || it->tipo == 10 || it->tipo == 11) && ob->tipo > 10000) {
                                    if (it->tipo == 8 || it->tipo == 9 || it->tipo == 10 || it->tipo == 11) {
                                        ApanhaItem(ob->tipo, it, lob, c);
                                    }
                                    ob->x = x;
                                }
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

//METE AS PERSONAGENS NO TABULEIRO

void CriarPerso(Cliente *clientes, Objecto *bjectos) {
    Cliente *it;
    Objecto *itb;
    Objecto *ult;
    Objecto *novo;
    int nMaxPantano;
    int i;

    it = clientes;
    itb = bjectos;

    while (itb->p != NULL) {
        itb = itb->p;
    }
    ult = itb;

    while (it != NULL) {
        if (it->Ajogar == 1) {
            novo = (Objecto*) malloc(sizeof (Objecto));
            id++;
            novo->id = id;
            novo->ativo = 1;
            novo->tipo = it->PID + 10000;
            it->pontos = 0;
            it->nMegaBomba = 2;
            it->nBomba = 3;
            ColocaJogador(novo, bjectos);
            ult->p = novo;
            novo->p = NULL;
            ult = novo;
        }
        it = it->p;
    }

    while (itb->p != NULL) {
        itb = itb->p;
    }
    ult = itb;

    for (i = 0; i < 5; i++) {
        novo = (Objecto*) malloc(sizeof (Objecto));
        id++;
        novo->id = id;
        novo->ativo = 1;
        novo->tipo = 7;
        ColocaInimigo(novo, bjectos);
        ult->p = novo;
        novo->p = NULL;
        ult = novo;
    }

    nMaxPantano = 5 + (rand() % 10);

    for (i = 0; i < nMaxPantano; i++) {
        novo = (Objecto*) malloc(sizeof (Objecto));
        id++;
        novo->id = id;
        novo->ativo = 1;
        novo->tipo = 6;
        ColocaPantano(novo, bjectos);
        ult->p = novo;
        novo->p = NULL;
        ult = novo;
    }

}

void ColocaPantano(Objecto *novo, Objecto *objectos) {

    Objecto *it;
    int x = 0, y = 0, sair = 1;

    srand(time(NULL));
    x = rand() % 30;
    x++;
    do {
        it = objectos;
        sair = 1;
        while (it != NULL) {
            if (it->x == x && it->y == y) {
                sair = 0;
                break;
            }
            it = it->p;
        }
        if (sair == 1) {
            novo->x = x;
            novo->y = y;

            break;
        } else
            y = rand() % 20;
        x = rand() % 30;
    } while (sair == 0);
}

//COLOCA O JOGADOR NO MAPA

void ColocaJogador(Objecto *novo, Objecto *objectos) {
    Objecto *it;
    int x = 2, y = 2, sair = 0;

    srand(time(NULL));

    do {
        it = objectos;
        while (it != NULL) {
            if (it->x == x && it->y == y) {
                sair = 0;
                break;
            }
            novo->x = x;
            novo->y = y;
            it = it->p;
            sair = 1;
        }
        y = rand() % 20;
    } while (sair == 0);
}

//COLOCA OS INIMIGOS NO MAPA

void ColocaInimigo(Objecto *novo, Objecto *objectos) {

    Objecto *it;
    int x = 18, y = 2, sair = 1;

    srand(time(NULL));

    do {
        it = objectos;
        sair = 1;
        while (it != NULL) {
            if (it->x == x && it->y == y) {
                sair = 0;
                break;
            }
            it = it->p;
        }
        if (sair == 1) {
            novo->x = x;
            novo->y = y;
            printf("\nposicao %d %d\n", x, y);
            break;
        } else
            y = rand() % 20;

    } while (sair == 0);
}

//CRIA MOVIMENTOS PARA O INIMIGO

void *MoveInimigo(void *dados) {
    PassarThreadJogo *x = (PassarThreadJogo*) dados;
    Objecto *it, alterado;
    int direcao; //1 cima 2 baixo 3 direita 4 esquedar
    int sair = 0, tentativas = 2, tempx, tempy;
    srand(time(NULL));

    while (*(x->Sair) == 0) {
        it = x->objectos;
        sleep(1);
        while (it != NULL) {
            if (it->tipo == 7) {
                sair = 1;
                do {
                    tentativas = 4;
                    direcao = rand() % 4;
                    direcao++;
                    tempx = it->x;
                    tempy = it->y;
                    alterado = VerificaMovimento(direcao, x->objectos, it, NULL);
                    it->ativo = alterado.ativo;
                    it->id = alterado.id;
                    it->tipo = alterado.tipo;
                    it->x = alterado.x;
                    it->y = alterado.y;
                    if (it->x == tempx && it->y == tempy) {
                        tentativas--;
                        if (tentativas == 0) {
                            sair = 0;
                        }
                    } else {
                        sair = 0;
                    }
                } while (sair == 1);
                pthread_mutex_lock(&bloqueiaBomba);
                EnviaNovopTodos(*(it), x->clientes);
                pthread_mutex_unlock(&bloqueiaBomba);
            }
            it = it->p;
        }
    }
}

///KICKA O JOGADOR DO JOGO

void KickPlayer(Palavra *p, Cliente *c) {
    Cliente *it;

    it = c;
    char*nome;
    Palavra *pa = (Palavra*) p->p;
    nome = pa->comando;
    while (it != NULL) {
        if (strcmp(it->nome, nome) == 0) {
            it->Ajogar = -1;
            printf("Utilizador %s fora do Jogo!\n", it->nome);
            return;
        }
        it = it->p;
    }
}

void EnviaNovopTodos(Objecto novo, Cliente *c) {
    char str[50];
    Cliente *it;
    Objecto kick;

    int fd;
    it = c;

    while (it != NULL) {
        if (it->Ajogar == 1 || it->Ajogar == 3) {
            sprintf(str, "../JJJ%d", it->PID);
            fd = open(str, O_WRONLY);

            if (fd == -1) {
                printf("<ERRO> Nao foi possivel abrir o FIFO <%s>\n", str);
                fflush(stdout);
                //   break;
            } else {
                //  printf("ENVIA --> ID: %d TIPO: %d  X: %d Y: %d E: %d\n", novo.id, novo.tipo, novo.x, novo.y, novo.ativo);
                fflush(stdout);
                write(fd, &novo, sizeof (novo));
                close(fd);
            }
        } else {
            if (it->Ajogar == -1) {
                sprintf(str, "../JJJ%d", it->PID);
                fd = open(str, O_WRONLY);

                if (fd == -1) {
                    printf("<ERRO> Nao foi possivel abrir o FIFO <%s>\n", str);
                    fflush(stdout);
                    //   break;
                } else {

                    kick.id = -5;

                    write(fd, &kick, sizeof (kick));
                    close(fd);
                }

            }
        }
        it = it->p;

    }
}

void *TrataBomba(void *dados) {
    PassaThreadBomba *x = (PassaThreadBomba*) dados;
    int periodo = 0;
    Objecto *it = x->objectos;
    Objecto *ant;

    x->bomba->tipo = 3;
    x->bomba->ativo = 1;
    pthread_mutex_lock(&bloqueiaBomba);
    EnviaNovopTodos(*(x->bomba), x->clientes);
    pthread_mutex_unlock(&bloqueiaBomba);

    sleep(2);

    x->bomba->ativo = 0;
    pthread_mutex_lock(&bloqueiaBomba);
    EnviaNovopTodos(*(x->bomba), x->clientes);
    pthread_mutex_unlock(&bloqueiaBomba);

    CriarFogo(x->objectos, x->bomba);


    it = x->bomba->explosao;
    if (x->bomba->explosao != NULL) {
        while (it != NULL) {
            pthread_mutex_lock(&bloqueiaBomba);
            EnviaNovopTodos(*it, x->clientes);
            pthread_mutex_unlock(&bloqueiaBomba);
            it = it->p;
        }
    }

    sleep(2);
    verificaColisao(x->bomba, x->objectos, x->clientes);

    it = x->bomba->explosao;
    if (x->bomba->explosao != NULL) {
        while (it != NULL) {
            it->ativo = 0;
            pthread_mutex_lock(&bloqueiaBomba);
            EnviaNovopTodos(*it, x->clientes);
            pthread_mutex_unlock(&bloqueiaBomba);
            it = it->p;
        }
    }

    it = x->objectos;
    ant = it;

    while (it != NULL) {
        if (it->id == x->bomba->id) {
            ant->p = it->p;
            free(it);
            break;
        }
        ant = it;
        it = it->p;
    }


    pthread_exit(0);
}

void CriarFogo(Objecto *objectos, Objecto *bomba) {
    Objecto *it = bomba;
    Objecto *itb = objectos;
    Objecto temp, *novo;
    int encontrou = 0;
    temp = *bomba;

    id++;
    novo = (Objecto*) malloc(sizeof (Objecto));
    novo->ativo = 1;
    novo->id = id;
    novo->p = NULL;
    novo->tipo = 5;
    novo->x = temp.x;
    novo->y = temp.y;

    it->explosao = novo;
    it = novo;

    for (int i = 0; i < 4; i++) {
        temp = *bomba;
        for (int j = 0; j < 2; j++) {
            if (i == 0) {
                temp.y++;
            } else if (i == 1) {
                temp.y--;
            } else if (i == 2) {
                temp.x++;
            } else if (i == 3) {
                temp.x--;
            }

            encontrou = 1;
            itb = objectos;
            while (itb != NULL) {
                if (itb->x == temp.x && itb->y == temp.y && itb->tipo == 1) {
                    encontrou = 0;

                    j = 2;
                    break;
                }
                itb = itb->p;
            }

            if (encontrou == 1) {
                id++;
                novo = (Objecto*) malloc(sizeof (Objecto));
                novo->ativo = 1;
                novo->id = id;
                novo->p = NULL;
                novo->tipo = 5;
                novo->x = temp.x;
                novo->y = temp.y;

                it->p = novo;
                it = novo;
            }
        }
    }

}

void *TrataMegaBomba(void *dados) {
    PassaThreadBomba *x = (PassaThreadBomba*) dados;
    int periodo = 0;
    Objecto *it = x->objectos;
    Objecto *ant;

    x->bomba->tipo = 4;
    x->bomba->ativo = 1;
    pthread_mutex_lock(&bloqueiaBomba);
    EnviaNovopTodos(*(x->bomba), x->clientes);
    pthread_mutex_unlock(&bloqueiaBomba);

    sleep(2);

    x->bomba->ativo = 0;
    pthread_mutex_lock(&bloqueiaBomba);
    EnviaNovopTodos(*(x->bomba), x->clientes);
    pthread_mutex_unlock(&bloqueiaBomba);

    CriarFogoMega(x->objectos, x->bomba);


    it = x->bomba->explosao;
    if (x->bomba->explosao != NULL) {
        while (it != NULL) {
            pthread_mutex_lock(&bloqueiaBomba);
            EnviaNovopTodos(*it, x->clientes);
            pthread_mutex_unlock(&bloqueiaBomba);
            it = it->p;
        }
    }

    sleep(2);
    verificaColisao(x->bomba, x->objectos, x->clientes);

    it = x->bomba->explosao;
    if (x->bomba->explosao != NULL) {
        while (it != NULL) {
            it->ativo = 0;
            pthread_mutex_lock(&bloqueiaBomba);
            EnviaNovopTodos(*it, x->clientes);
            pthread_mutex_unlock(&bloqueiaBomba);
            it = it->p;
        }
    }

    it = x->objectos;
    ant = it;

    while (it != NULL) {
        if (it->id == x->bomba->id) {
            ant->p = it->p;
            free(it);
            break;
        }
        ant = it;
        it = it->p;
    }
    pthread_exit(0);
}

void CriarFogoMega(Objecto *objectos, Objecto *bomba) {
    Objecto *it = bomba;
    Objecto *itb = objectos;
    Objecto temp, *novo;
    int encontrou = 0;
    temp = *bomba;

    id++;
    novo = (Objecto*) malloc(sizeof (Objecto));
    novo->ativo = 1;
    novo->id = id;
    novo->p = NULL;
    novo->tipo = 5;
    novo->x = temp.x;
    novo->y = temp.y;

    it->explosao = novo;
    it = novo;

    for (int i = 0; i < 4; i++) {
        temp = *bomba;
        for (int j = 0; j < 4; j++) {
            if (i == 0) {
                temp.y++;
            } else if (i == 1) {
                temp.y--;
            } else if (i == 2) {
                temp.x++;
            } else if (i == 3) {
                temp.x--;
            }

            encontrou = 1;
            itb = objectos;
            while (itb != NULL) {
                if (itb->x == temp.x && itb->y == temp.y && itb->tipo == 1) {
                    encontrou = 0;
                    j = 4;
                    break;
                }
                itb = itb->p;
            }

            if (encontrou == 1) {
                id++;
                novo = (Objecto*) malloc(sizeof (Objecto));
                novo->ativo = 1;
                novo->id = id;
                novo->p = NULL;
                novo->tipo = 5;
                novo->x = temp.x;
                novo->y = temp.y;

                it->p = novo;
                it = novo;
            }
        }
    }

}

void Shutdown(Cliente *c) {
    Objecto termina;

    termina.tipo = -1;
    EnviaNovopTodos(termina, c);


}

void verificaColisao(Objecto *bomba, Objecto *ob, Cliente *c) {
    Objecto *it = bomba->explosao;
    Objecto *it2 = ob;
    Objecto *ant = ob;
    Objecto *temp;

    while (it != NULL) {
        it2 = ob;
        while (it2 != NULL) {
            if (it2->tipo == 2 || it2->tipo == 6 || it2->tipo == 7 || it2->tipo > 10000) {
                if (it2->x == it->x && it2->y == it->y) {
                    temp = it2;
                    temp->ativo = 0;
                    if (temp->tipo > 10000) {
                        jogadorPerdeu(*it2, c);
                    } else if (temp->tipo == 7) {
                        largarItem(ob, *it2, c);
                    }

                    pthread_mutex_lock(&bloqueiaBomba);
                    EnviaNovopTodos(*temp, c);
                    pthread_mutex_unlock(&bloqueiaBomba);

                    ant->p = it2->p;
                    it2 = it2 -> p;
                    free(temp);

                } else {
                    ant = it2;
                    it2 = it2->p;
                }
            } else {
                ant = it2;
                it2 = it2->p;
            }

        }
        it = it->p;
    }
}

void ApanhaItem(int id, Objecto *ObjectoLargado, Objecto *lob, Cliente *c) {
    Objecto *ant;
    Objecto *ito;
    Cliente *it = c;
    int contador = 0;

    while (it != NULL) {
        if (it->PID == id - 10000) {
            if (ObjectoLargado->tipo == 8) {
                it->nBomba++;
            } else {
                if (ObjectoLargado->tipo == 9) {
                    it->nMegaBomba++;
                } else if (ObjectoLargado->tipo == 11) {
                    it->pontos += 10;
                } else if (ObjectoLargado->tipo == 10) {
                    ito = lob;
                    while (ito != NULL) {
                        if (ito->tipo >= 8 && ito->tipo <= 9) {
                            switch (ito->tipo) {
                                case 8:
                                    it->nBomba++;
                                    break;
                                case 9:
                                    it->nMegaBomba++;
                                    break;
                                case 10:
                                    ///COLETOS AUTOMATICO
                                    break;
                                case 11:
                                    it->pontos += 10;
                                    break;
                            }
                            contador++;
                            ito->ativo = 0;
                            pthread_mutex_lock(&bloqueiaBomba);
                            EnviaNovopTodos(*(ito), c);
                            pthread_mutex_unlock(&bloqueiaBomba);
                        }
                        if (contador == 5) {
                            break;
                        }
                        ito = ito->p;
                    }
                }
            }
            break;
        }
        it = it->p;
    }

    ito = lob;

    while (ito != NULL) {
        if (ito->id == ObjectoLargado->id) {
            ObjectoLargado->ativo = 0;
            pthread_mutex_lock(&bloqueiaBomba);
            EnviaNovopTodos(*(ObjectoLargado), c);
            pthread_mutex_unlock(&bloqueiaBomba);
            ant->p = ito->p;
            free(ito);
            break;
        }
        ant = ito;
        ito = ito->p;
    }





}

void jogadorPerdeu(Objecto jogador, Cliente *c) {
    Objecto perdeu;
    Cliente *it;
    char str[30];
    int fd;
    sprintf(str, "../JJJ%d", jogador.tipo - 10000);

    perdeu.tipo = -2;


    while (it != NULL) {
        if (it->PID == jogador.tipo - 10000) {
            it->Ajogar = 0;
            break;
        }
        it = it->p;
    }

    fd = open(str, O_WRONLY);

    if (fd == -1) {
        printf("<ERRO> Nao foi possivel abrir o FIFO <%s>\n", str);
        fflush(stdout);
    } else {
        printf("ENVIA -->Jogador: %d perdeu\n", jogador.tipo - 10000);
        fflush(stdout);
        write(fd, &perdeu, sizeof (perdeu));
        close(fd);
    }


}

void largarItem(Objecto *ob, Objecto it2, Cliente *c) {
    Objecto *it, *novo;
    Objecto temp;
    int probabilidade = rand() % 101;

    srand(time(NULL));

    if (probabilidade > 0 && 10 <= probabilidade) {
        temp.tipo = 8;
    } else if (probabilidade > 10 && 35 <= probabilidade) {
        temp.tipo = 9;
    } else if (probabilidade > 35 && 55 <= probabilidade) {
        temp.tipo = 10;
    } else if (probabilidade > 55) {
        temp.tipo = 11;
    }
    temp.tipo == 10;
    it = ob;

    while (it->p != NULL) {
        it = it->p;
    }

    novo = (Objecto*) malloc(sizeof (Objecto));
    it->p = novo;

    id++;
    novo->id = id;
    novo->tipo = 10;
    novo->explosao = NULL;
    novo->x = it2.x;
    novo->y = it2.y;

    pthread_mutex_lock(&bloqueiaBomba);
    EnviaNovopTodos(*(novo), c);
    pthread_mutex_unlock(&bloqueiaBomba);

}
