#include "p1-dogProgram.h"
#include "servidor.h"

tabla hash_table;  // 76MB
sprimos primos;    // 5MB
sem_t *accesoADatos;
sem_t *clientes;

int main(int argc, char* argv[]) {
  int r;
  uint i;
  dogType buffer;
  ulong key = 0, s = 1, n;
  FILE* archivo;
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
	DEBUG("hilosfuncion()");
	threadArg arg = *(threadArg*)ap;
	int clientfd = *(arg.clientfd), r;
	char p;
	struct sockaddr_in client = *(arg.client);
	((threadArg*)ap)->listo = true;
	/*
	char clntName[INET_ADDRSTRLEN];
	r = inet_ntop(AF_INET,client.sin_addr.s_addr,clntName,sizeof(clntName));
	ERROR(r == NULL, perror("hilosfuncion -> inet_ntop"));
  DEBUG("hilosfuncion -> inet_ntop funciona");
	*/

	char *ip = inet_ntoa(client.sin_addr);
	FILE *logArchivo;
	/*
	  15 + 1 + 7 + 1 + 15 + 1 + LEN_FUNCTION_NAME + 1 + SIZE_GRANDE
	  = 41 + LEN_FUNCTION_NAME + SIZE_GRANDE
192.168.255.254
	 */
	char log[41 + LEN_FUNCTION_NAME + SIZE_GRANDE], funcion[LEN_FUNCTION_NAME],
		*registroCadena = (char*) malloc(SIZE_GRANDE * sizeof(char));
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	// printf("now: %d-%02d-%02d %02d:%02d:%02d\n" tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	r = recv(clientfd, &p, sizeof(char), 0);
	ERROR(r == -1, perror("hilo -> recv"););
	switch(p){
	case '1':
		DEBUG("inserción");
		sprintf(funcion, "inserción");
		ingresar(clientfd, registroCadena);
		break;
	case '2':
		DEBUG("lectura");
		sprintf(funcion, "lectura");
	  ver(clientfd, registroCadena);
		break;
	case '3':
		DEBUG("borrado");
		sprintf(funcion, "borrado");
		borrar(clientfd, registroCadena);
		break;
	case '4':
		DEBUG("búsqueda");
		sprintf(funcion, "búsqueda");
		buscar(clientfd, registroCadena);
		break;
	default:
		ERROR(1, fprintf(stderr, "Cliente no envia el buen mensaje\n"));
	}
	DEBUG("de nuevo en hilosfuncion()");
	sprintf(log, "%04d%02d%02dT%02d%02d%02d Cliente %s %s %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
	        //clntName,'/',ntohs(client.sin_port),
	        ip,
	        funcion,
	        registroCadena);
	logArchivo = fopen(LOGFILENAME, "a");
	r = fwrite(log, sizeof(log), 1, logArchivo);
	//error
	fclose(logArchivo);

	close(clientfd);
	free(registroCadena);
	DEBUG("hilosfuncion -> return");
	sem_post(clientes);
	return EXIT_SUCCESS;
}

void servidor(){
	DEBUG("servidor()");
	int r, serverfd, clientfd, on=1, i;
	struct sockaddr_in server, client;
	socklen_t len;
	pthread_t *tfd = (pthread_t*) calloc(NUMCLIENTES, sizeof(pthread_t));
	threadArg arg;
	// crear servidor
	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	ERROR(serverfd == -1,
	      perror("socket");
	      exit(EXIT_FAILURE));
	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT); // big endian / little endian
	server.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY;
	bzero((server.sin_zero), 8);
	len = sizeof(struct sockaddr_in);

	r = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	ERROR(r == -1,
	      perror("setsockopt"););

	r = bind(serverfd, (struct sockaddr*) &server, len);
	ERROR(r == -1,
	      perror("bind"););
	r = listen(serverfd, BACKLOG);
	ERROR(r == -1,
	      perror("listen"));
	// done
	i = 0;
	arg.clientfd = &clientfd;
	arg.client = &client;
	r = sem_unlink("clientes");
	r = sem_unlink("acceso a datos");
	clientes = sem_open("clientes", O_CREAT, 0700, NUMCLIENTES);
	accesoADatos = sem_open("acceso a datos", O_CREAT, 0700, ACCESO);
	//semaforo
	
	while (1) {
		DEBUG("servidor -> while");
		arg.listo=false;
		DEBUG("servidor -> while -> semaforo a %d/%d", *(int*)clientes, NUMCLIENTES);
		sem_wait(clientes);
		clientfd = accept(serverfd, (struct sockaddr*) &client, &len);
		DEBUG("servidor -> while clientfd accept!");
		ERROR(clientfd == -1,
		      perror("accept"));

		
		pthread_create(&tfd[i], NULL, hilosfuncion, &arg);
		DEBUG("servidor -> while -> pthread_create : done");
		i++;
		if(i > NUMCLIENTES){
			i = 0;
		}
		while(!arg.listo){
			sleep(0.0001);
		}
		
	}
	for (uint i = 0; i < NUMCLIENTES; i++){
		if(tfd [i] != 0){
			pthread_join(tfd[i], NULL);
		}
	}
	free(tfd);
	
	/*
	if(argc > 1){
		r = send(clientfd, argv[1], sizeof(argv[1]), 0);
	}
	else{
		r = send(clientfd, "hola", 4, 0);
	}
	ERROR(r == -1,
	      perror("send"););
	*/
	close(serverfd);
}

