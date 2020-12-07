/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-tT.c que "implementa" la part "compartida" de les capes    */
/* de transport TCP i UDP, o més ben dit, que encapsula les funcions de   */
/* la interfície de sockets TCP o UDP "compartides" (en el sentit, que    */
/* són funcions que poden tractar alhora sockets TCP i UDP), en unes      */
/* altres funcions més simples i entenedores: la "nova" interfície de     */
/* sockets.                                                               */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include <string.h>
#include <errno.h>
#include "MIp2-tT.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int T_FuncioExterna(arg1, arg2...) { }     */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* són la "nova" interfície de la capa de transport (la "nova" interfície */
/* de sockets), en la part "compartida".                                  */

/* Examina simultàniament durant "Temps" (en [ms]) els sockets (poden ser */
/* TCP, UDP i teclat -stdin-) amb identificadors en la llista “LlistaSck” */
/* (de longitud “LongLlistaSck” sockets) per saber si hi ha arribat       */
/* alguna cosa per ser llegida. Si Temps és -1, s'espera indefinidament   */
/* fins que arribi alguna cosa.                                           */
/*                                                                        */
/* "LlistaSck" és un vector d'enters d'una longitud >= LongLlistaSck.     */
/*                                                                        */
/* Retorna -1 si hi ha error; retorna -2 si passa "Temps" sense que       */
/* arribi res; si arriba alguna cosa per algun dels sockets, retorna      */
/* l’identificador d’aquest socket.                                       */
/*                                                                        */
/* (aquesta funció podria substituir a l'anterior T_HaArribatAlgunaCosa() */
/* ja que quan “Temps” és -1 és equivalent a ella)                        */
int T_HaArribatAlgunaCosaEnTemps(const int *LlistaSck, int LongLlistaSck, int Temps) {
	
	fd_set conjunt; // Conjunt de descriptors de fitxer de lectura
    int descmax = -1; // Número de descriptor de fitxer major
	int i, j;
	int selected = -1;
	int retval;

    // Poden arribar dades via teclat o via scon: fem una llista de lectura amb els dos.
    FD_ZERO(&conjunt); // Esborrem el contingut de la llista

	for(i=0; i<LongLlistaSck; i++) {
		FD_SET(LlistaSck[i], &conjunt); // Afegim (“marquem”) el socket a la llista
		if(LlistaSck[i] > descmax) descmax = LlistaSck[i]; // Trobem el número de descriptor màxim
	}

	struct timeval *t = NULL;
	if(Temps != -1) {
		t = malloc(sizeof(struct timeval));
		t->tv_sec = 0;
		t->tv_usec = Temps*1000;
	}

	if((retval = select(descmax+1, &conjunt, NULL, NULL, t)) == -1) {
		return -1;
	}
	else if(retval) {
		for(j=0; j<LongLlistaSck && selected==-1; j++) {
			if(FD_ISSET(LlistaSck[j], &conjunt)) selected = LlistaSck[j];
		}

		return selected;
	}
	else {
		return -2;
	}
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets (de la part "compartida" de transport).               */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
char* T_ObteMissError(void) {
    
    return strerror(errno);
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int T_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */