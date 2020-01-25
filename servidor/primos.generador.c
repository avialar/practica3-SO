#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ARCHIVO "primos.dat"
#define ERROR(test, funcion) \
  if (test) {                \
    funcion;                 \
    return (EXIT_FAILURE);   \
  }

typedef unsigned int uint;

/*
  Usamos la criba de Eratostenes (
  https://lmgtfy.com/?q=criba+de+erat%C3%B3stenes&s=d&t=w )
 */
int main(int argc, char* argv[]) {
  ERROR(argc < 2 || argc > 3,
        printf("USAGE\n\t%s <numero maximo> [base]\n\tnumero > 0\n\t1 < base < "
               "37\n\tPor defecto, la base es automatica (10 = 0xA = 0xa = "
               "012)\nPARA LA PRACTICA\n\t%s 0xAA0000\n",
               argv[0], argv[0]));  // usage

  long n, b = 0;
  ulong i, j, max = sqrt(UINT_MAX);
  bool* tabla;
  char* endptr;
  FILE* archivo;

  if (argc == 3) {  // base
    b = strtol(argv[2], &endptr, 10);
    ERROR(endptr != NULL && *endptr != '\0',
          fprintf(stderr, "Error : La base debe ser un numero!\n"));
  }

  n = strtol(argv[1], &endptr, b);
  ERROR(endptr != NULL && *endptr != '\0',
        fprintf(stderr, "Error : el numero debe ser un numero!\n"));
  ERROR(n < 1, fprintf(stderr, "Error : el numero no es valido\n%ld < 1\n", n));

  // ahora sabemos que tenemos que trabajar
  archivo = fopen(ARCHIVO, "w");
  ERROR(archivo == NULL, perror("fopen"));
  tabla = (bool*)calloc(sizeof(bool), n);  // todo 0
  ERROR(tabla == NULL, perror("calloc"));

  for (i = 2; i < n; i++) {
    if (!tabla[i]) {  // solo si es primo
      fwrite(&i, sizeof(ulong), 1, archivo);
      for (j = i; i < max && (j * i) < n;
           j++) {  // 2 * 2, 2 * 3, 2 * 4... no son primos
        tabla[i * j] = 1;
      }
    }
  }

  free(tabla);
  fclose(archivo);
  return EXIT_SUCCESS;
}
