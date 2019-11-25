#include "p1-dogProgram.h"
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

  while (1) {
    p = 0;
    printf(
        "1. Ingresar registro\n2. Ver registro\n3. Borrar registro\n4. "
        "Buscar\n5. Salir\n");
    while (p < '1' || p > '5') {
      p = getc(stdin);
      // p = '5';
    }
    switch (p) {  // check si p es 1, 2, 3, 4 o 5
      case '1':
        ingresar();
        break;
      case '2':
        ver();
        break;
      case '3':
        borrar();
        break;
      case '4':
        buscar(termios_p_raw, termios_p_def, buf);
        break;
      case '5':
        salir(EXIT_SUCCESS);
        break;
      default:
        ERROR(1, perror("getc"));
    }
    printf("Cualquier tecla.\n");
    fflush(stdin);
    tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_raw);
    setbuf(stdin, NULL);
    getc(stdin);
    setbuf(stdin, buf);
    tcsetattr(STDIN_FILENO, TCSANOW, &termios_p_def);
  }
}

void salir(int exitcode) {
  free(hash_table.id);
  free(primos.primos);
  exit(exitcode);
}
