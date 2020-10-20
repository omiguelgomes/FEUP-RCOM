//byte(A)

//address field byte for command sent by sender,
//or answer by receiver
#define ADDRESS_FIELD_SENDER 0x03

//address field byte for command sent by receiver,
//or answer by sender
#define ADDRESS_FIELD_RECEIVER 0x01

/*-------------------------FUNCTIONS-------------------------*/
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