#ifndef MPING_FILE_H
#define MPING_FILE_H

#include "common.h"

uint8 file_append(const char *filename, struct addr address);
uint8 file_search(const char *filename, struct addr address);

struct peers *file_fetch(const char *filename);

#endif // MPING_FILE_H
