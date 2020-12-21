/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-aLUMIc.c que implementa la capa d'aplicació de MI, sobre   */
/* la capa de transport UDP (fent crides a la "nova" interfície de la     */
/* capa UDP o "nova" interfície de sockets UDP), en la part client.       */
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
#include "MIp2-aLUMIc.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

int Envia_i_RepResposta_amb_Intents_i_Timeout(int sckNodeLUMI, char *SeqBytes, int *LongSeqBytes, int milisTimeout, int fitxLog);
int Log_CreaFitx(const char *NomFitxLog);
int Log_Escriu(int FitxLog, const char *MissLog);
int Log_TancaFitx(int FitxLog);
int ctoi(char c);

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int LUMIc_FuncioExterna(arg1, arg2...) { } */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa LUMI, en la part client.               */

/*
    Fa la inicialització de l'agent LUMI creant el socket LUMI UDP i 
    obrint/creant el fitxer de logs. Retorna el file descriptor del 
    socket UDP de LUMI i passa el file descriptor del fitxer de logs per
    paràmetre.

    Retorna -1 si hi ha algún error, 0 en cas contrari.
*/
int LUMIc_IniciaClient(char *nomUsuariDomini, int *fitxLog) {
    int sckUDP;

    if((sckUDP = UDP_CreaSock("0.0.0.0", 0)) == -1) {
        return -1;
    }

    char nomFitxLog[100];
    sprintf(nomFitxLog, "p2p-%s.log", nomUsuariDomini);
    *fitxLog = Log_CreaFitx(nomFitxLog);

    if(*fitxLog == -1) return -1;

    return sckUDP;
}

/*
    Fa una petició de registre al node LUMI. El sckNodeLUMI passarà
    a ser un socket UDP "connectat" amb el node del domini si tot va bé.

    Retorna 0 si tot va bé; -1 si hi ha error; -2 si el node LUMI no ha 
    respost; 1 si l'usuari no està donat d'alta al node del domini.
*/
int LUMIc_DemanaRegistre(int sckNodeLUMI, const char *adrMI, int fitxLog) {
    char nomUsuari[100];
    char nomDomini[100];
    char ipServer[16];
    char missatgeRegistre[102];
    int len;

    sscanf(adrMI, "%[^@]@%s", nomUsuari, nomDomini);

    DNSc_ResolDNSaIP(nomDomini, ipServer);

    UDP_DemanaConnexio(sckNodeLUMI, ipServer, DEFAULT_PORT);

    len = sprintf(missatgeRegistre, "PR%s", nomUsuari);

    return Envia_i_RepResposta_amb_Intents_i_Timeout(sckNodeLUMI, missatgeRegistre, &len, DEFAULT_TIMEOUT, fitxLog);
}

/*
    Fa una petició de desregistre al node LUMI.

    Retorna 0 si tot va bé; -1 si hi ha error; -2 si el node LUMI no ha 
    respost; 1 si l'usuari no està donat d'alta al node del domini.
*/
int LUMIc_DemanaDesregistre(int sckNodeLUMI, const char *adrMI, int fitxLog) {
    char nomUsuari[100];
    char nomDomini[100];
    char ipServer[16];
    char missatgeDesregistre[102];
    int len;

    sscanf(adrMI, "%[^@]@%s", nomUsuari, nomDomini);

    len = sprintf(missatgeDesregistre, "PD%s", nomUsuari);

    return Envia_i_RepResposta_amb_Intents_i_Timeout(sckNodeLUMI, missatgeDesregistre, &len, DEFAULT_TIMEOUT, fitxLog);
}

