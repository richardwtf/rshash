/*
 * Copyright (c) 2010, Richard Schwarz, git@richardschwarz.de
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "rshash.h"

/* 
 * Users don't need to deal with individual rows, so we define its
 * structure here.
 */
struct rshash_row {
  const void* key; // keys can not be changed
  void* value;
  size_t len; // number of bytes that are used when computing hash value
};

/*
 * The user get's access to the data, by using this iterator structure.
 */
struct rshash_iterator_s {
  int i; // id of current row
  const rshash_t* hash; // pointer to hash structure
};

/*
 * Actual hash structure.
 */
struct rshash_s {
  struct rshash_row** table; // pointer to hash table
  size_t size; // number of occupied rows
  size_t cap; // number of allocated rows
  size_t deleted; // number of rows marked as deleted
  rshash_key_dealloc_t key_dealloc_func; // key deallocation function
  rshash_value_dealloc_t value_dealloc_func; // value deallocation function
};



/*
 * Compares the first _len_ bytes of pa and pb. Returns true only if they are
 * identical. Only useful if a and b are indeed of the same length of course.
 */
bool
__byte_compare(const void* pa, const void* pb, size_t len)
{
  const uint8_t* a = (const uint8_t*)pa;
  const uint8_t* b = (const uint8_t*)pb;

  for (size_t i = 0; i < len; i++)
    if (a[i] != b[i])
      return false;
  return true;
}


/*
 * Allocates memory for a new struct rshash_row and returns pointer to it.
 * Also sets key to NULL, len to 0, and value to RSH_DEFAULT_VALUE.
 */
struct rshash_row*
__rshash_row_init()
{
  struct rshash_row* row = (struct rshash_row*)malloc(sizeof(struct rshash_row));
  row->key = NULL;
  row->len = 0;
  row->value = RSH_DEFAULT_VALUE;
  return row;
}


rshash_t*
rshash_init(rshash_key_dealloc_t kdealloc, rshash_value_dealloc_t vdealloc)
{
  rshash_t* h = (rshash_t*)malloc(sizeof(rshash_t));
  if (!h) {
    fprintf(stderr, "fatal: unable to create hash\n");
    exit(1);
  }
  h->cap = RSH_INIT_SIZE;
  h->size = h->deleted = 0;
  h->table = (struct rshash_row**)calloc(h->cap, sizeof(struct rshash_row*));
  if (!h->table) {
    fprintf(stderr, "fatal: unable to create hash table\n");
    exit(1);
  }
  h->key_dealloc_func = kdealloc;
  h->value_dealloc_func = vdealloc;
  return h;
}


rshash_t*
__rshash_clear(rshash_t* h)
{
  for (size_t i = 0; i < h->cap; i++) {
    struct rshash_row* row = h->table[i];
    if (row) {
      if (h->key_dealloc_func)
        h->key_dealloc_func((void*)row->key);
      if (h->value_dealloc_func)
        h->value_dealloc_func(row->value);
      row->len = 0;
    }
    free(row);
  }
  free(h->table);
  h->table = NULL;
  h->cap = 0;
  h->size = 0;
  h->deleted = 0;
  return h;
}


void
rshash_free(void* p)
{
  if (!p)
    return;

  __rshash_clear((rshash_t*)p);
  free(p);
}


uint32_t
rshash_hash(const void* str, size_t len)
{
  uint32_t hash = 0;
  const unsigned char* key = (const unsigned char*)str;

  size_t i;
  for (i = 0; i < len; i++)
    hash = (uint8_t)key[i] + (hash<<13) + (hash<<9) + (hash<<4) - hash;

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}


size_t
__rshash_find(const rshash_t* h, const void* key, size_t len)
{
  size_t m = h->cap - 1;
  uint32_t pos = rshash_hash(key, len);
  struct rshash_row* row = h->table[m&pos];

  while (1) {
    if (row == NULL || __byte_compare(row->key, key, len))
      return m&pos;

    pos++; /* linear collision strategy */
    row = h->table[m&pos];
  }
}


size_t
rshash_size(const rshash_t* h)
{
  return h->size;
}


rshash_iterator_t*
rshash_iterator(const rshash_t* hash)
{
  rshash_iterator_t* result =
    (rshash_iterator_t*)malloc(sizeof(rshash_iterator_t));
  result->i = -1;
  result->hash = hash;

  return result;
}


rshash_iterator_t*
rshash_iterator_next(rshash_iterator_t* iter)
{
  int l = (int)iter->hash->cap;
  while (++iter->i < l)
    if (iter->hash->table[iter->i] && iter->hash->table[iter->i]->key)
      return iter;
  return NULL;
}


