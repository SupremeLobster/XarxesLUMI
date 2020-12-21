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
#define SIZE_TABLE 100


struct pair_MI_status {
    char adr_MI[100];
    struct pair_address *adr_LUMI; // NULL = offline
};

struct pair_MI_status taula[SIZE_TABLE];
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

        if( selected == sckUDP){
            
            char seq[100];
            int long_seq;
            char IPrem[16];
            int portUDPrem, tipus;

            tipus = LUMIs_ServeixPeticio(sckUDP, seq, &long_seq, IPrem, &portUDPrem, fitxLog);
            seq[long_seq] = '\0'; // Convertim a string

            if(tipus == 1) { // Peticio de registre (PR)
                int i;
                char missatge[3] = "RR0";

                if((i = cercaUsuari(seq)) == -1) missatge[2] = '1'; // Usuari no existeix

                if(LUMIs_Resposta(sckUDP, missatge, 3, IPrem, portUDPrem, fitxLog) == -1) {
                    perror("Error enviar resposta\n");
                    exit(-1);
                }

                if(i != -1) { // Només fem això si l'usuari SÍ existeix
                    if(taula[i].adr_LUMI == NULL) taula[i].adr_LUMI = malloc(sizeof(struct pair_address)); // Reservem memoria

                    strcpy(taula[i].adr_LUMI->ip, IPrem); // Assignem valor a IP
                    taula[i].adr_LUMI->port = portUDPrem; // Assignem valor a PORT

                    //printf("@MI: %s     @sckLUMI: %s:%d\n", taula[i].adr_MI, taula[i].adr_LUMI->ip, taula[i].adr_LUMI->port);
                }
            }
            else if(tipus == 2) { // Peticio de desregistre (PD)
                int i;
                char missatge[3] = "RD0";

                if((i = cercaUsuari(seq)) == -1) missatge[2] = '1'; // Usuari no existeix

                if(LUMIs_Resposta(sckUDP, missatge, 3, IPrem, portUDPrem, fitxLog) == -1) {
                    perror("Error enviar resposta\n");
                    exit(-1);
                }

                if(i != -1) { // Només fem això si l'usuari SÍ existeix
                    free(taula[i].adr_LUMI); // Alliberem memoria
                    taula[i].adr_LUMI = NULL; // El posem a offline
                }
            }
            else if(tipus == 3) { // Peticio de localització (PL)
                int i, longMissatge;
                char missatgeResposta[150];

                char adrMI_1[100];
                char nomUsuari_1[100];
                char nomDomini_1[100];

                char adrMI_2[100];
                char nomUsuari_2[100];
                char nomDomini_2[100];

                sscanf(seq, "%[^:]:%s", adrMI_1, adrMI_2);
                sscanf(adrMI_1, "%[^@]@%s", nomUsuari_1, nomDomini_1);
                sscanf(adrMI_2, "%[^@]@%s", nomUsuari_2, nomDomini_2);

                //if( strcmp(nomDomini, nomDomini_1) == 0 ) { // MILLORA 4. Només responem a peticions que ens facin els usuaris d'aquest domini
                    longMissatge = sprintf(missatgeResposta, "RL0%s:%s", adrMI_1, adrMI_2);

                    if( strcmp(nomDomini, nomDomini_2) == 0 ) { // Si la peticio es per un usuari d'aquest domini
                        if((i = cercaUsuari(nomUsuari_2)) == -1) { // Usuari no existeix
                            missatgeResposta[2] = '2';
                        }
                        else { // Només fem això si l'usuari SÍ existeix
                            if(taula[i].adr_LUMI != NULL) { // Si l'usuari està marcat com a "online"
                                if(LUMIs_DemanaLocalitzacio(sckUDP, taula[i].adr_LUMI->ip, taula[i].adr_LUMI->port, adrMI_1, adrMI_2, fitxLog) == -1) {
                                    perror("Error en demanar localització al client 2\n");
                                    exit(-1);
                                }
                            }
                            else {  // Si l'usuari està marcat com a "offline"
                                missatgeResposta[2] = '3';
                            }
                        }
                    }
                    else { // Si la peticio NO es per un usuari d'aquest domini
                        char IP_d2[16];
                        int portPeticio;

                        portPeticio = DEFAULT_PORT;

                        if(DNSc_ResolDNSaIP(nomDomini_2, IP_d2) == -1) {
                            missatgeResposta[2] = '2';
                        }
                        else if(LUMIs_DemanaLocalitzacio(sckUDP, IP_d2, portPeticio, adrMI_1, adrMI_2, fitxLog) == -1) {
                            perror("Error en demanar localització al client 2\n");
                            exit(-1);
                        }
                    }

                    if(missatgeResposta[2] != '0') {
                        if(LUMIs_Resposta(sckUDP, missatgeResposta, longMissatge, IPrem, portUDPrem, fitxLog) == -1) {
                            perror("Error enviar resposta\n");
                            exit(-1);
                        }
                    }
                //}
            }
            else if(tipus == 4) { // Resposta Localització (RL)
                int i, longMissatge;
                char missatgeResposta[150];

                char IPresposta[16];
                int portResposta;

                char adrMI_1[100];
                char nomUsuari_1[100];
                char nomDomini_1[100];

                longMissatge = sprintf(missatgeResposta, "RL%s", seq);

                sscanf(seq+1, "%[^:]", adrMI_1);
                sscanf(adrMI_1, "%[^@]@%s", nomUsuari_1, nomDomini_1);

                if(strcmp(nomDomini, nomDomini_1) == 0) { // Si la petició venia d'un usuari d'aquest domini
                    i = cercaUsuari(nomUsuari_1);

                    strcpy(IPresposta, taula[i].adr_LUMI->ip);
                    portResposta = taula[i].adr_LUMI->port;
                }
                else { // Si la peticio NO venia d'un usuari d'aquest domini, sinó d'un altre node
                    DNSc_ResolDNSaIP(nomDomini_1, IPresposta);
                    portResposta = DEFAULT_PORT;
                }

                LUMIs_Resposta(sckUDP, missatgeResposta, longMissatge, IPresposta, portResposta, fitxLog);
            }
        }
        else{

            char seq[2];
            int long_seq;

            if( (long_seq = read(0, seq, sizeof(seq))) == -1 ) {
                perror("Error en read");
                exit(-1);
            }
            seq[1] = '\0';

            if(strcmp(seq,"1") == 0)listUsers();
            else if(strcmp(seq,"2") == 0)addUser();
            else if(strcmp(seq,"3") == 0)removeUser();

            printf("---------------------- ADMINISTRATOR MENU -----------------------------\n");
            printf("1. List users\n");
            printf("2. Add user\n");
            printf("3. Remove user\n");
            
        }
    }

    LUMIs_AcabaServidor(fitxLog);

    return 0;
 }

