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
void salir(int exitcode);

typedef struct threadArg_s {
	int *clientfd;
	struct sockaddr_in *client;
} threadArg;
