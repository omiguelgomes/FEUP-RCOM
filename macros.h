#define FLAG        0x7E

#define A_EMIS      0x03                    // A(campo de endereço) Comandos enviados pelo Emissor / Respostas enviadas pelo Receptor
#define A_RECE      0x01                    // A(campo de endereço) Comandos enviados pelo Receptor / Respostas enviadas pelo Emissor

#define C_SET       0x03                    
#define C_DISC      0x0B
#define C_UA        0x07
#define C_RR(r)     ((0x05) ^ (r) << (7))
#define C_REJ(r)    ((0x01) ^ (r) << (7))
#define BCC(a, c)   (a ^ c)

typedef enum
{
    START = 0,
    FLAG_RCV = 1,
    A_RCV = 2,
    C_RCV = 3,
    BCC_OK = 4,
    STOP = 5
}states;