/*
    Fa una petició de localització de 'adrMIremot' al node LUMI.
    Emplena 'IPrem' i 'portTCPremot' amb, respectivament, l'@IP i el 
    #port TCP d'escolta de l'agent que es vol localitzar.

    Retorna 0 si tot va bé; -1 si hi ha error; -2 si el node LUMI no ha 
    respost; 1 si l'usuari remot està ocupat; 2 si l'usuari remot o 
    el domini no existeix; 3 si l'usuari remot està offline.
*/
int LUMIc_DemanaLocalitzacio(int sckNodeLUMI, const char *adrMIlocal, const char *adrMIremot, char *IPrem, int *portTCPremot, int fitxLog) {
    char ipServer[16];
    char missatgeLocalitzacio[102];
    int res, long_missatge;
    
    long_missatge = sprintf(missatgeLocalitzacio, "PL%s:%s", adrMIlocal, adrMIremot);

    res = Envia_i_RepResposta_amb_Intents_i_Timeout(sckNodeLUMI, missatgeLocalitzacio, &long_missatge, DEFAULT_TIMEOUT*2, fitxLog);

    // Ara, "missatgeLocalitzacio" és un string amb '\0' al final

    char adrMI_1[100];
    char adrMI_2[100];

    sscanf(missatgeLocalitzacio, "%[^:]:%[^:]:%[^:]:%d", adrMI_1, adrMI_2, IPrem, portTCPremot);

    return res;
}

/*
    Serveix una petició de localització que fa el node LUMI del domini.
    'ipTCPloc' i 'portTCPloc' han de contenir, respectivament, l'@IP i 
    el #port TCP d'escolta de l'agent local.

    Retorna 1 si tot va bé; 0 si s'han rebut 0 caràcters;
    -1 si hi ha error.
*/
int LUMIc_ServeixLocalitzacio(int sckNodeLUMI, const char *ipTCPloc, int portTCPloc, int fitxLog) {
    char ipServer[16];
    char missatgeRebut[102];
    char missatgeA_Enviar[102];
    int long_missatge;
    char adrMI_1[100];
    char adrMI_2[100];

    if(LUMIc_ServeixPeticio(sckNodeLUMI, missatgeRebut, &long_missatge, fitxLog) == 0) { // S'han rebut 0 bytes
        return 0;
    }

    missatgeRebut[long_missatge] = '\0'; // Convertim a string

    sscanf(missatgeRebut, "%[^:]:%s", adrMI_1, adrMI_2);

    long_missatge = sprintf(missatgeA_Enviar, "RL0%s:%s:%s:%d", adrMI_1, adrMI_2, ipTCPloc, portTCPloc);

    if(estatus == OCUPAT) { // Si client està en conversa (ocupat)
        missatgeA_Enviar[2] = '1';
    }

    UDP_Envia(sckNodeLUMI, missatgeA_Enviar, long_missatge);

    char missatge_log[150];
    char IPrem[16];
    int portUDPrem;

    UDP_TrobaAdrSockRem(sckNodeLUMI, IPrem, &portUDPrem);

    sprintf(missatge_log, "S: %s:UDP:%d, %s, %d", IPrem, portUDPrem, missatgeA_Enviar, long_missatge);
    Log_Escriu(fitxLog, missatge_log);

    return 1;
}

/* A través del socket de conversa de MI d’identificador “SckConvMI” (un  */
/* socket "connectat") atén una petició de servei de l’usuari remot, i    */
/* omple “SeqBytes” i “LongSeqBytes” amb una “informació” que depèn del   */
/* tipus de petició.                                                      */
/*                                                                        */
/* “LongSeqBytes”, abans de cridar la funció, ha de contenir la longitud  */ 
/* de la seqüència de bytes “SeqBytes”, que ha de ser >= 99 (després de   */
/* cridar-la, contindrà la longitud en bytes d’aquella “informació”).     */
/*                                                                        */
/* Atendre una petició de servei pot ser: rebre una línia escrita per     */
/* l’usuari remot i llavors omplir “SeqBytes” i “LongSeqBytes” amb els    */
/* caràcters i el nombre de caràcters de la línia rebuda, respectivament; */
/* o bé servir un altre tipus de petició de servei (a definir); o bé      */
/* detectar l’acabament de la conversa per part de l’usuari remot.        */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si alguna cosa; 1 si rep resposta petició */
/* registre                                                               */
int LUMIc_ServeixPeticio(int sckNodeLUMI, char *SeqBytes, int *LongSeqBytes, int fitxLog) {
    char tipus[3];
    char linea_rebuda[102];
    char missatge_log[150];
    char IPrem[16];
    int long_lineaRebuda, portUDPrem;

    if(( long_lineaRebuda = UDP_Rep(sckNodeLUMI, linea_rebuda, 102) ) == -1) return -1;
    else if( long_lineaRebuda == 0) return 0;

    UDP_TrobaAdrSockRem(sckNodeLUMI, IPrem, &portUDPrem);

    char temp[102];
    memcpy(temp, linea_rebuda, long_lineaRebuda);
    temp[long_lineaRebuda] = '\0';

    sprintf(missatge_log, "R: %s:UDP:%d, %s, %d", IPrem, portUDPrem, temp, long_lineaRebuda);
    Log_Escriu(fitxLog, missatge_log);

    tipus[0] = linea_rebuda[0];
    tipus[1] = linea_rebuda[1];
    tipus[2] = '\0';
    
    strcpy(SeqBytes, linea_rebuda+2);

    *LongSeqBytes = long_lineaRebuda-2;

    int ret;
    if( strcmp(tipus, "RR") == 0 ) ret = 1;
    else if( strcmp(tipus, "RD") == 0 ) ret = 2;
    else if( strcmp(tipus, "RL") == 0 ) ret = 3;
    else if( strcmp(tipus, "PL") == 0 ) ret = 4;

    return ret;
}