const void*
rshash_iterator_key(const rshash_iterator_t* iter)
{
  return iter->hash->table[iter->i]->key;
}


void*
rshash_iterator_value(const rshash_iterator_t* iter)
{
  return iter->hash->table[iter->i]->value;
}


void
__rshash_resize(rshash_t* h, size_t newsize)
{
  if (!h || newsize == 0 || newsize % 2 != 0)
    return;

  size_t l = rshash_size(h);
  size_t c = 0;
  struct rshash_row rows[l];
  for (size_t i = 0; c < h->cap; i++) {
    if (h->table[i] && h->table[i]->key) {
      rows[c].key = h->table[i]->key;
      rows[c].len = h->table[i]->len;
      rows[c].value = h->table[i]->value;
    }
  }

  /* empty the old hash table */
  for (size_t i = 0; i < h->cap; i++)
    free(h->table[i]);
  free(h->table);

  /* create new table */
  h->size = h->deleted = 0;
  h->cap = newsize;
  h->table = (struct rshash_row**)calloc(h->cap, sizeof(struct rshash_row*));
  if (h->table == NULL) {
    fprintf(stderr, "not enough memory to resize hash, exiting...");
    exit(1);
  }

  /* fill new table */
  for (size_t i = 0; i < l; i++)
    rshash_add(h, rows[i].key, rows[i].len, rows[i].value);

  if (rshash_size(h) != l) {
    fprintf(stderr, "bug: hash table inconsistency after resizing\n");
    exit(1);
  }
}


void
__rshash_grow(rshash_t* h)
{
  __rshash_resize(h, h->cap << 1);
}


void
__rshash_shrink(rshash_t* h)
{
  __rshash_resize(h, h->cap >> 1);
}


bool
rshash_has(const rshash_t* h, const void* key, size_t len)
{
  size_t pos = __rshash_find(h, key, len);

  if (h->table[pos] == NULL) // deleted element
    return false;
  else
    return true;
}


void*
rshash_get(const rshash_t* h, const void* key, size_t len)
{
  size_t pos = __rshash_find(h, key, len);

  if (h->table[pos] && h->table[pos]->key)
    return h->table[pos]->value;
  return RSH_DEFAULT_VALUE;
}


bool
rshash_add(rshash_t* h, const void* key, size_t len, void* value)
{
  if (!h || !key || !len)
    return false;

  size_t pos = __rshash_find(h, key, len);

  if (!h->table[pos]) {
    h->table[pos] = __rshash_row_init();
    h->table[pos]->key = key;
    h->table[pos]->value = value;
    h->table[pos]->len = len;
    h->size++;
  } else {
    return false;
  }

  /* it may be neccessary to grow the hash table now */
  if (((float)(h->size + h->deleted)/(float)h->cap) > RSH_MAX_LOAD)
    __rshash_grow(h);

  return true;
}


void
rshash_set(rshash_t* h, const void* key, size_t len, void* value)
{
  if (!h || !key)
    return;

  size_t pos = __rshash_find(h, key, len);

  if (h->table[pos])
    h->table[pos]->value = value;
}


void
rshash_del(rshash_t* h, const void* key, size_t len)
{
  if (!h || !key)
    return;

  size_t pos = __rshash_find(h, key, len);

  if (h->table[pos] == NULL)
    return;

  h->table[pos]->key = NULL; // NULL key marks row as deleted
  h->size--;
  h->deleted++;

  /* it may be useful to shrink the hash table now */
  if (h->cap > RSH_INIT_SIZE &&
        ((double)(h->size+h->deleted)/(double)h->cap) < RSH_MIN_LOAD)
    __rshash_shrink(h);
}


const void**
rshash_keys(rshash_t* h)
{
  if (!h || h->size == 0)
    return NULL;

  const void** keys = (const void**)calloc(h->size, sizeof(const void*));

  size_t i = 0;
  rshash_iterator_t* it = rshash_iterator(h);
  while (rshash_iterator_next(it))
    keys[i++] = rshash_iterator_key(it);
  free(it);

  return keys;
}


void**
rshash_values(rshash_t* h)
{
  if (!h || h->size == 0)
    return NULL;

  void** values = (void**)calloc(h->size, sizeof(void*));

  size_t i = 0;
  rshash_iterator_t* it = rshash_iterator(h);
  while (rshash_iterator_next(it))
    values[i++] = rshash_iterator_value(it);
  free(it);

  return values;
}

