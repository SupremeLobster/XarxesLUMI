/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-tTCP.c que "implementa" la capa de transport TCP, o més    */
/* ben dit, que encapsula les funcions de la interfície de sockets        */
/* TCP, en unes altres funcions més simples i entenedores: la "nova"      */
/* interfície de sockets TCP.                                             */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include <string.h>
#include <errno.h>
#include "MIp2-tTCP.h"
#include <stdlib.h>
#include <stdio.h>

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int TCP_FuncioExterna(arg1, arg2...) { }   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* són la "nova" interfície de la capa de transport TCP (la "nova"        */
/* interfície de sockets TCP).                                            */

/* Crea un socket TCP “client” a l’@IP “IPloc” i #port TCP “portTCPloc”   */
/* (si “IPloc” és “0.0.0.0” i/o “portTCPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port TCP, respectivament).      */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc) {
	
	struct sockaddr_in adrloc;
	int scon, i;

    if((scon = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }
	
	adrloc.sin_family = AF_INET;
    adrloc.sin_port = htons(portTCPloc);
    adrloc.sin_addr.s_addr = inet_addr(IPloc);

    for(i=0; i<8; i++) { adrloc.sin_zero[i]='\0'; }
	
	if(bind(scon, (struct sockaddr*)&adrloc, sizeof(adrloc)) == -1) {
		return -1;
	}
	
	return scon;
}

/* Crea un socket TCP “servidor” (o en estat d’escolta – listen –) a      */
/* l’@IP “IPloc” i #port TCP “portTCPloc” (si “IPloc” és “0.0.0.0” i/o    */
/* “portTCPloc” és 0 es fa una assignació implícita de l’@IP i/o del      */
/* #port TCP, respectivament).                                            */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc) {
	
	struct sockaddr_in adrloc;
	int sesc, i;

    if((sesc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }
	
	adrloc.sin_family = AF_INET;
    adrloc.sin_port = htons(portTCPloc);
    adrloc.sin_addr.s_addr = inet_addr(IPloc);

    for(i=0; i<8; i++) { adrloc.sin_zero[i]='\0'; }
	
	if(bind(sesc, (struct sockaddr*)&adrloc, sizeof(adrloc)) == -1) {
		return -1;
	}
	
	if((listen(sesc, 3)) == -1) {
        return -1;
    }
	
	return sesc;
}

/* El socket TCP “client” d’identificador “Sck” es connecta al socket     */
/* TCP “servidor” d’@IP “IPrem” i #port TCP “portTCPrem” (si tot va bé    */
/* es diu que el socket “Sck” passa a l’estat “connectat” o establert     */
/* – established –). Recordeu que això vol dir que s’estableix una        */
/* connexió TCP (les dues entitats TCP s’intercanvien missatges           */
/* d’establiment de la connexió).                                         */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem) {
	
	int i;
	struct sockaddr_in adrrem;
	
	adrrem.sin_family = AF_INET;
    adrrem.sin_port = htons(portTCPrem);
    adrrem.sin_addr.s_addr = inet_addr(IPrem); 

    for(i=0; i<8; i++) { adrrem.sin_zero[i] = '\0'; }

	return connect(Sck, (struct sockaddr*)&adrrem, sizeof(adrrem));
}

/* El socket TCP “servidor” d’identificador “Sck” accepta fer una         */
/* connexió amb un socket TCP “client” remot, i crea un “nou” socket,     */
/* que és el que es farà servir per enviar i rebre dades a través de la   */
/* connexió (es diu que aquest nou socket es troba en l’estat “connectat” */
/* o establert – established –; el nou socket té la mateixa adreça que    */
/* “Sck”).                                                                */
/*                                                                        */
/* Omple “IPrem” i “portTCPrem*” amb respectivament, l’@IP i el #port     */
/* TCP del socket remot amb qui s’ha establert la connexió.               */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket connectat creat  */
/* si tot va bé.                                                          */
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem) {

	struct sockaddr_in adrrem;
	int long_adrrem = sizeof(adrrem);
	int scon;

	scon = accept(Sck, (struct sockaddr*)&adrrem, &long_adrrem);
	if(TCP_TrobaAdrSockRem(scon, IPrem, portTCPrem) == -1) {
		return -1;
	}

	return scon;
}

/* Envia a través del socket TCP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket TCP remot amb qui està connectat.                 */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */
int TCP_Envia(int Sck, const void *SeqBytes, int LongSeqBytes) {
	
	int n = write(Sck, SeqBytes, LongSeqBytes);

	return n;
}

/* Rep a través del socket TCP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes” (que té una longitud de “LongSeqBytes” bytes), */
/* o bé detecta que la connexió amb el socket remot ha estat tancada.     */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si la connexió està tancada; el nombre de */
/* bytes rebuts si tot va bé.                                             */
int TCP_Rep(int Sck, void *SeqBytes, int LongSeqBytes) {
	
	int n = read(Sck, SeqBytes, LongSeqBytes);

	return n;
}

/* S’allibera (s’esborra) el socket TCP d’identificador “Sck”; si “Sck”   */
/* està connectat es tanca la connexió TCP que té establerta.             */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TancaSock(int Sck) {
	
	return close(Sck);
}

/* Donat el socket TCP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc” i “portTCPloc” amb respectivament, la seva     */
/* @IP i #port TCP.                                                       */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc) {
	
    struct sockaddr_in loc_adr;
    int loc_adrlon = sizeof(loc_adr);

    if( getsockname(Sck, (struct sockaddr *)&loc_adr, &loc_adrlon) == -1 ) {
        return -1;
    }

	strcpy(IPloc, inet_ntoa(loc_adr.sin_addr));
	*(portTCPloc) = ntohs(loc_adr.sin_port);

	return 0;
}

/* Donat el socket TCP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem” i             */
/* “portTCPrem*” amb respectivament, la seva @IP i #port TCP.             */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé.    */
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem) {
	
	struct sockaddr_in rem_adr;
    int loc_adrlon = sizeof(rem_adr);

    if( getpeername(Sck, (struct sockaddr *)&rem_adr, &loc_adrlon) == -1 ) {
        return -1;
    }

	strcpy(IPrem, inet_ntoa(rem_adr.sin_addr));
	*(portTCPrem) = ntohs(rem_adr.sin_port);

	return 0;
}

/* Omple “IPloc*” amb la @IP local de la interfície			              */
/* "IPloc*" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/* Retorna -1 si hi ha error; 0 si tot va bé.    */
int TCP_TrobaAdrLoc(char *IPloc) {
	FILE *fp;
	char ipbuff[16];

	if( (fp = popen("ifconfig | grep \"inet addr\" | cut -c21- | sed -n '1p' | awk '{print $1}'", "r")) == NULL) return -1;
	int i = fread(ipbuff, sizeof(char), sizeof(ipbuff), fp);
	if(ferror(fp)) return -1;

	pclose(fp);

	ipbuff[i-1] = '\0';

	memcpy(IPloc, ipbuff, i);

	return 0;
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets TCP.                                                  */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
char* TCP_ObteMissError(void) {
    
    return strerror(errno);
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int TCP_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es troben */
/* a l'inici d'aquest fitxer.                                             */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */