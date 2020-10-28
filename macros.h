#ifndef MACROS_H_INCLUDED
#define MACROS_H_INCLUDED

//enum states {START, FLAG_OK, A_OK, C_OK, BCC_OK, STOP};

#define TRANSMITER  0
#define RECEIVER    1
#define MAX_SIZE    255

#define HOME_PORT_0 10
#define HOME_PORT_1 11
#define LAB_PORT_0  0
#define LAB_PORT_1  1

#define FLAG        0x7E

#define A_SND       0x03                    // A(campo de endereço) Comandos enviados pelo Emissor / Respostas enviadas pelo Receptor
#define A_RCV       0x01                    // A(campo de endereço) Comandos enviados pelo Receptor / Respostas enviadas pelo Emissor

#define C_SND       0x03
#define C_RCV       0x07
#define C_DISC      0x0B
#define C_RR(r)     ((0x05) ^ (r) << (7))
#define C_REJ(r)    ((0x01) ^ (r) << (7))
#define BCC(a, c)   (a ^ c)

typedef struct
{
    int fd; /*Descritor correspondente à porta série*/
    int status;         /*TRANSMITTER | RECEIVER*/
} applicationLayer;

typedef struct {
    char port[20];
    int baudRate;
    unsigned int sequenceNumber;
    unsigned int timeout;
    unsigned int numTransmissions;
    char frame[MAX_SIZE];
} linkLayer;

#endif
