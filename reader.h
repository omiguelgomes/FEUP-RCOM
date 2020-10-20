

int main();

/*
argumentos
– fd: identificador da ligação de dados
– buffer: array de caracteres recebidos
*
retorno
– comprimento do array (número de caracteres lidos)
– valor negativo em caso de erro*/
int llread(int fd, char *buffer);