/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer MIp2-nodelumi.c que implementa la interfície aplicació-         */
/* administrador d'un node de LUMI, sobre la capa d'aplicació de (la      */
/* part servidora de) LUMI (fent crides a la interfície de la part        */
/* servidora de LUMI).                                                    */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "MIp2-aLUMIs.h"
#include "MIp2-aA.h"
#include "MIp2-aDNSc.h"


#define N_SOCKETS 2

int N_USUARIS = 0;
int fitxLog;

int cercaUsuari(const char *nomUsuari);
int i_cercaUsuari(const char *nomUsuari, int left, int right);
void inserirOrdenat(const struct pair_MI_status e);
void listUsers();
void addUser();
void removeUser();
void saveTableToFile();

struct sigaction old_action;

void sigint_handler(int sig_no) {
    LUMIs_AcabaServidor(fitxLog);
    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);
}


int main(int argc,char *argv[]) {
    // Llegir fitxer config [easy]
    // Guardar dades en alguna estructura de dades eficient (discutir) [no easy]
    // Rebre i tractar peticions i tal [meh, no te mala pinta]

    FILE *fpConfig;
    int sckUDP, i, long_linia, bytes_llegits;
    char *buffer = NULL;

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &sigint_handler;
    sigaction(SIGINT, &action, &old_action);

    if( (fpConfig = fopen("MIp2-nodelumi.cfg", "r")) == NULL) {
        printf("Error en llegir fitxer\n");
        perror(LUMIs_ObteMissError());
        exit(-1);
    }

    bytes_llegits = getline(&buffer, &long_linia, fpConfig);
    buffer[bytes_llegits-1] = '\0';
    strcpy(nomDomini, buffer);

    while((bytes_llegits = getline(&buffer, &long_linia, fpConfig))!=-1 && N_USUARIS<SIZE_TABLE) { // Mentre no arribem a EOF
        buffer[bytes_llegits-1] = '\0'; // Canviem el \n del final per un \0
        struct pair_MI_status element;

        strcpy(element.adr_MI, buffer);
        element.adr_LUMI = NULL;

        inserirOrdenat(element);
    }
    fclose(fpConfig);
    if(buffer) free(buffer);

    // printf("Domini: %s\n", nomDomini);
    
    // int j;
    // for(j=0; j<N_USUARIS; j++) {
    //     printf("%s\n", taula[j].adr_MI);
    // }

    // COMENÇAR A ESCOLTAR I TRACTAR PETICIONS

    if((sckUDP = LUMIs_IniciaServidor(&fitxLog)) == -1) {
        printf("Error en crear socket UDP\n");
        perror(LUMIs_ObteMissError());
        exit(-1);
    }

    int llistaSockets[2];
    llistaSockets[0] = sckUDP;
    llistaSockets[1] = 0;
    int selected;

    printf("---------------------- ADMINISTRATOR MENU -----------------------------\n");
    printf("1. List users\n");
    printf("2. Add user\n");
    printf("3. Remove user\n");

    //quan es crea l socket d udp.. per alguna rao..rel primer es detecta qu arriba algo
    if((selected = A_HaArribatAlgunaCosa(llistaSockets, 2)) == -1) {
        printf("Error ha arribat alguna cosa\n");
        perror(LUMIs_ObteMissError());
        exit(-1);
    }


    while(1) {
        if((selected = A_HaArribatAlgunaCosa(llistaSockets, 2)) == -1) {
            printf("Error ha arribat alguna cosa\n");
            perror(LUMIs_ObteMissError());
            exit(-1);
        }

        if(selected == sckUDP) {
            char seq[100];
            int long_seq;
            char IPrem[16];
            int portUDPrem, tipus;

            tipus = LUMIs_ServeixPeticio(sckUDP, seq, &long_seq, IPrem, &portUDPrem, fitxLog);
        }
        else {
            char seq[100]; // 100 per si l'usuari entra moltes coses enlloc d'un sol caràcter, així no peta.
            int long_seq;

            if( (long_seq = read(0, seq, sizeof(seq))) == -1 ) {
                perror("Error en read");
                exit(-1);
            }
            seq[1] = '\0';

            if(strcmp(seq, "1") == 0) listUsers();
            else if(strcmp(seq, "2") == 0) addUser();
            else if(strcmp(seq, "3") == 0) removeUser();

            printf("---------------------- ADMINISTRATOR MENU -----------------------------\n");
            printf("1. List users\n");
            printf("2. Add user\n");
            printf("3. Remove user\n");
        }
    }

    LUMIs_AcabaServidor(fitxLog); // No s'hauria d'arribar mai aquí, pero ho posem per si de cas.

    return 0;
 }

