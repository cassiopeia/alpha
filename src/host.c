#include <stdio.h>
#include <stdlib.h>

#include "file.h"

int main(int argc, char **argv)
{
  u8 r;
  char filename[255] = "misc/hosts_temp";
  struct addr address;

  if (argc > 1)
  {
    if (strcmp("append", argv[1]) == 0 || strcmp("search", argv[1]) == 0)
    {
      if (argc < 3)
      {
        printf("Unknown address\n");
        return 1;
      }

      r = sscanf(argv[2], "%hhu.%hhu.%hhu.%hhu", &address.chunk.a, &address.chunk.b, &address.chunk.c, &address.chunk.d);
      if (r != 4)
      {
        printf("Ivalid address %u.%u.%u.%u\n", (u32) address.chunk.a, (u32) address.chunk.b, (u32) address.chunk.c, (u32) address.chunk.d);
        return 1;
      }

      r = file_search(filename, address);

      if (strcmp("append", argv[1]) == 0)
      {
        if (r == 1)
        {
          printf("Address %u.%u.%u.%u exists\n", address.chunk.a, address.chunk.b, address.chunk.c, address.chunk.d);
//          return 1;
        }

        r = file_append(filename, address);
        printf("%s address %u.%u.%u.%u!\n", r == 0 ? "Not added" : "Added", address.chunk.a, address.chunk.b, address.chunk.c, address.chunk.d);
      }
      else
      {
        printf("%s address %u.%u.%u.%u!\n", r == 0 ? "Not found" : "Found", address.chunk.a, address.chunk.b, address.chunk.c, address.chunk.d);
      }
    }
    else if (strcmp("print", argv[1]) == 0)
    {
      u32 i;
      struct peers *tmp = file_fetch(filename);

      for (i = 0; i < tmp->cnt; ++i)
        printf("%u.%u.%u.%u\n", tmp->itms[i].chunk.a, tmp->itms[i].chunk.b, tmp->itms[i].chunk.c, tmp->itms[i].chunk.d);

      printf("Found %u hosts\n", tmp->cnt);

      free(tmp);
    }
    else if (strcmp("clean", argv[1]) == 0)
    {
      file_clean(filename);
      printf("Cleaned\n");
    }
    else
      printf("Unknown command\n");
  }

  return 0;
}