void listUsers(){
    int i;

    for(i=0;i<N_USUARIS;i++){
        if(!taula[i].adr_LUMI)printf("OFFILNE %s\n",taula[i].adr_MI);
        else printf("%s %s:%d\n",taula[i].adr_MI,taula[i].adr_LUMI->ip,taula[i].adr_LUMI->port);
    }
}

void addUser(){
    char nomUsuari[100];
    struct pair_MI_status element;
    printf("Enter new user name:");
    scanf("%s",nomUsuari);
    
    strcpy(element.adr_MI, nomUsuari);
    element.adr_LUMI = NULL;

    inserirOrdenat(element);
    saveTableToFile();
    printf("User %s added correctly\n",nomUsuari);
}

void saveTableToFile(){
    FILE *fp;
    int i;

    fp = fopen("./test.c", "w+");
    
    fprintf(fp, "%s\n",nomDomini);
    for(i=0;i<N_USUARIS;i++)fprintf(fp, "%s\n",taula[i].adr_MI);
    fclose(fp);
}

void removeUser(){

    char *nomUsuari = NULL;
    printf("Enter user name to remove:");
    scanf("%s",nomUsuari);

    int i = cercaUsuari(nomUsuari);
    if(i == -1)printf("Error: User not found\n");
    else{
        FILE *fp;
        fp = fopen("./test.c", "w+");
        for(i=i;i<N_USUARIS-1;i++){
            taula[i] = taula[i+1];
            fprintf(fp, "%s\n",taula[i].adr_MI);
        }
        fclose(fp);
        printf("User %s deleted correctly\n",nomUsuari);
    }
}

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Si l'usuari no existeix a la taula, retorna -1. Altrament retorna la   */
/* seva posició (índex) en el rang [0 , SIZE_TABLE-1]                     */
int cercaUsuari(const char *nomUsuari) {
    int pos = i_cercaUsuari(nomUsuari, 0, N_USUARIS-1);

    if( strcmp(taula[pos].adr_MI, nomUsuari) != 0) pos = -1;

    return pos;
}

/* Retorna la seva posició (índex) en el rang [0 , SIZE_TABLE-1]          */
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

void inserirOrdenat(const struct pair_MI_status e) {
    int pos = i_cercaUsuari(e.adr_MI, 0, N_USUARIS-1);
    int i;

    for(i=N_USUARIS; i>pos; i--) {
        strcpy(taula[i].adr_MI, taula[i-1].adr_MI);
        taula[i].adr_LUMI = taula[i-1].adr_LUMI;
    }

    N_USUARIS++;

    strcpy(taula[pos].adr_MI, e.adr_MI);
    taula[pos].adr_LUMI = e.adr_LUMI;
}
