#define NUMCLIENTES 32
#define ACCESO 1
#define LEN_FUNCTION_NAME 11 // insercion : 9 caracteres + \0 + unicode
#define LOGFILENAME "serverDogs.log"
#define USAGE "USAGE\n\t%s\n", argv[0]

void menu();
void *hilosfuncion(void *ap);
void servidor();
void ingresar(int clientfd, char* registroCadena);
void ver(int clientfd, char* registroCadena);
void borrar(int clientfd, char* registroCadena);
void buscar(int clientfd, char* registroCadena);
ulong hash(ulong key);  // In : key ; Out : id
ulong new_hash(char* nombre);       // Out : key
void ir_en_linea(FILE* archivo, ulong linea);
void sizemasmas();

typedef struct threadArg_s {
	int *clientfd;
	struct sockaddr_in *client;
	bool listo;
} threadArg;
