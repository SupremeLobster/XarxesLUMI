/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-tUDP.c que "implementa" la capa de transport UDP, o més    */
/* ben dit, que encapsula les funcions de la interfície de sockets        */
/* UDP, en unes altres funcions més simples i entenedores: la "nova"      */
/* interfície de sockets UDP.                                             */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include <string.h>
#include <errno.h>
#include "MIp2-tUDP.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int UDP_FuncioExterna(arg1, arg2...) { }   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* són la "nova" interfície de la capa de transport UDP (la "nova"        */
/* interfície de sockets UDP).                                            */


/* Crea un socket UDP a l’@IP “IPloc” i #port UDP “portUDPloc”            */
/* (si “IPloc” és “0.0.0.0” i/o “portUDPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port UDP, respectivament).      */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int UDP_CreaSock(const char *IPloc, int portUDPloc) {
	
    int scon, i;
    struct sockaddr_in adrloc;

    if((scon = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        return -1;
    }

    adrloc.sin_family = AF_INET;
    adrloc.sin_port = htons(portUDPloc);
    adrloc.sin_addr.s_addr = inet_addr(IPloc);
    
    for(i=0; i<8; i++) { adrloc.sin_zero[i]='\0'; }

    if((bind(scon, (struct sockaddr*)&adrloc, sizeof(adrloc))) == -1) {
        return -1;
    }

    return scon;
}

/* Envia a través del socket UDP d’identificador “Sck” la seqüència de    */
/* bytes escrita a “SeqBytes” (de longitud “LongSeqBytes” bytes) cap al   */
/* socket remot que té @IP “IPrem” i #port UDP “portUDPrem”.              */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0'); "SeqBytes"    */
/* és un vector de chars qualsevol (recordeu que en C, un char és un      */
/* enter de 8 bits) d'una longitud >= LongSeqBytes bytes.                 */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int UDP_EnviaA(int Sck, const char *IPrem, int portUDPrem, const char *SeqBytes, int LongSeqBytes) {
	
    struct sockaddr_in adrrem;
    int i;

    adrrem.sin_family = AF_INET;
    adrrem.sin_port = htons(portUDPrem);
    adrrem.sin_addr.s_addr = inet_addr(IPrem);

    for(i=0; i<8; i++) { adrrem.sin_zero[i]='\0'; }

    return sendto(Sck, SeqBytes, LongSeqBytes, 0, (struct sockaddr*)&adrrem, sizeof(adrrem));
}

/* Rep a través del socket UDP d’identificador “Sck” una seqüència de     */
/* bytes que prové d'un socket remot i l’escriu a “SeqBytes*” (que té     */
/* una longitud de “LongSeqBytes” bytes).                                 */
/*                                                                        */
/* Omple "IPrem" i "portUDPrem" amb respectivament, l'@IP i el #port      */
/* UDP del socket remot.                                                  */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0'); "SeqBytes"    */
/* és un vector de chars qualsevol (recordeu que en C, un char és un      */
/* enter de 8 bits) d'una longitud <= LongSeqBytes bytes.                 */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de bytes rebuts si tot va bé.     */
int UDP_RepDe(int Sck, char *IPrem, int *portUDPrem, char *SeqBytes, int LongSeqBytes) {
	
    struct sockaddr_in adrrem;
    int i, long_adrrem, bytes_rebuts;
    long_adrrem = sizeof(adrrem);

    if((bytes_rebuts = recvfrom(Sck, SeqBytes, LongSeqBytes, 0, (struct sockaddr*)&adrrem, &long_adrrem)) == -1) {
        return -1;
    }

    strcpy(IPrem, inet_ntoa(adrrem.sin_addr));
    *(portUDPrem) = ntohs(adrrem.sin_port);

    return bytes_rebuts;
}

/* S’allibera (s’esborra) el socket UDP d’identificador “Sck”.            */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TancaSock(int Sck) {
	
    return close(Sck);
}

/* Donat el socket UDP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc” i “portUDPloc” amb respectivament, la seva     */
/* @IP i #port UDP.                                                       */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portUDPloc) {
	
    struct sockaddr_in adrloc;
    int long_adrloc = sizeof(adrloc);

    if(getsockname(Sck, (struct sockaddr *)&adrloc, &long_adrloc) == -1) {
        return -1;
    }

    strcpy(IPloc, inet_ntoa(adrloc.sin_addr));
    *(portUDPloc) = ntohs(adrloc.sin_port);

    return 0;
}

/* El socket UDP d’identificador “Sck” es connecta al socket UDP d’@IP    */
/* “IPrem” i #port UDP “portUDPrem” (si tot va bé es diu que el socket    */
/* “Sck” passa a l’estat “connectat” o establert – established –).        */
/*                                                                        */
/* Recordeu que a UDP no hi ha connexions com a TCP, i que això només     */
/* vol dir que es guarda localment l’adreça “remota” i així no cal        */
/* especificar-la cada cop per enviar i rebre. Llavors quan un socket     */
/* UDP està “connectat” es pot fer servir UDP_Envia() i UDP_Rep() (a més  */
/* de les anteriors UDP_EnviaA() i UDP_RepDe()) i UDP_TrobaAdrSockRem()). */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_DemanaConnexio(int Sck, const char *IPrem, int portUDPrem) {
	
    struct sockaddr_in adrrem;
    int i;

    adrrem.sin_family = AF_INET;
    adrrem.sin_port = htons(portUDPrem);
    adrrem.sin_addr.s_addr = inet_addr(IPrem);

    for(i=0; i<8; i++) { adrrem.sin_zero[i]='\0'; }

    return connect(Sck, (struct sockaddr*)&adrrem, sizeof(adrrem));
}

/* Envia a través del socket UDP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket UDP remot amb qui està connectat.                 */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int UDP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes) {
	
    return write(Sck, SeqBytes, LongSeqBytes);
}

/* Rep a través del socket UDP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes” (que té una longitud de “LongSeqBytes” bytes). */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud <= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de bytes rebuts si tot va bé.     */
int UDP_Rep(int Sck, char *SeqBytes, int LongSeqBytes) {
	
    return read(Sck, SeqBytes, LongSeqBytes);
}

/* Donat el socket UDP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem” i             */
/* “portUDPrem” amb respectivament, la seva @IP i #port UDP.              */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int UDP_TrobaAdrSockRem(int Sck, char *IPrem, int *portUDPrem) {
	
    struct sockaddr_in adrrem;
    int long_adrrem = sizeof(adrrem);

    if(getpeername(Sck, (struct sockaddr *)&adrrem, &long_adrrem) == -1) {
        return -1;
    }

    strcpy(IPrem, inet_ntoa(adrrem.sin_addr));
    *(portUDPrem) = ntohs(adrrem.sin_port);

    return 0;
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets UDP.                                                  */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
const char* UDP_ObteMissError(void) {
    
    return strerror(errno);
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int UDP_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es troben */
/* a l'inici d'aquest fitxer.                                             */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */