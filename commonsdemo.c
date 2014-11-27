/*
 * Copyright 2014 Michael Tesch.
 * This file subject to terms of Creative Commons CC BY-NC 4.0, see LICENSE.html
 *
 * author(s): Michael Tesch (tesch1@gmail.com),
 */
/*! \file
 * \brief	Demo for using mexCommons.{ch}
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "premexh.h"
#include "mex.h"

#include "mexCommons.h"

typedef struct {
  double d1;
  float f1;
  int i1;
} demo_struct;

/*! descriptions of all fields in an varian main header
 */
static struct_fielddesc_t ds_fields[] = {
  MC_FIELD_DEF(d1, TY_DOUBLE, demo_struct),
  MC_FIELD_DEF(f1, TY_FLOAT, demo_struct),
  MC_FIELD_DEF(i1, TY_S32, demo_struct),
};

/* [a] = commonsdemo(b) */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  char * usage = 
    "usage: [a] = commonsdemo(b)\n";

  if (nrhs != 1 || nlhs != 1 || !mxIsStruct(prhs[0]))
    mexErrMsgTxt(usage);

  mwSize count;
  demo_struct * dp = mexArrayStruct(NARRAY(ds_fields), ds_fields, prhs[0], &count);

  /* for the demo, just truncate one entry */
  count--;

  /* convert it back */
  plhs[0] = mexStructArray(NARRAY(ds_fields), ds_fields, count, dp);
}
