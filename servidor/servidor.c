#include "p1-dogProgram.h"

tabla hash_table;  // 76MB
sprimos primos;    // 5MB

int main(int argc, char* argv[]) {
  int r;
  uint i;
  dogType buffer;
  ulong key = 0, s = 1, n;
  FILE* archivo;

  setbuf(stdout, NULL);

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

  archivo = fopen(PRIMOS, "r");
  ERROR(archivo == NULL,
        fprintf(stderr, "Error : No se puede leer %s\n", PRIMOS))

  // contar cuantos numeros hay

  r = fseek(archivo, 0, SEEK_END);
  ERROR(r == -1, perror("fseek"));

  primos.size = ftell(archivo) / sizeof(ulong);

  // leer los primos

  r = fseek(archivo, 0, SEEK_SET);
  ERROR(r == -1, perror("fseek"));

  primos.primos = (ulong*)malloc(sizeof(ulong) * primos.size);

  for (i = 0; i < primos.size; i++) {
    r = fread(primos.primos + i, sizeof(ulong), 1, archivo);
  }

  fclose(archivo);

  // hash table

  hash_table.numero_de_datos = 0;
  hash_table.last_key = 0;

  archivo = fopen(ARCHIVO, "r");
  if (archivo == 0) {  // file doesn't exist
    archivo = fopen(ARCHIVO, "w");
    fclose(archivo);

    for (i = 0; primos.primos[i] < FIRST_SIZE; i++)
      ;
    primos.cur = i;
    hash_table.id = (ulong*)calloc(primos.primos[primos.cur], sizeof(ulong));
    hash_table.size = primos.primos[primos.cur];
  } else {  // file exists
    // contar cuantos datos hay
    r = fseek(archivo, 0, SEEK_END);
    if (r == -1) {
      perror("fseek");
      return (EXIT_FAILURE);
    }

    n = ftell(archivo) / (sizeof(ulong) + sizeof(dogType));
    for (i = 0; i < primos.size && primos.primos[i] < n; i++)
      ;
    ERROR(i == primos.size,
          fprintf(
              stderr,
              "Error : hay %lu datos pero el primo maximo que tenemos es %lu\n",
              n, primos.primos[i - 1]));

    primos.cur = i;

    hash_table.id = (ulong*)calloc(primos.primos[primos.cur], sizeof(ulong));
    hash_table.size = primos.primos[primos.cur];

    r = fseek(archivo, 0, SEEK_SET);
    ERROR(r == -1, perror("fseek"));

    // ingresar datos

    s = fread(&key, sizeof(ulong), 1, archivo);
    for (i = 0; s != 0; i++) {  // reading the file
      hash_table.id[i] = key;
      if (key != 0) {
        hash_table.numero_de_datos++;
      }
      if(key > hash_table.last_key){
	      hash_table.last_key = key;
      }
      s = fread(&buffer, sizeof(dogType), 1, archivo);
      s = fread(&key, sizeof(ulong), 1, archivo);
    }
    fclose(archivo);
  }
  //test_hash();

  servidor();
  return EXIT_SUCCESS;
}

void *hilosfuncion(void *ap){
	int clientfd = (int) ap[0], r, buffer;
	r = recv(clientfd, &buffer, 1, 0);
	//error
	switch(buffer){
	case 1:
		ingresar(clientfd);
		break;
	case 2:
		ver(clientfd);
		break;
	case 3:
		borrar(clientfd);
		break;
	case 4:
		buscar(clientfd);
		break;
	default:
		ERROR(1, fprintf(stderr, "Cliente no envia el buen mensaje"));
	}
}

int hilos(){ // gcc -lpthread
	pthread_t tfd[NUMHILOS];
	int i, data[NUMHILOS];
	
	for(i = 0; i < NUMHILOS; i++){
		data[i] = i;
		pthread_create(&tfd[i], NULL, funcion, data+i);
	}
	for(i = 0; i < NUMHILOS; i++){
		pthread_join(tfd[i], NULL);
	}
	return EXIT_SUCCESS;
}


void servidor(){
	int r, serverfd, clientfd, on=1;
	struct sockaddr_in server, client;
	socklen_t len;
	pthread_t tfd[NUMHILOS];
	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	ERROR(serverfd == -1,
	      perror("socket");
	      exit(EXIT_FAILURE));
	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT); // big endian / little endian
	server.sin_addr.s_addr = INADDR_ANY;
	bzero((server.sin_zero), 8);
	len = sizeof(struct sockaddr_in);

	r = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	ERROR(r == -1,
	      perror("setsockopt"););

	r = bind(serverfd, (struct sockaddr*) &server, len);
	ERROR(r == -1,
	      perror("bind"););
	while (1) {
	
		r = listen(serverfd, BACKLOG);
		ERROR(r == -1,
		      perror("listen"););
		
		clientfd = accept(serverfd, (struct sockaddr*) &client, &len);
		ERROR(clientfd == -1,
		      perror("accept"););
	}
	if(argc > 1){
		r = send(clientfd, argv[1], sizeof(argv[1]), 0);
	}
	else{
		r = send(clientfd, "hola", 4, 0);
	}
	ERROR(r == -1,
	      perror("send"););
	
	close(clientfd);
	close(serverfd);
	return EXIT_SUCCESS;

}

void ingresar() {
  int r;
  dogType* new = (dogType*)malloc(sizeof(dogType));
  ulong key, id;
  FILE* archivo;
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

  key = new_hash(new->nombre);
  id = hash(key);
  if (hash_table.id[id] == key) {
    printf("Key : %lu\n", key);
  } else {
    ERROR(1, fprintf(stderr, "Error in new_hash\n"); free(new));
  }

  /*
    Escribir en un archivo
   */
  archivo = fopen(ARCHIVO, "r+");
  ERROR(archivo == 0, perror("fopen"); free(new));
  ir_en_linea(archivo, id);
  r = fwrite(&key, sizeof(ulong), 1, archivo);
  ERROR(r == 0, perror("fwrite"); free(new); fclose(archivo));
  fwrite(new, sizeof(dogType), 1, archivo);
  fclose(archivo);

  free(new);
}

