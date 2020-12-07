/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-aLUMIs.c que implementa la capa d'aplicació de MI, sobre   */
/* la capa de transport UDP (fent crides a la "nova" interfície de la     */
/* capa UDP o "nova" interfície de sockets), en la part servidora.        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions EXTERNES es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "MIp2-tUDP.h"
#include "MIp2-tT.h"
#include "MIp2-aDNSc.h"
#include "MIp2-aLUMIs.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

int Log_CreaFitx(const char *NomFitxLog);
int Log_Escriu(int FitxLog, const char *MissLog);
int Log_TancaFitx(int FitxLog);
/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int LUMIs_FuncioExterna(arg1, arg2...) { } */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI, en la part servidora.            */

/* Explcacio                                                              */
int LUMIs_IniciaServidor(int *fitxLog) {
    int sckUDP;

    if((sckUDP = UDP_CreaSock("0.0.0.0", DEFAULT_PORT)) == -1) {
        return -1;
    }

    char nomFitxLog[100];
    sprintf(nomFitxLog, "nodelumi-%s.log", nomDomini);
    *fitxLog = Log_CreaFitx(nomFitxLog);

    return sckUDP;
}

/* Retorna -1 si hi ha error; 0 si alguna cosa; 1 si petició registre     */
int LUMIs_ServeixPeticio(int sckNodeLUMI, char *SeqBytes, int *LongSeqBytes, char *IPrem, int *portUDPrem, int fitxLog) {
    char tipus[3];
    char linea_rebuda[102];
    char missatge_log[150];
    int long_lineaRebuda;
    
    if((long_lineaRebuda = UDP_RepDe(sckNodeLUMI, IPrem, portUDPrem, linea_rebuda, 102)) == -1) return -1;
    else if( long_lineaRebuda == 0) return 0;

    char temp[102];
    memcpy(temp, linea_rebuda, long_lineaRebuda);
    temp[long_lineaRebuda] = '\0';

    sprintf(missatge_log, "R: %s:UDP:%d, %s, %d", IPrem, *portUDPrem, temp, long_lineaRebuda);
    Log_Escriu(fitxLog, missatge_log);

    tipus[0] = linea_rebuda[0];
    tipus[1] = linea_rebuda[1];
    tipus[2] = '\0';

    *LongSeqBytes = long_lineaRebuda-2;
    
    strcpy(SeqBytes, linea_rebuda+2);

    int ret;
    if( strcmp(tipus, "PR") == 0 ) ret = 1;
    else if( strcmp(tipus, "PD") == 0 ) ret = 2;
    else if( strcmp(tipus, "PL") == 0 ) ret = 3;
    else if( strcmp(tipus, "RL") == 0 ) ret = 4;

    return ret;
}

int LUMIs_Resposta(int sckNodeLUMI, const char *resposta, int longResposta, const char *IPrem, int portUDPrem, int fitxLog) {

    int len_enviada;
    char miss_log[150];

    if( (len_enviada = UDP_EnviaA(sckNodeLUMI, IPrem, portUDPrem, resposta, longResposta)) == -1) {
        return -1;
    }

    char temp[100];
    memcpy(temp, resposta, longResposta);
    temp[longResposta] = '\0';

    sprintf(miss_log, "S: %s:UDP:%d, %s, %d", IPrem, portUDPrem, temp, len_enviada);
    Log_Escriu(fitxLog, miss_log);

    return len_enviada;
}

int LUMIs_DemanaLocalitzacio(int sckNodeLUMI, const char *IPrem, int portUDPrem, char *adrMI_1, char *adrMI_2, int fitxLog) {
    char missatgeLocalitzacio[100];
    char miss_log[100];
    int len;

    len = sprintf(missatgeLocalitzacio, "PL%s:%s", adrMI_1, adrMI_2); // PL

    if(UDP_EnviaA(sckNodeLUMI, IPrem, portUDPrem, missatgeLocalitzacio, len) == -1) {
        return -1;
    }

    if(errno == EHOSTUNREACH) {
        return -1;
    }

    sprintf(miss_log, "S: %s:UDP:%d, %s, %d", IPrem, portUDPrem, missatgeLocalitzacio, len);
    Log_Escriu(fitxLog, miss_log);

    return 0;
}

void LUMIs_AcabaServidor(int fitxLog) {

    Log_TancaFitx(fitxLog);
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets UDP (de la part servidora de LUMI).                   */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
/*                                                                        */
/* (aquesta funció és equivalent a la funció UDP_MostraError() de la      */
/* "nova" interfície de la capa UDP però ara a la capa d’aplicació)       */
const char* LUMIs_ObteMissError(void) {
    
    return UDP_ObteMissError();
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int LUMIs_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */


/* Crea un fitxer de "log" de nom "NomFitxLog".                           */
/*                                                                        */
/* "NomFitxLog" és un "string" de C (vector de chars imprimibles acabat   */
/* en '\0') d'una longitud qualsevol.                                     */
/*                                                                        */
/* Retorna -1 si hi ha error; l'identificador del fitxer creat si tot va  */
/* bé.                                                                    */
int Log_CreaFitx(const char *NomFitxLog) {
    int fd;

    /* Amb O_APPEND: si existeix el fitxer que es vol crear, s'escriu a */
    /* continuació mantenint el que hi havia; amb  O_TRUNC, si existeix */
    /* el fitxer que es vol crear, es trunca  a 0 bytes (s'esborra).    */
    fd = open(NomFitxLog, O_CREAT | O_WRONLY | O_APPEND | O_SYNC, 00644);
    if (fd==-1) return -1;

    if(write(fd, "------ INICI LOG ------\n", 24 )==-1) return -1;
    close(fd);
    fd = open(NomFitxLog, O_CREAT | O_WRONLY | O_APPEND | O_SYNC, 00644);

    return fd; 
}

/* Escriu al fitxer de "log" d'identificador "FitxLog" el missatge de     */
/* "log" "MissLog".                                                       */
/*                                                                        */
/* "MissLog" és un "string" de C (vector de chars imprimibles acabat      */
/* en '\0') d'una longitud qualsevol.                                     */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de caràcters del missatge de      */
/* "log" (sense el '\0') si tot va bé.                                    */
int Log_Escriu(int FitxLog, const char *MissLog) {
    int ncars;
    
    if ((ncars = write(FitxLog, MissLog, strlen(MissLog)))==-1) return -1;

    if (write(FitxLog, "\n", 1)==-1) return -1;

    return ncars;
}

/* Tanca el fitxer de "log" d'identificador "FitxLog".                    */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si tot va bé.                             */
int Log_TancaFitx(int FitxLog) {
    if (write(FitxLog, "------ FINAL LOG ------\n", 24 )==-1) return -1;

    return close(FitxLog);
}

/* Si ho creieu convenient, feu altres funcions INTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */
