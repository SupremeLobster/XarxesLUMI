/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de MIp2-aMI.c                                         */
/*                                                                        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de aMI.c, és a dir, d'aquelles         */
/* funcions que es faran servir en un altre fitxer extern a aMI.c,        */
/* p.e., int MI_FuncioExterna(arg1, arg2...) { }                          */
/* El fitxer extern farà un #include del fitxer aMI.h a l'inici, i        */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES formen la interfície de la capa MI.                           */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define CHUNK_SIZE 99

int MI_IniciaEscPetiRemConv(int *portTCPloc, char *IPloc);
int MI_DemanaConv(const char *IPrem, int portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem);
int MI_AcceptaConv(int SckEscMI, char *IPrem, int *portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem);
int MI_EnviaLinia(int SckConvMI, const char *Linia, int LongLinia);
int MI_EnviaFitxer(int SckConvMI, FILE *fp);
int MI_RepFitxer(int SckConvMI, const char *fileName, int mida);
int MI_ServeixPeticio(int SckConvMI, void *SeqBytes, int *LongSeqBytes);
int MI_AcabaConv(int SckConvMI);
int MI_AcabaEscPetiRemConv(int SckEscMI);
char* MI_ObteMissError(void);
/* int MI_FuncioExterna(arg1, arg2...);                                   */
