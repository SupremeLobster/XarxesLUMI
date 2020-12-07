/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-aMI.c que implementa la capa d'aplicació de MI, sobre la   */
/* cap de transport TCP (fent crides a la "nova" interfície de la         */
/* capa TCP o "nova" interfície de sockets TCP).                          */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include "MIp2-tTCP.h"
#include "MIp2-tT.h"
#include "MIp2-aMI.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int MI_FuncioExterna(arg1, arg2...) { }    */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa MI.                                    */

/* Inicia l’escolta de peticions remotes de conversa a través d’un nou    */
/* socket TCP, que tindrà una @IP local qualsevol i el #port TCP          */
/* “portTCPloc” (és a dir, crea un socket “servidor” o en estat d’escolta */
/* – listen –).                                                           */
/*                                                                        */
/* Omple “IPloc” i “portTCPloc” amb, respectivament, l’@IP i el #port     */
/* TCP del socket del procés local.                                       */
/* Retorna -1 si hi ha error; l’identificador del socket d’escolta de MI  */
/* creat si tot va bé.                                                    */
int MI_IniciaEscPetiRemConv(int *portTCPloc, char *IPloc) {
    int sesc;

	if( (sesc = TCP_CreaSockServidor( IPloc, *portTCPloc)) == -1 ) return -1;
    
    TCP_TrobaAdrSockLoc(sesc, IPloc, portTCPloc);

    TCP_TrobaAdrLoc(IPloc);
    
    return sesc;
}

/* Crea una conversa iniciada per una petició local que arriba a través   */
/* del teclat: crea un socket TCP “client” (en un #port TCP i @IP local   */
/* qualsevol), a través del qual fa una petició de conversa a un procés   */
/* remot, el qual les escolta a través del socket TCP ("servidor") d'@IP  */
/* “IPrem” i #port TCP “portTCPrem” (és a dir, crea un socket “connectat” */
/* o en estat establert – established –). Aquest socket serà el que es    */
/* farà servir durant la conversa.                                        */
/*                                                                        */
/* Omple “IPloc” i “portTCPloc” amb, respectivament, l’@IP i el #port     */
/* TCP del socket del procés local.                                       */
/*                                                                        */
/* El nickname local “NicLoc” i el nickname remot són intercanviats amb   */
/* el procés remot, i s’omple “NickRem” amb el nickname remot. El procés  */
/* local és qui inicia aquest intercanvi (és a dir, primer s’envia el     */
/* nickname local i després es rep el nickname remot).                    */
/*                                                                        */
/* "IPrem" i "IPloc" són "strings" de C (vectors de chars imprimibles     */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/* "NicLoc" i "NicRem" són "strings" de C (vectors de chars imprimibles   */
/* acabats en '\0') d'una longitud màxima de 100 chars (incloent '\0').   */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket de conversa de   */
/* MI creat si tot va bé.                                                 */
int MI_DemanaConv(const char *IPrem, int portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem) {
    int scon;
    char lineaPerEnviar[102];
    int nickLenght = 0;
    int i = 0;    

	if( ( scon = TCP_CreaSockClient( "0.0.0.0", 0)) == -1 ) return -1;

    if( TCP_DemanaConnexio( scon, IPrem, portTCPrem ) == -1 )return -1;
    if( TCP_TrobaAdrSockLoc( scon, IPloc, portTCPloc ) == -1 ) return -1;

    while(NicLoc[i] != '\0') { nickLenght++; i++; }
    char nickEnviar[nickLenght];
    memcpy(nickEnviar, NicLoc, nickLenght);

    sprintf(lineaPerEnviar, "N%02d%s", nickLenght, nickEnviar);

    if( TCP_Envia(scon, lineaPerEnviar, nickLenght+3) == -1 )return -1;

    int longNicRem;
    if(MI_ServeixPeticio(scon, NicRem, &longNicRem) != 2) return -1;
    NicRem[longNicRem] = '\0';

    return scon;
}

/* Crea una conversa iniciada per una petició remota que arriba a través  */
/* del socket d’escolta de MI d’identificador “SckEscMI” (un socket       */
/* “servidor”): accepta la petició i crea un socket (un socket            */
/* “connectat” o en estat establert – established –), que serà el que es  */
/* farà servir durant la conversa.                                        */
/*                                                                        */
/* Omple “IPrem”, “portTCPrem”, “IPloc” i “portTCPloc” amb,               */
/* respectivament, l’@IP i el #port TCP del socket del procés remot i del */
/* socket del procés local.                                               */
/*                                                                        */
/* El nickname local “NicLoc” i el nickname remot són intercanviats amb   */
/* el procés remot, i s’omple “NickRem” amb el nickname remot. El procés  */
/* remot és qui inicia aquest intercanvi (és a dir, primer es rep el      */
/* nickname remot i després s’envia el nickname local).                   */
/*                                                                        */
/* "IPrem" i "IPloc" són "strings" de C (vectors de chars imprimibles     */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/*                                                                        */
/* "NicLoc" i "NicRem" són "strings" de C (vectors de chars imprimibles   */
/* acabats en '\0') d'una longitud màxima de 100 chars (incloent '\0').   */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket de conversa      */
/* de MI creat si tot va bé.                                              */
int MI_AcceptaConv(int SckEscMI, char *IPrem, int *portTCPrem, char *IPloc, int *portTCPloc, const char *NicLoc, char *NicRem) {
	int scon;
    char lineaPerEnviar[102];
    int nickLenght = 0;
    int i = 0;    
    
    if( (scon = TCP_AcceptaConnexio(SckEscMI, IPrem, portTCPrem) ) == -1 ) return -1;
    if( TCP_TrobaAdrSockLoc( scon, IPloc, portTCPloc ) == -1 ) return -1;

    int longNicRem;
    if(MI_ServeixPeticio(scon, NicRem, &longNicRem) != 2) return -1;
    NicRem[longNicRem] = '\0';

    while(NicLoc[i] != '\0') { nickLenght++; i++; }
    char nickEnviar[nickLenght];
    memcpy(nickEnviar, NicLoc, nickLenght);

    sprintf(lineaPerEnviar, "N%02d%s", nickLenght, nickEnviar);

    if( TCP_Envia( scon, lineaPerEnviar, nickLenght+3 ) == -1 )return -1;

    return scon;
}

/* A través del socket de conversa de MI d’identificador “SckConvMI” (un  */
/* socket "connectat") envia una línia escrita per l’usuari local, els    */
/* caràcters i el nombre de caràcters de la qual es troben a “Linia” i    */
/* “LongLinia”, respectivament                                            */
/*                                                                        */
/* "Linia" no és un "string" de C sinó un vector de chars imprimibles,    */
/* sense el caràter fi d'string ('\0'); tampoc conté el caràcter fi de    */
/* línia ('\n') i cal que 0<= LongLinia <= 99.                            */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de caràcters de la línia enviada  */
/* si tot va bé.                                                          */
int MI_EnviaLinia(int SckConvMI, const char *Linia, int LongLinia) {
    int res;

	if( LongLinia < 0 || LongLinia > 99 ) return -1;

    char lineaPerEnviar[102]; // 1 tipus + 4 LongLinia(int) + 99 Linia + 1 char(\0)

    sprintf(lineaPerEnviar, "L%02d%s", LongLinia, Linia); 

    if( (res = TCP_Envia(SckConvMI, lineaPerEnviar, LongLinia+3)) == -1 )return -1;

    return res;
}

/* A través del socket de conversa de MI d’identificador “SckConvMI” (un  */
/* socket "connectat") envia un fitxer, el punter del qual es troba a "fp"*/
/* Retorna -1 si hi ha error; 0 si tot va bé                              */
int MI_EnviaFitxer(int SckConvMI, FILE *fp) {
    char header[CHUNK_SIZE+3];
    char chunk[CHUNK_SIZE];

    fseek(fp, 0L, SEEK_END); // seek to end of file
    int size = ftell(fp); // get current file pointer
    fseek(fp, 0L, SEEK_SET); // seek back to beginning of file

    int x = 1;
    int num = size;
    while(num/10!=0) { num = num/10; x++; }
    
    sprintf(header, "F%02d%d", x, size);

    if(TCP_Envia(SckConvMI, header, 3+x) == -1) return -1;

    char c;
    int i=0;
    int n=0;
    char metaChunk[CHUNK_SIZE+3];

    while(feof(fp) == 0) { // Mentres no arribem a EOF
        i = fread(chunk, sizeof(char), CHUNK_SIZE, fp);

        sprintf(metaChunk, "D%02d", i);
        memcpy(metaChunk+3, chunk, i);


        if(TCP_Envia(SckConvMI, metaChunk, i+3) == -1) return -1;
        bzero(metaChunk, CHUNK_SIZE+3);
        bzero(chunk, CHUNK_SIZE);
        i = 0;
    }

    fclose(fp);

    return 0;
}

/* A través del socket de conversa de MI d’identificador “SckConvMI” (un  */
/* socket "connectat") rep un fitxer, el directori i la mida del qual es  */
/* troben respectivament a "fileName" i "mida".                           */
/* Retorna -1 si hi ha error; 0 si tot va bé                              */
int MI_RepFitxer(int SckConvMI, const char *fileName, int mida) {
    FILE *fp;
    char chunk[CHUNK_SIZE];
    int long_chunk;

    if( (fp = fopen(fileName, "w")) == NULL ) return -1;

    int mida_llegida = 0;
        
    while(mida_llegida < mida) {
        if(MI_ServeixPeticio(SckConvMI, chunk, &long_chunk) == -1) return -1;

        mida_llegida += long_chunk;

        fwrite(chunk, sizeof(char), long_chunk, fp);
        bzero(chunk, CHUNK_SIZE);
    }

    fclose(fp);

    return 0;
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
/* Retorna -1 si hi ha error; 0 si l’usuari remot acaba la conversa; 1 si */
/* rep una línia; 2 si rebem un nickname; 3 si rep un fitxer;             */
/* 4 si rep dades binàries                                                */
int MI_ServeixPeticio(int SckConvMI, void *SeqBytes, int *LongSeqBytes) {   
    char tipus;
    char longSeq[3];
    char linea_rebuda[102];
    int long_lineaRebuda;

    if(( long_lineaRebuda = TCP_Rep(SckConvMI, linea_rebuda, 102) ) == -1) return -1;
    else if( long_lineaRebuda == 0) return 0;

    tipus = linea_rebuda[0]; //seqbytes -> 1 pra posicio el ti1pus , 2 i 3 posicio la longitud, tot lo altre info
    longSeq[0] = linea_rebuda[1];
    longSeq[1] = linea_rebuda[2];
    longSeq[2] = '\0';
    
    *LongSeqBytes = atoi(longSeq);

    memcpy(SeqBytes, linea_rebuda+3, *LongSeqBytes); //nose si esta be (copiem valors nomes de la informacio)

    int ret;
    if( tipus == 'L' ) ret = 1;
    else if(tipus == 'N') ret = 2;
    else if(tipus == 'F') ret = 3;
    else if(tipus == 'D') ret = 4;

    return ret;
}

/* Acaba la conversa associada al socket de conversa de MI                */
/* d’identificador “SckConvMI” (un socket “connectat”).                   */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int MI_AcabaConv(int SckConvMI) {
    
    return TCP_TancaSock(SckConvMI);
}

/* Acaba l’escolta de peticions remotes de conversa que arriben a través  */
/* del socket d’escolta de MI d’identificador “SckEscMI” (un socket       */
/* “servidor”).                                                           */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int MI_AcabaEscPetiRemConv(int SckEscMI) {
	
    return TCP_TancaSock(SckEscMI);
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets TCP (de MI).                                          */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
/*                                                                        */
/* (aquesta funció és equivalent a la funció TCP_MostraError() de la      */
/* "nova" interfície de la capa TCP però ara a la capa d’aplicació)       */
char* MI_ObteMissError(void) {
    
    return TCP_ObteMissError();
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int MI_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */
