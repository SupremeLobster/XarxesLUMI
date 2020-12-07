/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de MIp2-aDNSc.c                                       */
/*                                                                        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de dnsC.c, és a dir, d'aquelles        */
/* funcions que es faran servir en un altre fitxer extern a dnsC.c,       */
/* p.e., int DNSc_FuncioExterna(arg1, arg2...) { }                        */
/* El fitxer extern farà un #include del fitxer dnsC.h a l'inici, i       */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES formen la interfície de la capa DNS, en la part client.       */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int DNSc_ResolDNSaIP(const char *NomDNS, char *IP);
int DNSc_ResolIPaDNS(const char *IP, char *NomDNS);
char* DNSc_ObteMissError(void);
/* int DNSc_FuncioExterna(arg1, arg2...);                                 */