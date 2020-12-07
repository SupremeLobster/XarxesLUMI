/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer p2p.c que implementa la interfície aplicació-usuari             */
/* de l'aplicació de MI amb l'agent de LUMI integrat, sobre les capes     */
/* d'aplicació de  MI i (la part client de) LUMI (fent crides a les       */
/* interfícies de les capes MI i la part client de LUMI).                 */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <stdio.h> o #include "meu.h"     */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "MIp2-aLUMIc.h"
#include "MIp2-aA.h"
#include "MIp2-aMI.h"


#define N_SOCKETS 3


char ipTCPloc[16] = "0.0.0.0";
int portTCPloc = 0;
int fitxLog;

int DemanarDades(char *nomUsuariDomini);
void IniciarConversa(int scon, int sckUDP, char *nicknameR);

int main(int argc, char *argv[]) {
    int sckUDP, sesc, scon, bytes_llegits;
    char nomUsuariDominiLocal[100];
    char nomUsuariDominiRemot[100];
    char ipTCPrem[16];
    int portTCPrem;
    char nicknameL[100];
    char nicknameR[100];
    char buff[2] = "y\0";

    estatus = LLIURE;

    printf("[SYSTEM] Entra el nom d'usuari local [user@domain] (max 99 caracters):\n");
    if( DemanarDades(nomUsuariDominiLocal) == -1 ) {
        printf("Error al llegir les dades\n");
    }

    printf("[SYSTEM] Entra el nickname (max 99 caracters):\n");
    if( DemanarDades(nicknameL) == -1 ) {
        printf("Error al llegir les dades\n");
    }

    if( (sckUDP = LUMIc_IniciaClient(nomUsuariDominiLocal, &fitxLog)) == -1 ) {
        printf("Error inicialitzar client\n");
    }

    if(LUMIc_DemanaRegistre(sckUDP, nomUsuariDominiLocal, fitxLog) == -1) {
        printf("Error al registrarse al servidor\n");
        perror(LUMIc_ObteMissError());
        exit(-1);
    }

    while( strcmp("y", buff) == 0) {
        estatus = LLIURE;
        strcpy(ipTCPloc, "0.0.0.0");
        portTCPloc = 0;

        if( (sesc=MI_IniciaEscPetiRemConv(&portTCPloc, ipTCPloc)) == -1 ) {
                printf("Error al crear el socket servidor\n");
                perror(A_ObteMissError());
                exit(-1);
        }

        printf("[SYSTEM] Entra el nom d'usuari remot [user@domain] (max 99 caracters) [OPCIONAL]:\n");

        int conjunt[2] = {0, sckUDP};
        int selected;

        if( (selected = A_HaArribatAlgunaCosa(conjunt, 2)) == -1) {
            printf("Error en select\n");
            perror(LUMIc_ObteMissError());
            MI_AcabaEscPetiRemConv(sesc);
            exit(-1);
        }

        if(selected == 0) { // Entrem @MI remota per teclat i ens connectem. Ens comportarem com un client.
            if( DemanarDades(nomUsuariDominiRemot) == -1 ) {
                printf("Error al llegir les dades\n");
            }

            if(LUMIc_DemanaLocalitzacio(sckUDP, nomUsuariDominiLocal, nomUsuariDominiRemot, ipTCPrem, &portTCPrem, fitxLog) == -1) {
                printf("Error al demanar localitzció al servidor\n");
                perror(LUMIc_ObteMissError());
                exit(-1);
            }

            if( (scon = MI_DemanaConv(ipTCPrem, portTCPrem, ipTCPloc, &portTCPloc, nicknameL, nicknameR)) == -1 ) {
                perror("Error en demanar conversa\n");
                perror(A_ObteMissError());
                MI_AcabaEscPetiRemConv(sesc);
                exit(-1);
            }

            estatus = OCUPAT;
        }
        else {
            if(selected == sckUDP) {
                LUMIc_ServeixLocalitzacio(sckUDP, ipTCPloc, portTCPloc, fitxLog);
            }

            int conjunt[1] = {sesc};
            int selected;

            if( (selected = A_HaArribatAlgunaCosa(conjunt, 1)) == -1) {
                printf("Error en select\n");
                perror(LUMIc_ObteMissError());
                MI_AcabaEscPetiRemConv(sesc);
                exit(-1);
            }

            if(selected == sesc) { // Ens arriba petició de connexió per sesc i l'acceptem [IP:port]. Ens comportarem com un servidor.
                if( (scon = MI_AcceptaConv(sesc, ipTCPrem, &portTCPrem, ipTCPloc, &portTCPloc, nicknameL, nicknameR)) == -1) {
                    printf("Error al enviar el nickname local a remot\n");
                    perror(LUMIc_ObteMissError());
                    MI_AcabaConv(scon);
                    MI_AcabaEscPetiRemConv(sesc);
                    exit(-1);

                }
                
                estatus = OCUPAT;
            }
        }

        printf("---------------------- INICI CHAT -----------------------------\n");

        printf("[SYSTEM] Nickname LOC: %s\n", nicknameL);
        printf("[SYSTEM] Nickname REM: %s\n\n", nicknameR);

        printf("[SYSTEM] Sock LOC: @IP %s, TCP, #port %d\n", ipTCPloc, portTCPloc);
        printf("[SYSTEM] Sock REM: @IP %s, TCP, #port %d\n\n", ipTCPrem, portTCPrem);

        printf("[SYSTEM] Per enviar un fitxer utilitzar comanda /sendFile\n\n"); 

        IniciarConversa(scon, sckUDP, nicknameR);

        printf("---------------------- FINAL CHAT -----------------------------\n");

        MI_AcabaEscPetiRemConv(sesc);

        printf("[SYSTEM] Vols crear una conversa nova? y/n\n");
        scanf("%s", buff);
    }
    

    if(LUMIc_DemanaDesregistre(sckUDP, nomUsuariDominiLocal, fitxLog) == -1) {
        printf("Error al desregistrarse del servidor\n");
        perror(LUMIc_ObteMissError());
        exit(-1);
    }

    LUMIc_AcabaClient(sckUDP, fitxLog);

    return 0;
}

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/**
 * Demana per pantalla el nom d-usuari i domini i l'omple
 * 
 * "nomUsuariDomini" és una "string" de C d'una longitud màxima de 100 chars 
 * incloent el '\0', en el format [user@domain]
 * 
 * Retorna -1 si hi ha error, 1 si tot va bé
*/
int DemanarDades(char *nomUsuariDomini) {
    if(scanf("%s", nomUsuariDomini) < 0) return -1;

    return 1;
}

