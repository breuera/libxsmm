/******************************************************************************
* Copyright (c) Intel Corporation - All rights reserved.                      *
* This file is part of the LIBXSMM library.                                   *
*                                                                             *
* For information on the license, see the LICENSE file.                       *
* Further information: https://github.com/hfp/libxsmm/                        *
* SPDX-License-Identifier: BSD-3-Clause                                       *
******************************************************************************/
/* Alexander Heinecke (Intel Corp.)
******************************************************************************/
#include <libxsmm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

void unary_copy_f32_f32_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, float *in, float *out) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      out[(j*ldo) + i] = in[(j*ldi) + i];
    }
  }
}

void test_unary_copy_f32_f32( libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  float *in;
  float *out, *out_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;

  if ( M > ldi ) {
    fprintf( stderr, "test_unary_copy_f32_f32_fwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_unary_copy_f32_f32_fwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldi,   64);
  out       = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  out_gold  = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      in[(i*ldi)+j] = (float)libxsmm_rng_f64();
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    unary_copy_f32_f32_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)] );
  }

  /* use jited tranpose */
  unary_param.in_ptr  = (void*)in;
  unary_param.out_ptr = (void*)out;
  unary_param.mask_ptr = NULL;
  unary_flags = LIBXSMM_MELTW_FLAG_UNARY_NONE;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_IDENTITY);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );

  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
}

int main( int argc, char* argv[] ) {
  libxsmm_blasint dtype_in;
  libxsmm_blasint dtype_out;
  libxsmm_blasint dtype_comp;
  unsigned char op;
  libxsmm_blasint bitm;
  libxsmm_blasint M;
  libxsmm_blasint N;
  libxsmm_blasint ldi;
  libxsmm_blasint ldo;

  if ( argc != 10 ) {
    printf(" Error! Usage: %s [type] [bitmask: 0/1] [prec_in: 4/2] [compute_prec: 4/2] [prec_out: 4/2] [M] [N] [ldi] [ldo]\n", argv[0] );
    exit(-1);
  }

  op         = atoi(argv[1]);
  bitm       = atoi(argv[2]);
  dtype_in   = atoi(argv[3]);
  dtype_comp = atoi(argv[4]);
  dtype_out  = atoi(argv[5]);
  M          = atoi(argv[6]);
  N          = atoi(argv[7]);
  ldi        = atoi(argv[8]);
  ldo        = atoi(argv[9]);

  if ( op == 1 && dtype_in == 4 && dtype_out == 4 && dtype_comp == 4 ) {
    printf("Testing F32 F32 copy\n");
    test_unary_copy_f32_f32( M, N, ldi, ldo );
  } else {
    printf(" Error! Usage: %s [type] [bitmask: 0/1] [prec_in: 4/2] [compute_prec: 4/2] [prec_out: 4/2] [M] [N] [ldi] [ldo]\n", argv[0] );
    exit(-1);
  }
}
