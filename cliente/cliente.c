#include "p1-dogProgram.h"
#include "cliente.h"
struct winsize window;

int main(){
	menu();
	return EXIT_SUCCESS;
}

void menu() {
  char p, buf[BUFSIZ];
  struct termios termios_p_raw, termios_p_def;
  tcgetattr(STDIN_FILENO, &termios_p_def);
  tcgetattr(STDIN_FILENO, &termios_p_raw);
  termios_p_raw.c_iflag &=
      ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  termios_p_raw.c_oflag &= ~OPOST;
  termios_p_raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  termios_p_raw.c_cflag &= ~(CSIZE | PARENB);
  termios_p_raw.c_cflag |= CS8;

  int r, clientfd;
  struct sockaddr_in client;
  socklen_t len;
  
  client.sin_family = AF_INET;
  client.sin_port = htons(PORT);
  client.sin_addr.s_addr = inet_addr("127.0.0.1");
  bzero(client.sin_zero, 8);
  len = sizeof(struct sockaddr_in);

  while (1) {
	  clientfd = socket(AF_INET, SOCK_STREAM, 0);
	  //validar, clientfd == -1
    p = 0;
    printf(
        "1. Ingresar registro\n2. Ver registro\n3. Borrar registro\n4. "
        "Buscar\n5. Salir\n");
    while (p < '1' || p > '5') {
      p = getc(stdin);
      // p = '5';
    }

    if (p > '0' && p < '5') {
	    r = connect(clientfd, (const struct sockaddr*) &client, len);
	    //error
	    ERROR(r == -1, perror("connect"););
	    DEBUG("p = %c, r = %d, clientfd = %d\n", p, r, clientfd);
	    r = send(clientfd, &p, sizeof(char), 0);
	    //error?
	    ERROR(r == -1, perror("send"););
    }
	    
    switch (p) {  // check si p es 1, 2, 3, 4 o 5
      case '1':
        ingresar(clientfd);
        break;
      case '2':
        ver(clientfd);
        break;
      case '3':
        borrar(clientfd);
        break;
      case '4':
	      buscar(clientfd, termios_p_raw, termios_p_def, buf);
        break;
      case '5':
        salir(EXIT_SUCCESS);
        break;
      default:
        ERROR(1, perror("getc"));
    }
    close(clientfd);
    printf("Cualquier tecla.\n");
    fflush(stdin);
    tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_raw);
    //setbuf(stdin, NULL);
    getc(stdin);
    //setbuf(stdin, buf);
    tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_def);
  }
}

void salir(int exitcode) {
  exit(exitcode);
}
void ingresar(int clientfd) {
  int r;
  dogType* new = (dogType*)malloc(sizeof(dogType));
  ulong key;
  printf("Cual es el nombre de la mascota?\n");
  r = scanf("%s", new->nombre);
  ERROR(r == 0, perror("scanf"); free(new));
  printf("Cual es su tipo?\n");
  r = scanf("%s", new->tipo);
  ERROR(r == 0, perror("scanf"); free(new));
  printf("Cual es su edad?\n");
  r = scanf("%lu", &new->edad);
  ERROR(r == 0, perror("scanf"); free(new));
  printf("Cual es su raza?\n");
  r = scanf("%s", new->raza);
  ERROR(r == 0, perror("scanf"); free(new));
  printf("Cual es su estatura (en cm)?\n");
  r = scanf("%lu", &new->estatura);
  ERROR(r == 0, perror("scanf"); free(new));
  printf("Cual es su peso (en Kg)?\n");
  r = scanf("%lf", &new->peso);
  ERROR(r == 0, perror("scanf"); free(new));
  printf("Cual es su sexo?\n");
  getchar();
  r = scanf("%c", &new->sexo);
  ERROR(r == 0, perror("scanf"); free(new));

  r = send(clientfd, new, sizeof(dogType), 0);
  // error
  free(new);
  r = recv(clientfd, &key, sizeof(ulong), 0);
  printf("Key : %lu\n", key);
}

