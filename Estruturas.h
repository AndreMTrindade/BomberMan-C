/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Estruturas.h
 * Author: hp
 *
 * Created on 6 de Novembro de 2017, 20:35
 */

#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H

#ifdef __cplusplus
extern "C" {
#endif

#define FIFOLOGIN "../CCC"
    
typedef struct Cliente{
   char nome[50];
   char PalavraChave[50];
   int PID;
   int Ajogar;
   struct Cliente *p;
   int pontos;
   int nMegaBomba;
   int nBomba;
}Cliente;    
    
typedef struct Objecto{
    int id;
    int x,y;
    int ativo;
    int tipo;
    struct Objecto *explosao;
    struct Objecto *p;
}Objecto;
   


#ifdef __cplusplus
}
#endif

#endif /* ESTRUTURAS_H */

