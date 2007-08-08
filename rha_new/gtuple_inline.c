/*
 * gtuple - an fixed length array of values
 *
 * This file contains the inline functions
 * 
 * (c) 2005 by Mikio L. Braun
 */

/*
 * This file has been generated from the gtuple.tmpl template + some
 * editing
 */

INLINE
int gtuple_length(gtuple_t *t)
{
  return t->len;
}

INLINE
void gtuple_set_(gtuple_t *t, int i, intptr_t x)
{
  t->array[i] = x;
}	

INLINE
intptr_t gtuple_get(gtuple_t *t, int i)
{
  return t->array[i];
}	

INLINE
intptr_t *gtuple_array(gtuple_t *t)
{
  return t->array;
}