void ver(int clientfd) {
	bool test;
	int r;
	struct stat statBefore, statAfter;
	long i;
  ulong key;
  dogType* mascota;
  FILE* archivo;
  char l, command[] = TEXT_EDITOR" .tmp.txt", filename[] = ".tmp.txt", // linux -> .* = archivo oculto
	  bufferArchivo[STRING_BUFFER], *s;  // 32 + 32 + 20 + 16 + (20 + 13 = 33) + 8
                             // (malo/femenino) + (22 + 5*7 = 22 + 35 = 57)
  /*
    Nombre : 10
    Tipo : 7
    Edad : 7
    Raza : 7
    Estatura : 12
    Peso : 7
    Sexo : 7
  */
  r = recv(clientfd, &i, sizeof(long), 0);
  //error
  printf("Hay %ld numeros presentes.\nCual es la clave de la mascota?\n",
         i);
  r = scanf("%ld", &key);
  ERROR(r == 0, perror("scanf"));
  r = send(clientfd, &key, sizeof(ulong), 0);
  //error
  r = recv(clientfd, &test, sizeof(bool), 0);
  //error
  if (!test) {
    return;
  }
  mascota = (dogType*)malloc(sizeof(dogType));
  recv(clientfd, mascota, sizeof(dogType), 0);
  //error
  printf(
      "Nombre : %s\nTipo : %s\nEdad : %ld\nRaza : %s\nEstatura : %ld\nPeso : "
      "%.12lf\nSexo : %c\n",
      mascota->nombre, mascota->tipo, mascota->edad, mascota->raza,
      mascota->estatura, mascota->peso, mascota->sexo);

  // existe la historia clinica?

  printf("Quiere abrir la historia clinica de %s? [S/n]\n", mascota->nombre);
  l = 0;
  while (l != 'S' && l != 's' && l != 'N' && l != 'n') {  // [S/N]
    r = scanf("%c", &l);
    ERROR(r == 0, perror("scanf"); free(mascota));
  }
  send(clientfd, &l, sizeof(char), 0);
  if (l == 'S' || l == 's') {  // abrir historia clinica
	  //pid = fork();
	  //ERROR(pid == -1, perror("can't fork"));
	  //if (pid == 0) {  // hijo
	  //recibir command2
	  archivo = fopen(filename, "w");
	  for(r = recv(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0); bufferArchivo[0] != EOF; r = recv(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0)) {
		  //error
		  r = fputs(bufferArchivo, archivo);
		  //error
		  DEBUG("ver -> recv -> loop -> r=%d, buffer=\"%s\" (buffer[0] = '%c' = %d)", r, bufferArchivo, bufferArchivo[0], bufferArchivo[0]);
	  }
	  DEBUG("ver -> recv -> out of loop -> r=%d, buffer=\"%s\"", r, bufferArchivo);
	  fclose(archivo);
	  r = stat(filename, &statBefore);
	  //error
	  DEBUG("ver -> antes de system");
	  system(command);
	  DEBUG("ver -> despues de system");

	  r = stat(filename, &statAfter);
	  test = statBefore.st_mtim.tv_sec != statAfter.st_mtim.tv_sec;
	  DEBUG("ver -> test=%d (%ld == %ld)", test, statBefore.st_mtim.tv_sec, statAfter.st_mtim.tv_sec);
	  //enviar bool si tenemos que enviar command2?
	  r = send(clientfd, &test, sizeof(bool), 0);
	  if (test) {
		  archivo = fopen(filename, "r");
		  for(s = fgets(bufferArchivo, STRING_BUFFER, archivo); bufferArchivo[0] != EOF && s != NULL ; s = fgets(bufferArchivo, STRING_BUFFER, archivo)) {
			  ERROR(s == NULL, perror("ver -> fgets"););
			  r = send(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0);
		  }
		  bufferArchivo[0] = EOF;
		  r = send(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0); // EOF
		  fclose(archivo);
	  }
	  r = remove(filename);
	  //error
	  
	  //salir(EXIT_SUCCESS);
  }
  //}
  free(mascota);
}

void borrar(int clientfd) {
  int r;
  long i;
  ulong key;
  bool test;
  
  r = recv(clientfd, &i, sizeof(long), 0);
  //error
  printf("Hay %ld numeros presentes.\nCual es la clave de la mascota?\n",
         i);
  r = scanf("%ld", &key);
  ERROR(r == 0, perror("scanf"));
  r = send(clientfd, &key, sizeof(ulong), 0);
  //error
  r = recv(clientfd, &test, sizeof(bool), 0);
  //error
  if (test) {
	  printf("La mascota fue borrada.\n");
  } else {
	  printf("La mascota no existe.\n");
  }
}

void buscar(int clientfd, struct termios termios_p_raw,
            struct termios termios_p_def,
            char buf[]) {
  int r;
  ulong k = 0, lineas, key;
  char buffer_u[SIZE_GRANDE];
  bool test = true;
  printf("Cual es el nombre de la mascota?\n");
  r = scanf("%s", buffer_u);
  ERROR(r == 0, perror("scanf"));
  r = send(clientfd, buffer_u, sizeof(char) * SIZE_GRANDE, 0);

  fflush(stdin);
  tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_raw);  // set term to raw
  //setbuf(stdin, NULL);
  lineas = (window.ws_row - 1);  // leer las lineas del term
  DEBUG("lineas = %lu", lineas);

  while (test) {
	  r = recv(clientfd, &key, sizeof(ulong), 0);
	  //error
	  if(key == 0) { // fin
		  test = false;
	  } else {
		  r = recv(clientfd, buffer_u, sizeof(char) * SIZE_GRANDE, 0);
		  printf("key : %ld - nombre : %s\n\r", key, buffer_u);
		  k++;
		  if (k == lineas) {
			  printf("Continuar - cualquier tecla ; Salir - q");
			  r = getc(stdin);
			  printf("\n\r");
			  if (r == 'q') {
				  test = false;
				  r = send(clientfd, &test, sizeof(bool), 0);
				  //error
				  //setbuf(stdin, buf);  // set buffer
				  tcsetattr(STDIN_FILENO, TCSANOW,
				            &termios_p_def);  // set back term to default
				  return;
			  }
			  k = 0;
		  }
		  r = send(clientfd, &test, sizeof(bool), 0);
		  //DEBUG("buscar -> recibido : %lu - %s\nenviado : %d", key, buffer_u, test);
	  }
  }
  //setbuf(stdin, buf);
  tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_def);
}
