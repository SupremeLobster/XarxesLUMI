/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de MIp2-aLUMIc.c                                      */
/*                                                                        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de aLUMIc.c, és a dir, d'aquelles      */
/* funcions que es faran servir en un altre fitxer extern a aLUMIc.c,     */
/* p.e., int LUMIc_FuncioExterna(arg1, arg2...) { }                       */
/* El fitxer extern farà un #include del fitxer aLUMIc.h a l'inici, i     */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES formen la interfície de la capa LUMI, en la part client.      */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MIp2-tT.h"
#include "MIp2-tUDP.h"
#include "MIp2-aDNSc.h"

#define MAX_ATTEMPTS 10 // Maxim d'intents per enviar un paquet
#define DEFAULT_TIMEOUT 500     // 500ms de timeout per rebre resposta de servidor a una peticio
#define DEFAULT_PORT 3344

#define OCUPAT 0
#define LLIURE 1

int estatus;

int LUMIc_IniciaClient(char *nomUsuariDomini, int *fitxLog);
int LUMIc_DemanaRegistre(int sckNodeLUMI, const char *adrMI, int fitxLog);
int LUMIc_DemanaDesregistre(int sckNodeLUMI, const char *adrMI, int fitxLog);
int LUMIc_DemanaLocalitzacio(int sckNodeLUMI, const char *adrMIlocal, const char *adrMIremot, char *IPrem, int *portTCPremot, int fitxLog);
int LUMIc_ServeixLocalitzacio(int sckNodeLUMI, const char *ipTCPloc, int portTCPloc, int fitxLog);
int LUMIc_ServeixPeticio(int sckNodeLUMI, char *SeqBytes, int *LongSeqBytes, int fitxLog);
int LUMIc_AcabaClient(int sckUDP, int fitxLog);
const char* LUMIc_ObteMissError(void);