void ver() {
  int r, buffersize;
  ulong key, id, key2;
  dogType* mascota;
  FILE* archivo;
  pid_t pid;
  char l, command1[] = TEXT_EDITOR, command2[46] = "",
          buffer[211] = "";  // 32 + 32 + 20 + 16 + (20 + 13 = 33) + 8
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

  printf("Hay %ld numeros presentes.\nCual es la clave de la mascota?\n",
         hash_table.numero_de_datos);
  r = scanf("%ld", &key);
  ERROR(r == 0, perror("scanf"));
  id = hash(key);
  if (hash_table.id[id] != key) {
    return;
  }
  mascota = (dogType*)malloc(sizeof(dogType));
  archivo = fopen(ARCHIVO, "r");
  ir_en_linea(archivo, id);
  r = fread(&key2, sizeof(ulong), 1, archivo);
  ERROR(r == 0, perror("fread"); free(mascota); fclose(archivo));
  if (key != key2) {
    printf("/!\\ key (%ld) != key2 (%ld)\n", key, key2);
    return;
  }
  r = fread(mascota, sizeof(dogType), 1, archivo);
  ERROR(r == 0, perror("fread"); free(mascota); fclose(archivo));
  fclose(archivo);
  printf(
      "Nombre : %s\nTipo : %s\nEdad : %ld\nRaza : %s\nEstatura : %ld\nPeso : "
      "%.12lf\nSexo : %c\n",
      mascota->nombre, mascota->tipo, mascota->edad, mascota->raza,
      mascota->estatura, mascota->peso, mascota->sexo);

  // existe la historia clinica?

  r = sprintf(command2, "historias_clinicas/%lu_hc.txt", key);  // 19 + 20 + 7
  ERROR(r < 0, perror("sprintf"); free(mascota));

  archivo = fopen(command2, "r");

  if (archivo == NULL) {  // tenemos que crearla

    archivo = fopen(command2, "w");
    if (archivo == NULL) {
      r = mkdir("historias_clinicas", 0755);
      ERROR(r == -1, perror("mkdir"));
      archivo = fopen(command2, "w");
    }
    char malo[] = "malo", femenino[] = "femenino", otro[] = "otro", *sexo;
    if (mascota->sexo == 'm') {
      sexo = malo;
    } else if (mascota->sexo == 'f') {
      sexo = femenino;
    } else {
      sexo = otro;
    }
    buffersize = sprintf(buffer,
                         "Nombre : %s\nTipo : %s\nEdad : %ld\nRaza : "
                         "%s\nEstatura : %ld\nPeso : %.12lf\nSexo : %s\n",
                         mascota->nombre, mascota->tipo, mascota->edad,
                         mascota->raza, mascota->estatura, mascota->peso, sexo);
    r = 0;
    while (r < buffersize) {
      r += fwrite(&buffer[r], sizeof(char), buffersize, archivo);
    }
  }
  fclose(archivo);
  // existe

  printf("Quiere abrir la historia clinica de %s? [S/N]\n", mascota->nombre);
  l = 0;
  while (l != 'S' && l != 's' && l != 'N' && l != 'n') {  // [S/N]
    r = scanf("%c", &l);
    ERROR(r == 0, perror("scanf"); free(mascota));
  }
  if (l == 'S' || l == 's') {  // abrir historia clinica
    pid = fork();
    ERROR(pid == -1, perror("can't fork"));
    if (pid == 0) {  // hijo
      char env1[32], env2[32], env3[64];

      char
          /*
           *caml    = getenv("CAML_LD_LIBRARY_PATH"),
           *dbus    = getenv("DBUS_SESSION_BUS_ADDRESS"),
           */
          *display = getenv("DISPLAY"),  // para X11
          /*
           *editor  = getenv("EDITOR"),
           *home    = getenv("HOME"),
           *invocid = getenv("INVOCATION_ID"),
           *journ   = getenv("JOURNAL_STREAM"),
           *lang    = getenv("LANG"),
           *less    = getenv("LESSOPEN"),
           *logn    = getenv("LOGNAME"),
           *mail    = getenv("MAIL"),
           *moz     = getenv("MOZ_PLUGIN_PATH"),
           *ocaml   = getenv("OCAML_TOPLEVEL_PATH"),
           *oldpwd  = getenv("OLDPWD"),
           *opam    = getenv("OPAM_SWITCH_PREFIX "),
           *path    = getenv("PATH"),
           *prompt  = getenv("PROMPT"),
           *pwd     = getenv("PWD"), // para abrir
           *shell   = getenv("SHELL"),
           *shlvl   = getenv("SHLVL"),
           */
          *term = getenv("TERM"),             // para consola
                                              /*
                                               *tmux0   = getenv("TMUX"),
                                               *tmux1   = getenv("TMUX_PANE"),
                                               *user    = getenv("USER"),
                                               *winid   = getenv("WINDOWID"),
                                               */
              *xauth = getenv("XAUTHORITY");  // para X11
                                              /*
                                               *xdg0    = getenv("XDG_RUNTIME_DIR"),
                                               *xdg1    = getenv("XDG_SEAT"),
                                               *xdg2    = getenv("XDG_SESSION_CLASS"),
                                               *xdg3    = getenv("XDG_SESSION_ID"),
                                               *xdg4    = getenv("XDG_SESSION_TYPE"),
                                               *xdg5    = getenv("XDG_VTNR"),
                                               *manp    = getenv("MANPATH");
                                               */

      sprintf(env1, "TERM=%s", term);
      sprintf(env2, "DISPLAY=%s", display);
      sprintf(env3, "XAUTHORITY=%s", xauth);  // !!! > 32
      char *argv[3], *envp[] = {env1, env2, env3, 0};
      argv[0] = command1;
      argv[1] = command2;
      argv[2] = 0;
      execve(command1, argv, envp);  // xdg-open archivo.txt
      salir(EXIT_SUCCESS);
    }
  }
  free(mascota);
}

void borrar() {
  int r;
  ulong key, id;
  FILE* archivo;
  printf("Hay %ld numeros presentes.\nCual es la clave de la mascota?\n",
         hash_table.numero_de_datos);
  r = scanf("%ld", &key);
  ERROR(r == 0, perror("scanf"));
  id = hash(key);
  if (hash_table.id[id] == 0) {
    return;
  }
  archivo = fopen(ARCHIVO, "r+");
  ir_en_linea(archivo, id);
  hash_table.id[id] = 0;
  hash_table.numero_de_datos--;
  key = 0;
  r = fwrite(&key, sizeof(ulong), 1, archivo);
  ERROR(r == 0, perror("fwrite"); fclose(archivo));
  fclose(archivo);
}

