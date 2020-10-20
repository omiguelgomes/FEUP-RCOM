

/*-------------------------FUNCTIONS-------------------------*/
int main();

/*
argumentos
– fd: identificador da ligação de dados
– buffer: array de caracteres a transmitir
– length: comprimento do array de caracteres
*
retorno
– número de caracteres escritos
– valor negativo em caso de erro
*/
int llwrite(int fd, char *buffer, int length);

/*
argumentos
– porta: COM1, COM2, ...
– flag: TRANSMITTER / RECEIVER
*
retorno
– identificador da ligação de dados
– valor negativo em caso de erro
*/
int llopen(int porta);

/*
argumentos
– fd: identificador da ligação de dados
*
retorno
– valor positivo em caso de sucesso
– valor negativo em caso de erro*/
int llclose();