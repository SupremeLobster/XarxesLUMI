/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de MIp2-tTCP.c                                        */
/*                                                                        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de tTCP.c, és a dir, d'aquelles        */
/* funcions que es faran servir en un altre fitxer extern a tTCP.c,       */
/* p.e., int TCP_FuncioExterna(arg1, arg2...) { }                         */
/* El fitxer extern farà un #include del fitxer tTCP.h a l'inici, i       */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES són la "nova" interfície de la capa de transport TCP (la      */
/* "nova" interfície de sockets TCP).                                     */

#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

int TCP_CreaSockClient(const char *IPloc, int portTCPloc);
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc);
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem);
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem);
int TCP_Envia(int Sck, const void *SeqBytes, int LongSeqBytes);
int TCP_Rep(int Sck, void *SeqBytes, int LongSeqBytes);
int TCP_TancaSock(int Sck);
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc);
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem);
int TCP_TrobaAdrLoc(char *IPloc);
char* TCP_ObteMissError(void);
/* int TCP_FuncioExterna(arg1, arg2...);                                  */
