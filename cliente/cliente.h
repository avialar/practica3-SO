void menu();
void ingresar(int clientfd);
void ver(int clientfd);
void borrar(int clientfd);
void buscar(int clientfd,
            struct termios termios_p_raw,
            struct termios termios_p_def,
            char buf[]);

