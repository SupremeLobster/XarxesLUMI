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

    nPeticionsIguals = 0;
    SeqBytesAnterior[0] = '\0';

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

    int nTipus;
    if( strcmp(tipus, "PR") == 0 ) nTipus = 1;
    else if( strcmp(tipus, "PD") == 0 ) nTipus = 2;
    else if( strcmp(tipus, "PL") == 0 ) nTipus = 3;
    else if( strcmp(tipus, "RL") == 0 ) nTipus = 4;

    SeqBytes[*LongSeqBytes] = '\0'; // Convertim a string

    // Es podria millorar guardant una taula que conti quantes peticions iguals s'han rebut des d'un origen concret (cada entrada de la taula tindria aquesta informació per un origen diferent)
    // ---------------------------------
    // | ORIGEN | MISSATGE | QUANTITAT |
    // |--------|----------|-----------|
    // |   MI   |  "PXxx"  |     3     |
    // |--------|----------|-----------|
    if(strcmp(SeqBytes, SeqBytesAnterior) == 0) nPeticionsIguals++;
    else nPeticionsIguals = 0;
    strcpy(SeqBytesAnterior, SeqBytes);

    if(nTipus == 1) { // Peticio de registre (PR)
        int i;
        char missatge[3] = "RR0";

        if((i = cercaUsuari(SeqBytes)) == -1) missatge[2] = '1'; // Usuari no existeix

        if(LUMIs_Resposta(sckNodeLUMI, missatge, 3, IPrem, *portUDPrem, fitxLog) == -1) {
            perror("Error enviar resposta\n");
            exit(-1);
        }

        if(i != -1) { // Només fem això si l'usuari SÍ existeix
            if(taula[i].adr_LUMI == NULL) taula[i].adr_LUMI = malloc(sizeof(struct pair_address)); // Reservem memoria

            strcpy(taula[i].adr_LUMI->ip, IPrem); // Assignem valor a IP
            taula[i].adr_LUMI->port = *portUDPrem; // Assignem valor a PORT

            //printf("@MI: %s     @sckLUMI: %s:%d\n", taula[i].adr_MI, taula[i].adr_LUMI->ip, taula[i].adr_LUMI->port);
        }
    }
    else if(nTipus == 2) { // Peticio de desregistre (PD)
        int i;
        char missatge[3] = "RD0";

        if((i = cercaUsuari(SeqBytes)) == -1) missatge[2] = '1'; // Usuari no existeix

        if(LUMIs_Resposta(sckNodeLUMI, missatge, 3, IPrem, *portUDPrem, fitxLog) == -1) {
            perror("Error enviar resposta\n");
            exit(-1);
        }

        if(i != -1) { // Només fem això si l'usuari SÍ existeix
            free(taula[i].adr_LUMI); // Alliberem memoria
            taula[i].adr_LUMI = NULL; // El posem a offline
        }
    }
    else if(nTipus == 3) { // Peticio de localització (PL)
        int i, longMissatge;
        char missatgeResposta[150];

        char adrMI_1[100];
        char nomUsuari_1[100];
        char nomDomini_1[100];

        char adrMI_2[100];
        char nomUsuari_2[100];
        char nomDomini_2[100];

        sscanf(SeqBytes, "%[^:]:%s", adrMI_1, adrMI_2);
        sscanf(adrMI_1, "%[^@]@%s", nomUsuari_1, nomDomini_1);
        sscanf(adrMI_2, "%[^@]@%s", nomUsuari_2, nomDomini_2);

        longMissatge = sprintf(missatgeResposta, "RL0%s:%s", adrMI_1, adrMI_2);

        if( strcmp(nomDomini, nomDomini_2) == 0 ) { // Si la peticio es per a un usuari d'aquest domini
            if((i = cercaUsuari(nomUsuari_2)) == -1) { // Usuari no existeix
                missatgeResposta[2] = '2';
            }
            else { // Només fem això si l'usuari SÍ existeix
                if(taula[i].adr_LUMI != NULL) { // Si l'usuari està marcat com a "online"
                    if(nPeticionsIguals < MAX_ATTEMPTS-1) {
                        if(LUMIs_DemanaLocalitzacio(sckNodeLUMI, taula[i].adr_LUMI->ip, taula[i].adr_LUMI->port, adrMI_1, adrMI_2, fitxLog) == -1) {
                            perror("Error en demanar localització al client 2\n");
                            exit(-1);
                        }
                    }
                    else {
                        missatgeResposta[2] = '3'; // Assumim que l'usuari està "offline" i fem el desregistre
                        free(taula[i].adr_LUMI); // Alliberem memoria
                        taula[i].adr_LUMI = NULL; // El posem a offline
                    }
                }
                else {  // Si l'usuari està marcat com a "offline"
                    missatgeResposta[2] = '3';
                }
            }
        }
        else { // Si la peticio NO es per un usuari d'aquest domini
            char IP_d2[16];
            int portPeticio;

            portPeticio = DEFAULT_PORT;

            if(DNSc_ResolDNSaIP(nomDomini_2, IP_d2) == -1) {
                missatgeResposta[2] = '2';
            }
            else if(LUMIs_DemanaLocalitzacio(sckNodeLUMI, IP_d2, portPeticio, adrMI_1, adrMI_2, fitxLog) == -1) {
                perror("Error en demanar localització al client 2\n");
                exit(-1);
            }
        }

        if(missatgeResposta[2] != '0') {
            if(LUMIs_Resposta(sckNodeLUMI, missatgeResposta, longMissatge, IPrem, *portUDPrem, fitxLog) == -1) {
                perror("Error enviar resposta\n");
                exit(-1);
            }
        }
    }
    else if(nTipus == 4) { // Resposta Localització (RL)
        int i, longMissatge;
        char missatgeResposta[150];

        char IPresposta[16];
        int portResposta;

        char adrMI_1[100];
        char nomUsuari_1[100];
        char nomDomini_1[100];

        longMissatge = sprintf(missatgeResposta, "RL%s", SeqBytes);

        sscanf(SeqBytes+1, "%[^:]", adrMI_1);
        sscanf(adrMI_1, "%[^@]@%s", nomUsuari_1, nomDomini_1);

        if(strcmp(nomDomini, nomDomini_1) == 0) { // Si la petició venia d'un usuari d'aquest domini
            i = cercaUsuari(nomUsuari_1);

            strcpy(IPresposta, taula[i].adr_LUMI->ip);
            portResposta = taula[i].adr_LUMI->port;
        }
        else { // Si la peticio NO venia d'un usuari d'aquest domini, sinó d'un altre node
            DNSc_ResolDNSaIP(nomDomini_1, IPresposta);
            portResposta = DEFAULT_PORT;
        }

        LUMIs_Resposta(sckNodeLUMI, missatgeResposta, longMissatge, IPresposta, portResposta, fitxLog);
    }

    return nTipus;
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