/*
    Escriu per pantalla la llista de tots els usuaris donats d'alta en aquest node
    i el seu estat (OFFLINE/ONLINE). En cas d'estar online es mostrarà l'@LUMI de l'usuari.
*/
void listUsers() {
    int i;

    for(i=0; i<N_USUARIS; i++) {
        if(!taula[i].adr_LUMI) printf("OFFLINE %s\n", taula[i].adr_MI);
        else printf("%s %s:%d\n", taula[i].adr_MI, taula[i].adr_LUMI->ip, taula[i].adr_LUMI->port);
    }
}

/*
    Dona d'alta un nou usuari en aquest node.
*/
void addUser() {
    struct pair_MI_status element;

    printf("Enter new user name:");
    scanf("%s", element.adr_MI);
    
    element.adr_LUMI = NULL;

    inserirOrdenat(element);
    
    printf("User %s added correctly\n", element.adr_MI);
}

/*
    Guarda al fitxer de configuració la taula d'usuaris-estats que el node té a memòria (RAM).
*/
void saveTableToFile() {
    FILE *fp;
    int i;

    fp = fopen("MIp2-nodelumi.cfg", "w+");
    
    fprintf(fp, "%s\n", nomDomini);
    for(i=0; i<N_USUARIS; i++) fprintf(fp, "%s\n", taula[i].adr_MI);

    fclose(fp);
}

/*
    Dona de baixa un usuari d'aquest node.
*/
void removeUser() {
    char *nomUsuari = NULL;
    int pos, i;

    printf("Enter user name to remove:");
    scanf("%s", nomUsuari);

    pos = cercaUsuari(nomUsuari);

    if(pos == -1) printf("Error: User not found\n");
    else {
        for(i=pos; i<N_USUARIS-1; i++) taula[i] = taula[i+1];

        printf("User %s deleted correctly\n", nomUsuari);
    }
}

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/*
    "nomUsuari" és l'@MI d'un usuari i volem trobar la posició en què aquesta
    es troba dins la taula d'usuaris. La taula ha d'estar ordenada per @MI, 
    alfabèticament.

    Retorna:
    -1 si l'usuari no existeix a la taula;
    la posició de l'usuari (índex) en el rang [0 , SIZE_TABLE-1] si tot va bé;
*/
int cercaUsuari(const char *nomUsuari) {
    int pos = i_cercaUsuari(nomUsuari, 0, N_USUARIS-1);

    if( strcmp(taula[pos].adr_MI, nomUsuari) != 0) pos = -1;

    return pos;
}

/*
    Fem cerca dicotòmica recursiva.

    "nomUsuari" és l'@MI d'un usuari i volem trobar la posició en què aquesta
    es troba dins la taula d'usuaris. La taula ha d'estar ordenada per @MI, 
    alfabèticament.

    Retorna:
    la posició de l'usuari (índex) en el rang [0 , SIZE_TABLE-1];

    Si l'usuari no es troba en la taula, retorna la posició a on HAURIA d'estar.
    La funció que ha fet la crida té la responsabilitat de comprovar si l'usuari
    que es busca existeix realment en la posició retornada o no.
*/
int i_cercaUsuari(const char *nomUsuari, int left, int right) {
    int mid = (left+right)/2;

    if(left <= right) {
        int cmp = strcmp(nomUsuari, taula[mid].adr_MI);

        if(cmp < 0) {
            return i_cercaUsuari(nomUsuari, left, mid-1);
        }
        else if(cmp > 0) {
            return i_cercaUsuari(nomUsuari, mid+1, right);
        }
        else return mid;
    }
}

/*
    Fa una inserció ordenada de "e" a la taula ordenada d'usuaris-estats.
*/
void inserirOrdenat(const struct pair_MI_status e) {
    int i, pos = i_cercaUsuari(e.adr_MI, 0, N_USUARIS-1);

    if(strcmp(taula[pos].adr_MI, e.adr_MI) != 0) { // Només fem inserció si no hi és a la taula. Evitem tenir usuaris repetits.
        for(i=N_USUARIS; i>pos; i--) {
            strcpy(taula[i].adr_MI, taula[i-1].adr_MI);
            taula[i].adr_LUMI = taula[i-1].adr_LUMI;
        }

        N_USUARIS++;

        strcpy(taula[pos].adr_MI, e.adr_MI);
        taula[pos].adr_LUMI = e.adr_LUMI;
    }
}
