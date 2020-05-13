#ifndef __UTILS_H__
#define __UTILS_H__

#include "tox/tox.h"

void string_to_id(unsigned char* dest, unsigned char* src, int size);
void id_to_string(unsigned char *dest, unsigned char *src, int size);
size_t load_save(uint8_t **out_data);
void write_save(Tox *tox);

#endif