void ingresar(int clientfd, char* registroCadena) {
	DEBUG("ingresar()");
  int r;
  dogType* new = (dogType*)malloc(sizeof(dogType));
  ulong key, id;
  FILE* archivo;

  r = recv(clientfd, new, sizeof(dogType), 0);
  //error
  ERROR(r == -1, perror("ingresar -> recv dogtype"))
  DEBUG("recv -> r = %d", r);
  sem_wait(accesoADatos);
  DEBUG("sem_wait");
  
  key = new_hash(new->nombre);
  id = hash(key);
  ERROR(hash_table.id[id] != key, fprintf(stderr, "Error in new_hash\n"); free(new));
  DEBUG("key=%lu, id=%lu", key, id);

  /*
    Escribir en un archivo
   */
  archivo = fopen(ARCHIVO, "r+");
  ERROR(archivo == 0, perror("fopen"); free(new));
  DEBUG("tenemos el archivo\n");
  ir_en_linea(archivo, id);
  DEBUG("estamos en la linea %lu", id);
  r = fwrite(&key, sizeof(ulong), 1, archivo);
  ERROR(r == 0, perror("fwrite"); free(new); fclose(archivo));
  fwrite(new, sizeof(dogType), 1, archivo);
  fclose(archivo);

  sem_post(accesoADatos);

  r = send(clientfd, &key, sizeof(ulong), 0);

  sprintf(registroCadena, "%lu", key);

  free(new);
}

