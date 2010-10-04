#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rshash.h"

typedef struct foo_s {
  long a;
  long b;
  short c;
} foo_t;


int main() {
  foo_t x, y, z;
  bool b;
  char* s = NULL;

  x.a = 12;
  x.b = 14;
  x.c = 1;

  y.a = 23;
  y.b = 25;
  y.c = 3;

  z.a = 12;
  z.b = 14;
  z.c = 1;

  
  rshash_t* hash = rshash_init(NULL, NULL);

  /* add x */
  b = rshash_add(hash, &x, sizeof(foo_t), "x");
  if (!b)
    fprintf(stderr, "unable to add x\n");
  else
    fprintf(stderr, "added x\n");

  b = rshash_has(hash, &x, sizeof(foo_t));
  if (!b)
    fprintf(stderr, "hash couldn't find x");
  else
    fprintf(stderr, "found x in hash\n");

  s = (char*)rshash_get(hash, &x, sizeof(foo_t));
  fprintf(stderr, "x has value: '%s'\n", s);

  /* add y */
  b = rshash_add(hash, &y, sizeof(foo_t), "y");
  if (!b)
    fprintf(stderr, "unable to add y\n");
  else
    fprintf(stderr, "added y\n");

  b = rshash_has(hash, &y, sizeof(foo_t));
  if (!b)
    fprintf(stderr, "hash couldn't find y");
  else
    fprintf(stderr, "found y in hash\n");

  s = (char*)rshash_get(hash, &y, sizeof(foo_t));
  fprintf(stderr, "y has value: '%s'\n", s);

  /* add z */
  b = rshash_add(hash, &z, sizeof(foo_t), "z");
  if (!b)
    fprintf(stderr, "unable to add z\n");
  else
    fprintf(stderr, "added z\n");

  b = rshash_has(hash, &z, sizeof(foo_t));
  if (!b)
    fprintf(stderr, "hash couldn't find z");
  else
    fprintf(stderr, "found z in hash\n");

  s = (char*)rshash_get(hash, &z, sizeof(foo_t));
  fprintf(stderr, "z has value: '%s'\n", s);

  /* add z again */
  b = rshash_add(hash, &z, sizeof(foo_t), "z");
  if (!b)
    fprintf(stderr, "unable to add z\n");
  else
    fprintf(stderr, "added z\n");

  b = rshash_has(hash, &z, sizeof(foo_t));
  if (!b)
    fprintf(stderr, "hash couldn't find z");
  else
    fprintf(stderr, "found z in hash\n");

  s = (char*)rshash_get(hash, &z, sizeof(foo_t));
  fprintf(stderr, "z has value: '%s'\n", s);



  rshash_free(hash);

}