/**
 * Realitza la conversa
 * 
 * "nicknameR" és una "string" de C d'una longitud màxima de 100 chars 
 * incloent el '\0'
*/
void IniciarConversa(int scon, int sckUDP, char *nicknameR) {
    int conjunt[N_SOCKETS] = {0, scon, sckUDP};
    int selected;
    int bytes_llegits, bytes_escrits;
    char buffer[99]; // Buffer per llegir, escriure i enviar missatges

    int fi = 0;

    while( fi != 1) {
        // Select
        if( (selected = A_HaArribatAlgunaCosa(conjunt, N_SOCKETS)) == -1) {
            //perror("Error en select");
            //perror(A_MostraError());
            MI_AcabaConv(scon);
            exit(-1);
        }

        if(selected == 0) { // El teclat està “marcat”? han arribat dades al teclat?
            if( (bytes_llegits = read(0, buffer, sizeof(buffer))) == -1 ) {
                //perror("Error en read");
                MI_AcabaConv(scon);
                exit(-1);
            }

            buffer[bytes_llegits-1] = '\0'; // Eliminem el '\n' i fiquem '\0' per poder fer strcmp()

            if( strcmp(buffer, "/end") == 0 ) {
                MI_AcabaConv(scon);
                fi=1;
            }
            else if(strcmp(buffer, "/sendFile") == 0) { // Usuari vol enviar un fitxer
                printf("[SYSTEM] Ubicació i nom del fitxer: ");
                
                char fileName[500];
                scanf("%s", fileName);
                
                FILE *fp;
                
                if( (fp = fopen(fileName, "r")) == NULL) {
                    //perror("Error en llegir fitxer");
                    MI_AcabaConv(scon);
                    exit(-1);
                }

                if(MI_EnviaFitxer(scon, fp) == -1) {
                    //perror("Error en llegir fitxer");
                    //perror(A_MostraError());
                    MI_AcabaConv(scon);
                    exit(-1);
                }
            }
            else if( MI_EnviaLinia(scon, buffer, bytes_llegits-1) == -1 ) { // Envia linea. Enviem bytes_llegits-1 per no enviar el '\0'
                //perror("Error envia");
                //perror(A_MostraError());
                MI_AcabaConv(scon);
                exit(-1);
            }
        }

        if(selected == scon) { // El socket scon està “marcat”? han arribat dades a scon?
            int sizeBuffer;
            int status;
            
            if( (status = MI_ServeixPeticio(scon, buffer, &sizeBuffer)) == -1) {
                //perror("Error servir peticio");
                //perror(A_MostraError());
                MI_AcabaConv(scon);
                exit(-1);
            }

            buffer[sizeBuffer] = '\0'; // Afegim el '\0' per poder fer servir el printf()

            if(status == 0) {
                fi = 1;
            }
            else if(status == 1) {
                if(printf("[%s]: %s\n", nicknameR, buffer) < 0) {
                    //perror("Error en write");
                    MI_AcabaConv(scon);
                    exit(-1);
                }
            }
            else if(status == 3) {
                printf("[SYSTEM] S'ha rebut un fitxer! Directori i nom a guardar-lo:\n");
                
                char fileName[500];
                scanf("%s", fileName);
                
                int mida = atoi(buffer);

                if(MI_RepFitxer(scon, fileName, mida) == -1) {
                    //perror("Error en rebre fitxer");
                    //perror(A_MostraError());
                    MI_AcabaConv(scon);
                }
            }
        }

        if(selected == sckUDP) {
            int status = LUMIc_ServeixLocalitzacio(sckUDP, ipTCPloc, portTCPloc, fitxLog);
            
            if(status == 0) {
                fi = 1;
            }
        }
    }
}