#define NUMCLIENTES 32
#define ACCESO 1
#define LEN_FUNCTION_NAME 11 // insercion : 9 caracteres + \0 + unicode
#define LOGFILENAME "serverDogs.log"
#define USAGE "USAGE\n\t%s s/m/t\nDESCRIPTION\n\tSemaforo / Mutex / Tuberias\n", argv[0]

void menu();

void *hilosfuncionSemaforo(void *ap);
void *hilosfuncionMutex   (void *ap);                            
void *hilosfuncionTuberias(void *ap);                            

void servidorSemaforo();                                         
void servidorMutex   ();                                         
void servidorTuberias();                                         

void ingresarSemaforo(int clientfd, char* registroCadena, int n);
void ingresarMutex   (int clientfd, char* registroCadena, int n);
void ingresarTuberias(int clientfd, char* registroCadena, int n);

void verSemaforo(int clientfd, char* registroCadena, int n);     
void verMutex   (int clientfd, char* registroCadena, int n);     
void verTuberias(int clientfd, char* registroCadena, int n);     

void borrarSemaforo(int clientfd, char* registroCadena, int n);  
void borrarMutex   (int clientfd, char* registroCadena, int n);  
void borrarTuberias(int clientfd, char* registroCadena, int n);  

void buscarSemaforo(int clientfd, char* registroCadena, int n);  
void buscarMutex   (int clientfd, char* registroCadena, int n);  
void buscarTuberias(int clientfd, char* registroCadena, int n);  

ulong hash(ulong key);  // In : key ; Out : id
ulong new_hash(char* nombre);       // Out : key
void ir_en_linea(FILE* archivo, ulong linea);
void sizemasmas();

typedef struct threadArg_s {
	int *clientfd;
	struct sockaddr_in *client;
	bool listo;
	int *numero;
} threadArg;
