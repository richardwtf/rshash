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

/*
 * Real Simple Hashing Module
 *
 * for void* -> void* mappings
 */

#ifndef _RSHASH_H_
#define _RSHASH_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


/* initial size of an empty hash, power of two needed */
#define RSH_INIT_SIZE 32

/* default value for non existing keys */
#define RSH_DEFAULT_VALUE NULL

/* while adding, a load > RSH_MAX_LOAD causes growth */
#define RSH_MAX_LOAD 0.6

/* while deleting, a load < RSH_MIN_LOAD causes shrinking */
#define RSH_MIN_LOAD 0.125


/* Type definitions */

typedef struct rshash_s rshash_t;
typedef struct rshash_iterator_s rshash_iterator_t;

/* Function to deallocate a key. */
typedef void (*rshash_key_dealloc_t)(void*);
/* Function to deallocate a value. */
typedef void (*rshash_value_dealloc_t)(void*);

/* Function prototypes */

/*
 * Allocates memory for a new rshash_t and returns a pointer to it.
 * Also allocates memory for the table, and sets all rows to NULL.
 * Sets size to RSH_INIT_SIZE and occupied and deleted to 0.
 */
rshash_t* rshash_init(rshash_key_dealloc_t, rshash_value_dealloc_t);

/*
 * Free's all memory that the hash consumes. Free's every row, the
 * table, and the hash itself. If deallocation functions are not NULL,
 * use them to free keys and values.
 *
 */
void rshash_free(void*);

/*
 * Returns iterator over hash _hash_.
 */
rshash_iterator_t* rshash_iterator(const rshash_t* hash);

/*
 * Advances the iterator.
 */
rshash_iterator_t* rshash_iterator_next(rshash_iterator_t* iter);

/*
 * Returns the key of the current row the iterator points to.
 */
const void* rshash_iterator_key(const rshash_iterator_t*);

/*
 * Returns the value of the current row the iterator points to.
 */
void* rshash_iterator_value(const rshash_iterator_t*);

/*
 * Returns a 32 bit hash value for the first _len_ bytes of a given key
 * _key_. The hash function is hash(i) = hash(i-1) * 8719 (for each byte
 * i in _key_). With some bit mangling afterwards.
 */
uint32_t rshash_hash(const void* key, size_t len);

/*
 * Return integral number of populated rows in _h_.
 */
size_t rshash_size(const rshash_t* h);

/*
 * Return true if the given _key_ with length _len_ is a key in the
 * table of _h_, according to the comparison function.
 */
bool rshash_has(const rshash_t* h, const void* key, size_t len);

/*
 * Delete the given _key_ with length _len_ from _h_, or do nothing if
 * _key_ is not in _h_.
 */
void rshash_del(rshash_t* h, const void* key, size_t len);

/*
 * Add _key_/_value_ pair to hash. Returns false if _key_ exists in _h_,
 * true if adding was successfull.
 */
bool rshash_add(rshash_t* h, const void* key, size_t len, void* value);

/*
 * Returns value of given _key_ or RSH_DEFAULT_VALUE if _key_ doesn't
 * exist in _h_.
 */
void* rshash_get(const rshash_t* h, const void* key, size_t len);

/*
 * Set the value of _key_ to _value_. If _key_ doesn't exist in _h_, do
 * nothing.
 */
void rshash_set(rshash_t* h, const void* key, size_t len, void* value);

/*
 * Returns array of keys that populate hash _h_.
 */
const void** rshash_keys(rshash_t* h);

/*
 * Returns array of values that populate hash _h_.
 */
void** rshash_values(rshash_t* h);


#endif /* _RSHASH_H_ */
