
#ifndef CHECK_DATA_H
#define CHECK_DATA_H

#include <stdint.h>

#include "../src/mem.h"
#include "../src/bstrlib/bstrlib.h"

typedef struct _BLOB {
   uint32_t sentinal_start;
   size_t data_len;
   uint16_t* data;
   uint32_t sentinal_end;
} BLOB;

void free_blob( BLOB* blob );
BLOB* create_blob( uint32_t sent_s, uint16_t ptrn, size_t c, uint32_t sent_e );

#endif /* CHECK_DATA_H */
