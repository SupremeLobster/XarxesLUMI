/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-aA.c que "implementa" la part "compartida" de les capes    */
/* d'aplicació de MI i de LUMI, sobre les capes de transport TCP i UDP    */
/* (fent crides a les "noves" interfícies de les capes TCP i UDP o        */
/* "noves" interfícies de sockets TCP i UDP), "compartides" en el         */
/* sentit que són funcions que poden tractar alhora sockets de MI         */
/* (TCP) i de LUMI (UDP).                                                 */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include "MIp2-aA.h"

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int A_FuncioExterna(arg1, arg2...) { }     */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa d'aplicació, en la part "compartida".  */

/* Examina simultàniament i sense límit de temps (una espera indefinida)  */
/* els sockets (poden ser TCP, UDP i  teclat -stdin-) amb identificadors  */
/* en la llista “LlistaSck” (de longitud “LongLlistaSck” sockets) per     */
/* saber si hi ha arribat alguna cosa per ser llegida.                    */
/*                                                                        */
/* "LlistaSck" és un vector d'enters d'una longitud >= LongLlistaSck      */
/*                                                                        */
/* Retorna -1 si hi ha error; si arriba alguna cosa per algun dels        */
/* sockets, retorna l’identificador d’aquest socket.                      */
/*                                                                        */
/* (aquesta funció és equivalent a la funció T_HaArribatAlgunaCosa() de   */
/* de la “nova” interfície de la capa T però ara a la capa d’aplicació)   */
int A_HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck) {
    
    return T_HaArribatAlgunaCosaEnTemps(LlistaSck, LongLlistaSck, -1);
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets (de la part "compartida" d'aplicació).                */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
/*                                                                        */
/* (aquesta funció és equivalent a la funció T_MostraError() de la "nova" */
/* interfície de la capa T però ara a la capa d’aplicació)                */
char* A_ObteMissError(void) {
    
    return T_ObteMissError();
} 

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int A_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */
