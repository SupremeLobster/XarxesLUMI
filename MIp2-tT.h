/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de MIp2-tT.c                                          */
/*                                                                        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de tT.c, és a dir, d'aquelles          */
/* funcions que es faran servir en un altre fitxer extern a tT.c,         */
/* p.e., int T_FuncioExterna(arg1, arg2...) { }                           */
/* El fitxer extern farà un #include del fitxer tT.h a l'inici, i         */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES són la "nova" interfície de la capa de transport (la          */
/* "nova" interfície de sockets), en concret, la part "compartida".       */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

int T_HaArribatAlgunaCosaEnTemps(const int *LlistaSck, int LongLlistaSck, int Temps);
char* T_ObteMissError(void);