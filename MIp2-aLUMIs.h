/**************************************************************************/
/*                                                                        */
/* P2 - MI amb sockets TCP/IP - Part II                                   */
/* Fitxer capçalera de MIp2-aLUMIc.c                                      */
/*                                                                        */
/* Autors: Joan Plaja, Martí Mas                                          */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de aLUMIs.c, és a dir, d'aquelles      */
/* funcions que es faran servir en un altre fitxer extern a aLUMIs.c,     */
/* p.e., int LUMIs_FuncioExterna(arg1, arg2...) { }                       */
/* El fitxer extern farà un #include del fitxer aLUMIs.h a l'inici, i     */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES formen la interfície de la capa LUMI, en la part servidora.   */

#define MAX_ATTEMPTS 10 // Maxim d'intents per enviar un paquet
#define TIMEOUT 500     // 500ms de timeout per rebre resposta de servidor a una peticio
#define DEFAULT_PORT 3344
#define SIZE_TABLE 100

struct pair_address {
    char ip[16];
    int port;
};
struct pair_MI_status {
    char adr_MI[100];
    struct pair_address *adr_LUMI; // NULL = offline
};

char nomDomini[100];
char SeqBytesAnterior[100];
int nPeticionsIguals;
struct pair_MI_status taula[SIZE_TABLE];

int LUMIs_IniciaServidor(int *fitxLog);
int LUMIs_ServeixPeticio(int sckNodeLUMI, char *SeqBytes, int *LongSeqBytes, char *IPrem, int *portUDPrem, int fitxLog);
int LUMIs_Resposta(int sckNodeLUMI, const char *resposta, int longResposta, const char *IPrem, int portUDPrem, int fitxLog);
int LUMIs_DemanaLocalitzacio(int sckNodeLUMI, const char *IPrem, int portUDPrem, const char *adrMI_1, const char *adrMI_2, int fitxLog);
void LUMIs_AcabaServidor(int fitxLog);
const char* LUMIs_ObteMissError(void);