void ver(int clientfd, char* registroCadena) {
  int r, buffersize;
  ulong key, id, key2;
  bool test = true;
  dogType* mascota;
  FILE* archivo;
  char l, command2[46] = "",
	  buffer[211] = "", bufferArchivo[STRING_BUFFER], *s;  // 32 + 32 + 20 + 16 + (20 + 13 = 33) + 8
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

  r = send(clientfd, &hash_table.numero_de_datos, sizeof(long), 0);
  //error
  r = recv(clientfd, &key, sizeof(ulong), 0);
  ERROR(r == -1, perror("ver -> recv"));
  sprintf(registroCadena, "%lu", key);
  id = hash(key);
  if (hash_table.id[id] != key) {
	  test = false;
	  r = send(clientfd, &test, sizeof(bool), 0);
	  DEBUG("hash_table.id[%lu] = %lu != %lu = key\nr=%d", id, hash_table.id[id], key, r);
    return;
  }
  r = send(clientfd, &test, sizeof(bool), 0);

  sem_wait(accesoADatos);
  
  mascota = (dogType*)malloc(sizeof(dogType));
  archivo = fopen(ARCHIVO, "r");
  ir_en_linea(archivo, id);
  r = fread(&key2, sizeof(ulong), 1, archivo);
  ERROR(r == 0, perror("ver -> fread"); free(mascota); fclose(archivo));
  ERROR(key != key2, perror("ver -> key != key2");
        free(mascota); fclose(archivo));
  r = fread(mascota, sizeof(dogType), 1, archivo);
  ERROR(r == 0, perror("ver -> fread"); free(mascota); fclose(archivo));
  fclose(archivo);

  sem_post(accesoADatos);
  
  r = send(clientfd, mascota, sizeof(dogType), 0);

  r = recv(clientfd, &l, sizeof(char), 0);

  // semaforo?

  // existe la historia clinica?

  r = sprintf(command2, "historias_clinicas/%lu_hc.txt", key);  // 19 + 20 + 7
  ERROR(r < 0, perror("ver -> sprintf"); free(mascota));

  archivo = fopen(command2, "r");

  if (archivo == NULL) {  // tenemos que crearla

    archivo = fopen(command2, "w");
    if (archivo == NULL) {
      r = mkdir("historias_clinicas", 0755);
      ERROR(r == -1, perror("ver -> mkdir"));
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

  if (l == 'S' || l == 's') {  // abrir historia clinica
	  // enviar command2
	  archivo = fopen(command2, "r");
	  for(s = fgets(bufferArchivo, STRING_BUFFER, archivo); bufferArchivo[0] != EOF, s != NULL ; s = fgets(bufferArchivo, STRING_BUFFER, archivo)) {
		  //ERROR(s == NULL, perror("ver -> fgets"););
		  DEBUG("ver -> send -> s=%d, buffer=\"%s\"", s, bufferArchivo);
		  r = send(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0);
	  }
	  DEBUG("ver -> send -> out of loop -> s=%d, buffer=\"%s\"", s, bufferArchivo);
	  sprintf(bufferArchivo, "");
	  r = send(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0); // EOF
	  fclose(archivo);
	  // recibir bool si tenemos que recibir command2 ? recibir | no recibir
	  r = recv(clientfd, &test, sizeof(bool), 0);
	  if (test) {
		  archivo = fopen(command2, "w");
		  for(r = recv(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0); bufferArchivo[0] != '\0'; r = recv(clientfd, bufferArchivo, STRING_BUFFER * sizeof(char), 0)) {
			  //error
			  r = fputs(bufferArchivo, archivo);
			  //error
			  DEBUG("ver -> recv -> buffer=\"%s\"", bufferArchivo);
		  }
		  fclose(archivo);
		  
	  }
  }
  free(mascota);
}

void borrar(int clientfd, char* registroCadena) {
  int r;
  ulong key, id;
  FILE* archivo;
  bool test = true;

  r = send(clientfd, &hash_table.numero_de_datos, sizeof(long), 0);
  //error
  r = recv(clientfd, &key, sizeof(int), 0);
  ERROR(r == 0, perror("scanf"));
  id = hash(key);
  if (hash_table.id[id] == 0) {
	  test = false;
	  r = send(clientfd, &test, sizeof(bool), 0);
	  return;
  }
  r = send(clientfd, &test, sizeof(bool), 0);

  sem_wait(accesoADatos);
  
  archivo = fopen(ARCHIVO, "r+");
  ir_en_linea(archivo, id);
  hash_table.id[id] = 0;
  hash_table.numero_de_datos--;
  key = 0;
  r = fwrite(&key, sizeof(ulong), 1, archivo);
  ERROR(r == 0, perror("fwrite"); fclose(archivo));
  fclose(archivo);

  sem_post(accesoADatos);

  sprintf(registroCadena, "%lu", key);
}

void buscar(int clientfd, char* registroCadena) {
  int r;
  ulong i, j;
  char buffer_u[SIZE_GRANDE], buffer_d[SIZE_GRANDE];
  FILE* archivo;
  bool test;
  r = recv(clientfd, buffer_u, SIZE_GRANDE * sizeof(char), 0);
  ERROR(r == 0, perror("scanf"));
  sprintf(registroCadena, "%s", buffer_u);
  for (i = 0; i < SIZE_GRANDE; i++) {
    if (buffer_u[i] >= 'A' && buffer_u[i] <= 'Z') {
      buffer_u[i] += 32;
    }
  }
  archivo = fopen(ARCHIVO, "r");
  ERROR(archivo == NULL, perror("fopen"));


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
	      /* 2 posibilidades :
	       * send cada clave/nombre, y al final send 0/0 para que el cliente sepa que es bueno < hacemos esto porque ocupa menos memoria
	       * escribir todo en 1/2 archivo.s, y asi podemos saber cual es el tamano antes de send
	       */
	      r = send(clientfd, &hash_table.id[i], sizeof(ulong), 0);
	      //error
	      r = send(clientfd, buffer_d, sizeof(char) * SIZE_GRANDE, 0);
	      //error
	      r = recv(clientfd, &test, sizeof(bool), 0);
	      //error
	      if(test) {
		      fclose(archivo);
		      return;
	      }
      }
    }
  }
  i = 0;
  r = send(clientfd, &i, sizeof(ulong), 0);
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

void salir(int exitcode) {
  free(hash_table.id);
  free(primos.primos);
  exit(exitcode);
}