void buscar(struct termios termios_p_raw,
            struct termios termios_p_def,
            char buf[]) {
  int r;
  ulong i, j, k = 0, lineas;
  char buffer_u[SIZE_GRANDE], buffer_d[SIZE_GRANDE];
  FILE* archivo;
  printf("Cual es el nombre de la mascota?\n");
  r = scanf("%s", buffer_u);
  ERROR(r == 0, perror("scanf"));
  for (i = 0; i < SIZE_GRANDE; i++) {
    if (buffer_u[i] >= 'A' && buffer_u[i] <= 'Z') {
      buffer_u[i] += 32;
    }
  }
  archivo = fopen(ARCHIVO, "r");
  ERROR(archivo == NULL, perror("fopen"));

  fflush(stdin);
  tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_raw);  // set term to raw
  setbuf(stdin, NULL);
  lineas = (window.ws_row - 1);  // leer las lineas del term

  for (i = 0; i < hash_table.size; i++) {
	  if (hash_table.id[i] != 0) {
      fseek(archivo, sizeof(ulong), SEEK_CUR);
      r = fread(buffer_d, sizeof(char), SIZE_GRANDE, archivo);
      fseek(archivo, sizeof(dogType) - 32 * sizeof(char), SEEK_CUR);
      for (j = 0; j < SIZE_GRANDE; j++) {
        if (buffer_d[j] >= 'A' && buffer_d[j] <= 'Z') {
          buffer_d[j] += ('a' - 'A');
        }
      }
      for (j = 0; j < SIZE_GRANDE && buffer_u[j] != 0 && buffer_d[j] != 0 &&
                  buffer_u[j] == buffer_d[j];
           j++) {
      }
      if (buffer_u[j] == '\0') {
        printf("key : %ld - nombre : %s\n\r", hash_table.id[i], buffer_d);
        k++;
        if (k == lineas) {
          printf("Continuar - cualquier tecla ; Salir - q");
          r = getc(stdin);
          printf("\n\r");
          if (r == 'q') {
            setbuf(stdin, buf);  // set buffer
            tcsetattr(STDIN_FILENO, TCSANOW,
                      &termios_p_def);  // set back term to default
            fclose(archivo);
            return;
          }
          k = 0;
        }
      }
    }
  }
  setbuf(stdin, buf);
  tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_def);
  fclose(archivo);
}

// returns id ; hash_table[id] == key dice si existe
ulong hash(ulong key) {
  ulong id = key % hash_table.size, id1 = id;
  while (hash_table.id[id] != key) {
    id = (id + key) % hash_table.size;
    if (id == id1) {  // no hay el id en la tabla
      return 0;
    }
  }
  return id;
}

ulong new_hash(char* nombre) {
  if (hash_table.size == hash_table.numero_de_datos) {
    sizemasmas();  // hacer con numeros primos
  }
  hash_table.numero_de_datos++;
  hash_table.last_key++;
  for(;hash_table.last_key % hash_table.size == 0;hash_table.last_key++);
  ulong key = hash_table.last_key;
  ulong id = key % hash_table.size;
  while (hash_table.id[id] != 0) {
    id = (id + key) % hash_table.size;
  }

  hash_table.id[id] = key;
  return key;
}

void ir_en_linea(FILE* archivo, ulong linea) {
  ulong i = 0;
  int r = 1;
  ulong tmp;
  dogType buffer;
  while (r != 0 && i < linea) {
    r = fread(&tmp, sizeof(ulong), 1, archivo);
    r = fread(&buffer, sizeof(dogType), 1, archivo);
    if(r > 0) {
	    i++;
    }
  }
  tmp=0;
  while(i < linea){
	  r = fwrite(&tmp, sizeof(ulong), 1, archivo);
	  r = fwrite(&buffer, sizeof(dogType), 1, archivo);
	  i++;
  }
}

/*
hash = datos1
tmp = datos1
hash = datos2
datos1 -> datos2
free datos1
 */
void sizemasmas() {
  uint i;
  tabla tmp;
  tmp.size = hash_table.size;
  tmp.id = hash_table.id;

  primos.cur++;

  hash_table.size = primos.primos[primos.cur];
  hash_table.id = (ulong*)calloc(hash_table.size, sizeof(ulong));
  ERROR(hash_table.id == NULL, perror("calloc"); free(tmp.id));
  for (i = 0; i < tmp.size; i++) {
    hash_table.id[i] = tmp.id[i];
  }
  free(tmp.id);
}
