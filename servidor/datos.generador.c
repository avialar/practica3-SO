#include <time.h>

#include "p1-dogProgram.h"

#define ESTRUCTURAS 10000000
#define NOMBRES 1717

int main(int argc, char* argv[]) {
  ERROR(argc < 2 || argc > 3,
        fprintf(stderr,
                "USAGE\n\t%s <numero de estructuras> [base]\n\tnumero > 0\n\t1 "
                "< base < 37\n\tPor defecto, la base es automatica (10 = 0xA = "
                "0xa = 012)\nPARA LA PRACTICA\n\t%s 10000000\n",
                argv[0], argv[0]));  // usage

  FILE *datos_a, *nombre_a;
  ulong i, j, n, key, cols, e, b;
  int r;
  dogType tmp = {"", "", 0, "", 0, 0, 'm'};
  char *nombres[NOMBRES], buffer, *endptr;
  clockid_t c = CLOCK_MONOTONIC_RAW;
  struct timespec t;
  clock_gettime(c, &t);
  srand(t.tv_sec);
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  cols = (w.ws_col - 8);

  if (argc == 3) {  // base
    b = strtol(argv[2], &endptr, 10);
    ERROR(endptr != NULL && *endptr != '\0',
          fprintf(stderr, "Error : La base debe ser un numero!\n"));
  }

  e = strtol(argv[1], &endptr, b);
  ERROR(endptr != NULL && *endptr != '\0',
        fprintf(stderr, "Error : el numero debe ser un numero!\n"));
  ERROR(e < 1, fprintf(stderr, "Error : el numero no es valido\n%ld < 1\n", e));

  setbuf(stdout, NULL);

  for (i = 0; i < NOMBRES; i++) {
    nombres[i] = (char*)calloc(SIZE_GRANDE, sizeof(char));
  }

  datos_a = fopen(ARCHIVO, "w");
  nombre_a = fopen("nombresMascotas.txt", "r");
  ERROR(datos_a == 0 || nombre_a == 0, perror("fopen"));
  buffer = (char)fgetc(nombre_a);
  for (i = 0, j = 0; buffer != -1; buffer = (char)fgetc(nombre_a)) {
    if (buffer == '\n') {
      i++;
      j = 0;
    } else {
      nombres[i][j] = buffer;
      j++;
    }
  }

  for (i = 1; i <= e; i++, rewind(nombre_a)) {
	  if(e > 100) {
		  PROGRESSION(i, e, cols, 100);
	  }
    n = rand() % NOMBRES;
    for (j = 0; nombres[n][j] != 0; j++) {
      tmp.nombre[j] = nombres[n][j];
    }
    tmp.nombre[j] = 0;
    key = i;
    r = fwrite(&key, sizeof(ulong), 1, datos_a);
    ERROR(r == 0, perror("fwrite"));
    r = fwrite(&tmp, sizeof(dogType), 1, datos_a);
    ERROR(r == 0, perror("fwrite"));
  }

  printf("\n");
  return EXIT_SUCCESS;
}

void salir(int exitcode) {
  exit(exitcode);
}
