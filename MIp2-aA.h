/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de MIp2-aA.c                                          */
/*                                                                        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de aA.c, és a dir, d'aquelles          */
/* funcions que es faran servir en un altre fitxer extern a aA.c,         */
/* p.e., int A_FuncioExterna(arg1, arg2...) { }                           */
/* El fitxer extern farà un #include del fitxer aA.h a l'inici, i         */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES formen la interfície de la capa d'aplicació, en la part       */
/* "compartida".                                                          */
 
#include "MIp2-tT.h"

int A_HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck);
char* A_ObteMissError(void);