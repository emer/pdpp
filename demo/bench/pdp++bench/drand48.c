/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <math.h>

#include <stddef.h>

#include <limits.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

extern "C" {

/* Global state for non-reentrant functions.  */
  struct drand48_data
  {
    unsigned short int X[3];	/* Current state.  */
    unsigned short int a[3];	/* Factor in congruential formula.  */
    unsigned short int c;	/* Additive const. in congruential formula.  */
    unsigned short int old_X[3]; /* Old state.  */
    int init;			/* Flag for initializing.  */
    drand48_data() { };
  };

struct drand48_data __libc_drand48_data;

int
__drand48_iterate (unsigned short int xsubi[3], struct drand48_data *buffer)
{
  u_int64_t X, a, result;

  /* Initialize buffer, if not yet done.  */
  if (!buffer->init)
    {
#if (USHRT_MAX == 0xffffU)
      buffer->a[2] = 0x5;
      buffer->a[1] = 0xdeec;
      buffer->a[0] = 0xe66d;
#else
      buffer->a[2] = 0x5deecUL;
      buffer->a[1] = 0xe66d0000UL;
      buffer->a[0] = 0;
#endif
      buffer->c = 0xb;
      buffer->init = 1;
    }

  /* Do the real work.  We choose a data type which contains at least
     48 bits.  Because we compute the modulus it does not care how
     many bits really are computed.  */

  if (sizeof (unsigned short int) == 2)
    {
      X = (u_int64_t)xsubi[2] << 32 | (u_int64_t)xsubi[1] << 16 | xsubi[0];
      a = ((u_int64_t)buffer->a[2] << 32 | (u_int64_t)buffer->a[1] << 16
	   | buffer->a[0]);

      result = X * a + buffer->c;

      xsubi[0] = result & 0xffff;
      xsubi[1] = (result >> 16) & 0xffff;
      xsubi[2] = (result >> 32) & 0xffff;
    }
  else
    {
      X = (u_int64_t)xsubi[2] << 16 | xsubi[1] >> 16;
      a = (u_int64_t)buffer->a[2] << 16 | buffer->a[1] >> 16;

      result = X * a + buffer->c;

      xsubi[0] = result >> 16 & 0xffffffffl;
      xsubi[1] = result << 16 & 0xffff0000l;
    }

  return 0;
}

int
erand48_r (unsigned short int xsubi[3], struct drand48_data *buffer, double *result)
{
  /* Compute next state.  */
  if (__drand48_iterate (xsubi, buffer) < 0)
    return -1;

  /* Construct a positive double with the 48 random bits distributed over
     its fractional part so the resulting FP number is [0.0,1.0).  */

#if USHRT_MAX == 65535
  *result = ((double) xsubi[2] / (1ULL << 48) +
	     (double) xsubi[1] / (1ULL << 32) +
	     (double) xsubi[0] / (1ULL << 16));
#else
# error Unsupported size of short int
#endif

  return 0;
}

double
drand48 ()
{
  double result;

  (void) erand48_r (__libc_drand48_data.X, &__libc_drand48_data, &result);

  return result;
}

int
drand48_r (struct drand48_data *buffer, double *result)
{
  return erand48_r (buffer->X, buffer, result);
}

double
erand48 (unsigned short int xsubi[3])
{
  double result;

  (void) erand48_r (xsubi, &__libc_drand48_data, &result);

  return result;
}

int
jrand48_r (unsigned short int xsubi[3], struct drand48_data *buffer, long *result)
{
  /* Compute next state.  */
  if (__drand48_iterate (xsubi, buffer) < 0)
    return -1;

  /* Store the result.  */
  if (sizeof (unsigned short int) == 2)
    *result = (xsubi[2] & 0x7fff) | xsubi[1];
  else
    *result = xsubi[2] & 0x7fffffffl;

  if (xsubi[2] & (1 << (sizeof (xsubi[2]) * 8 - 1)))
    *result *= -1;

  return 0;
}

long
jrand48 (unsigned short int xsubi[3])
{
  long result;

  (void) jrand48_r (xsubi, &__libc_drand48_data, &result);

  return result;
}

int
lcong48_r (unsigned short int param[7], struct drand48_data *buffer)
{
  /* Store the given values.  */
  memcpy (buffer->X, &param[0], sizeof (buffer->X));
  memcpy (buffer->a, &param[3], sizeof (buffer->a));
  buffer->c = param[6];
  buffer->init = 1;

  return 0;
}

void
lcong48 (unsigned short int param[7])
{
  (void) lcong48_r (param, &__libc_drand48_data);
}

int
nrand48_r (unsigned short int xsubi[3], struct drand48_data *buffer, long *result)
{
  /* Compute next state.  */
  if (__drand48_iterate (xsubi, buffer) < 0)
    return -1;

  /* Store the result.  */
  if (sizeof (unsigned short int) == 2)
    *result = xsubi[2] << 15 | xsubi[1] >> 1;
  else
    *result = xsubi[2] >> 1;

  return 0;
}

long
lrand48 ()
{
  long result;

  (void) nrand48_r (__libc_drand48_data.X, &__libc_drand48_data, &result);

  return result;
}

int
lrand48_r (struct drand48_data *buffer, long *result)
{
  /* Be generous for the arguments, detect some errors.  */
  if (buffer == NULL)
   return -1;

  return nrand48_r (buffer->X, buffer, result);
}

long
mrand48 ()
{
  long result;

  (void) jrand48_r (__libc_drand48_data.X, &__libc_drand48_data, &result);

  return result;
}

int
mrand48_r (struct drand48_data *buffer, long *result)
{
  /* Be generous for the arguments, detect some errors.  */
  if (buffer == NULL)
   return -1;

  return jrand48_r (buffer->X, buffer, result);
}

long
nrand48 (unsigned short int xsubi[3])
{
  long result;

  (void) nrand48_r (xsubi, &__libc_drand48_data, &result);

  return result;
}

int
seed48_r (unsigned short int seed16v[3], struct drand48_data *buffer)
{
  /* Save old value at a private place to be used as return value.  */
  memcpy (buffer->old_X, buffer->X, sizeof (buffer->X));

  /* Install new state.  */
  memcpy (buffer->X, seed16v, sizeof (buffer->X));

  return 0;
}

int
srand48_r (long seedval, struct drand48_data *buffer)
{
  /* The standards say we only have 32 bits.  */
  if (sizeof (long) > 4)
    seedval &= 0xffffffffl;

#if (USHRT_MAX == 0xffffU)
#ifdef CYGWIN
  buffer->X[0] = seedval >> 16;
  buffer->X[1] = seedval & 0xffffl;
  buffer->X[2] = 0x330e;
#else
  buffer->X[2] = seedval >> 16;
  buffer->X[1] = seedval & 0xffffl;
  buffer->X[0] = 0x330e;
#endif
#else
  buffer->X[2] = seedval;
  buffer->X[1] = 0x330e0000UL;
  buffer->X[0] = 0;
#endif

  return 0;
}

unsigned short int *
seed48 (unsigned short int seed16v[3])
{
  (void) seed48_r (seed16v, &__libc_drand48_data);

  return __libc_drand48_data.old_X;
}

void
srand48 (long seedval)
{
  (void) srand48_r (seedval, &__libc_drand48_data);
}

}
