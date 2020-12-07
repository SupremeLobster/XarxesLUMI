/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-aDNSc.c que "implementa" la capa d'aplicació DNS (part     */
/* client), o més ben dit, que encapsula les funcions de la interfície    */
/* de sockets de la part DNS, en unes altres funcions més simples i       */
/* entenedores: la"nova" interfície de la capa DNS, en la part client.    */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions EXTERNES es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include "MIp2-aDNSc.h"

//* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int DNSc_FuncioExterna(arg1, arg2...) { }  */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa DNS, en la part client.                */

/* Donat el nom DNS "NomDNS" obté la corresponent @IP i l'escriu a "IP"   */
/*                                                                        */
/* "NomDNS" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud qualsevol, i "IP" és un "string" de C (vector de  */
/* chars imprimibles acabat en '\0') d'una longitud màxima de 16 chars    */
/* (incloent '\0').                                                       */
/*                                                                        */
/* Retorna -1 si hi ha error; un valor positiu qualsevol si tot va bé     */
int DNSc_ResolDNSaIP(const char *NomDNS, char *IP) {
	
    struct hostent *host = gethostbyname(NomDNS);

    if(host == NULL) return -1;

    struct in_addr adrHOST;
    adrHOST.s_addr = *((unsigned long *)host->h_addr_list[0]);

    strcpy(IP, (char*)inet_ntoa(adrHOST));

    return 0;
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets (de la part client de DNS).                           */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
/*                                                                        */
char* DNSc_ObteMissError(void) {
    
    return strerror(errno);
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int DNSc_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */

