#include "file.h"

uint8 file_append(const char *filename, struct addr address)
{
  FILE *fp = fopen(filename, "rb+");

  if (fp == NULL)
    return 0;

  uint32 cnt = 0;

  fseek(fp, 0, SEEK_SET);
  fread(&cnt, 1, sizeof(uint32), fp);

  cnt++;

  fseek(fp, 0, SEEK_SET);
  fwrite(&cnt, 1, sizeof(uint32), fp);

  fseek(fp, 0, SEEK_END);
  fwrite(&address.full, 1, sizeof(uint32), fp);

  fclose(fp);

  return 1;
}

uint8 file_search(const char *filename, struct addr address)
{
  uint8 r = 0;
  uint32 i = 0;
  struct peers *tmp = file_fetch(filename);

  for (i = 0; i < tmp->cnt; ++i)
  {
    if (tmp->itms[i].full == address.full)
    {
      r = 1;
      break;
    }
  }

  free(tmp);

  return r;
}

void file_clean(const char *filename)
{
  FILE *fp = fopen(filename, "wb");

  if (fp == NULL)
    return;

  uint32 cnt = 0;

  fseek(fp, 0, SEEK_SET);
  fwrite(&cnt, 1, sizeof(uint32), fp);

  fclose(fp);
}

struct peers *file_fetch(const char *filename)
{
  uint32 i;
  FILE *fp = fopen(filename, "rb");

  if (fp == NULL)
    return NULL;

  struct peers *tmp = calloc(1, sizeof(struct peers));

  fread(&tmp->cnt, 1, sizeof(uint32), fp);

  tmp->itms = calloc(tmp->cnt, sizeof(*tmp->itms));

  fread(tmp->itms, tmp->cnt, sizeof(*tmp->itms), fp);

  fclose(fp);

  return tmp;
}