int LUMIc_AcabaClient(int sckUDP, int fitxLog) {

    UDP_TancaSock(sckUDP);

    return Log_TancaFitx(fitxLog);
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets UDP (de la part client de LUMI).                      */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
/*                                                                        */
/* (aquesta funció és equivalent a la funció UDP_MostraError() de la      */
/* "nova" interfície de la capa UDP però ara a la capa d’aplicació)       */
const char* LUMIc_ObteMissError(void) {
    
    return UDP_ObteMissError();
}

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int T_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Retorna -1 si hi ha error; -2 si el node respon amb codi d'error; -3   */
/* si el node no ha respost a la petició; 0 si el node respon amb codi    */
/* correcte                                                               */
int Envia_i_RepResposta_amb_Intents_i_Timeout(int sckNodeLUMI, char *SeqBytes, int *LongSeqBytes, int milisTimeout, int fitxLog) {
    char respostaPeticio[150];
    char missatge_log[150];
    char IPrem[16];
    int portUDPrem;
    int i = 1, hiHaTimeout = 1, selected, long_respostaPeticio;
    int llistaSockets[1];
    int ret;

    llistaSockets[0] = sckNodeLUMI;

    UDP_TrobaAdrSockRem(sckNodeLUMI, IPrem, &portUDPrem);

    while(i<=MAX_ATTEMPTS && hiHaTimeout==1) { // Mentre hi hagi timeouts farà com a màxim MAX_ATTEMPTS. Si en un intent NO es dona timeout, acaba, ja que s'ha rebut resposta.
        if(UDP_Envia(sckNodeLUMI, SeqBytes, *LongSeqBytes) == -1) {
            return -1;
        }

        sprintf(missatge_log, "S: # ATTEMPT: %d, %s:UDP:%d, %s, %d", i, IPrem, portUDPrem, SeqBytes, *LongSeqBytes);
        Log_Escriu(fitxLog, missatge_log);
        
        if((selected = T_HaArribatAlgunaCosaEnTemps(llistaSockets, 1, milisTimeout)) == -1) {
            return -1;
        }
        else if(selected == -2) {
            hiHaTimeout = 1;
        }
        else hiHaTimeout = 0;

        i++;
    }

    if(hiHaTimeout == 0) { // S'ha rebut resposta sense fer timeout dintre del limit d'intents.
        int aux = LUMIc_ServeixPeticio(sckNodeLUMI, respostaPeticio, &long_respostaPeticio, fitxLog);

        if(aux!=1 && aux!=2 && aux!=3) { // Si no retorna "1" o "2" "3", hi ha hagut error. "1" vol dir que ha arribat RR del node, "2" que ha arribat RD, "3" que ha arribat RL
            return -1;
        } // POTSER NO CAL

        ret = ctoi(respostaPeticio[0]);
        
        respostaPeticio[long_respostaPeticio] = '\0'; // Convertim a string

        bzero(SeqBytes, *LongSeqBytes);
        *LongSeqBytes = sprintf(SeqBytes, "%s", respostaPeticio+1);
    }
    else { // NO s'ha rebut resposta. S'ha esgotat el limit d'intents i tots els intents han fet timeout.
        return -2;
    }

    return ret;
}

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

int ctoi(char c) {

    char a[2];
    a[0] = c;
    a[1] = '\0';

    return atoi(a);
}

/* Si ho creieu convenient, feu altres funcions INTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */
