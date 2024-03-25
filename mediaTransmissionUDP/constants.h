#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#define SERVER_PORT 5432
#define BUF_SIZE 4096

enum
{
    FILE_REQUEST,
    ACK,
    FILE_INFO_AND_DATA,
    DATA,
    FILE_NOT_FOUND
};

typedef struct
{
    uint8_t type;
    uint8_t filename_size;
    char *filename;
} FileRequest;

typedef struct
{
    uint8_t type;
    uint8_t num_sequences;
    uint16_t *sequence_no;
} Ack;

typedef struct {
    uint8_t type;
    uint16_t sequence_number;
    uint8_t filename_size;
    char *filename;
    uint32_t file_size;
    uint16_t block_size;
    char *data;
} FileInfoAndDataPacket;

typedef struct {
    uint8_t type;
    uint16_t sequence_number;
    uint16_t block_size;
    char *data;
} DataPacket;

typedef struct {
    uint8_t type;
    uint8_t filename_size;
    char *filename;
} FileNotFoundPacket;

#endif /* CONSTANTS_H */
