/*
 * (c) Copyright 2013 Regents of the University of Minnesota.
 * This file subject to terms of Creative Commons CC BY-NC 4.0, see LICENSE.html
 * (c) Copyright 2014 Michael Tesch. Modifications released under CC BY-NC 4.0.
 *
 * author(s): Michael Tesch (tesch1@gmail.com),
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mex.h"

#include "mexCommons.h"
#define DEBUG
/*! \brief      Reroute printf() calls out through Matlab's mexPrintf()
 *
 * Utility function to help patch normal c-output into matlab/octave's
 * output screen.  To do this, create a local function pointer for calling
 * printf and have it default to printf.  When the mex function is setup,
 * it can re-write the pointer to point to this callback...
 *
 *    int (*my_printf)(const char *format, ...) = printf;
 *
 * then, before calling any functions, reassign the global my_printf:
 *
 *  my_printf = mexCommonsPrintfCallback;
 *
 * and call it like normal printf from your code: my_printf("hello world\n");
 *
 */
int mexCommonsPrintfCallback(const char *format, ...)
{
  int ret = mexPrintf(format);
#if 1
  /*mexEvalString("drawnow;"); /* let matlab flush the printf */
  /*mexEvalString("pause(0.0001);"); /* let matlab flush the printf */
  mexEvalString("drawnow;"); /* let matlab flush the printf */
#else
  mxArray * exception;
  exception = mexCallMATLABWithTrap(0, NULL, 0, NULL, "drawnow"); /* let matlab flush the printf */
  if(exception != NULL) {
    fprintf(stderr, "EXCEPTION\n");
    /* Throw the MException returned by mexCallMATLABWithTrap
     * after cleaning up any dynamically allocated resources */
    mexCallMATLAB(0, (mxArray **)NULL, 
                  1, &exception, "throw");
  }
#endif
  return ret;
}

/*! \brief      Construct a mex array from an array of structs described by structfields
 */
mxArray * mexStructToArray(int nfields, struct_fielddesc_t * sfields, size_t count, void * base)
{
  mxArray * array;
  int i, j;
  size_t structbytes = 0;

  /* Construct outdata */
  const char ** field_names = mxCalloc(nfields, sizeof(char **));
  for (i = 0; i < nfields; i++) {
    int known = 0;

    field_names[i] = sfields[i].name;

    /* make sure [i] isnt already accounted for */
    for (j = 0; j < i; j++) {
      if (sfields[i].offset == sfields[j].offset)
        known = 1;
    }
    if (!known)
      structbytes += sfields[i].size;
  }

  array = mxCreateStructMatrix(1, 1, nfields, field_names);
  mxFree((void *)field_names);

  /* mexWarnMsgTxt("finished making array\n"); */

  /* make mxArrays from each field */
  for (i = 0; i < nfields; i++) {
    mxArray *field_value = NULL;
    int array_count;

    switch (sfields[i].type) {
    case TY_U32: {
      uint32_t * data;
      array_count = sfields[i].size / sizeof(uint32_t);
      field_value = mxCreateNumericMatrix(count, array_count, mxUINT32_CLASS, mxREAL);
      data = mxGetData(field_value);
      for (j = 0; j < count; j++) {
        int k;
        uint32_t * start = (uint32_t *)(base + j * structbytes + sfields[i].offset);
        for (k = 0; k < array_count; k++) {
          data[k * count + j] = start[k];
        }
      }
      break;
    }
    case TY_S32: {
      int32_t * data;
      array_count = sfields[i].size / sizeof(int32_t);
      field_value = mxCreateNumericMatrix(count, array_count, mxINT32_CLASS, mxREAL);
      data = mxGetData(field_value);
      for (j = 0; j < count; j++) {
        int k;
        int32_t * start = (int32_t *)(base + j * structbytes + sfields[i].offset);
        for (k = 0; k < array_count; k++) {
          data[k * count + j] = start[k];
        }
      }
      break;
    }
    case TY_U16: {
      uint16_t * data;
      array_count = sfields[i].size / sizeof(uint16_t);
      field_value = mxCreateNumericMatrix(count, array_count, mxUINT16_CLASS, mxREAL);
      data = mxGetData(field_value);
      for (j = 0; j < count; j++) {
        int k;
        uint16_t * start = (uint16_t *)(base + j * structbytes + sfields[i].offset);
        for (k = 0; k < array_count; k++) {
          data[k * count + j] = start[k];
        }
      }
      break;
    }
    case TY_FLOAT: {
      float * data;
      array_count = sfields[i].size / sizeof(float);
      field_value = mxCreateNumericMatrix(count, array_count, mxSINGLE_CLASS, mxREAL);
      data = mxGetData(field_value);
      for (j = 0; j < count; j++) {
        int k;
        float * start = (float *)(base + j * structbytes + sfields[i].offset);
        for (k = 0; k < array_count; k++) {
          data[k * count + j] = start[k];
        }
      }
      break;
    }
    case TY_DOUBLE: {
      double * data;
      array_count = sfields[i].size / sizeof(double);
      field_value = mxCreateNumericMatrix(count, array_count, mxDOUBLE_CLASS, mxREAL);
      data = mxGetData(field_value);
      for (j = 0; j < count; j++) {
        int k;
        double * start = (double *)(base + j * structbytes + sfields[i].offset);
        for (k = 0; k < array_count; k++) {
          data[k * count + j] = start[k];
        }
      }
      break;
    }
    default:
      mexErrMsgTxt("internal error");
    }

    mxSetFieldByNumber(array, 0, i, field_value);
  }
  return array;
}

#define CAST_TO(ctype, mctype)                                          \
  ctype mctype##_read(mxArray * array, mwSize index) {                  \
  void * vv = (((void *)mxGetData(array)) + mxGetElementSize(array) * index); \
  switch (mxGetClassID(array)) {                                        \
  case mxCHAR_CLASS: return (ctype)*(char *)vv;                         \
  case mxDOUBLE_CLASS: return (ctype)*(double *)vv;                     \
  case mxSINGLE_CLASS: return (ctype)*(float *)vv;                      \
  case mxINT8_CLASS: return (ctype)*(signed char *)vv;                  \
  case mxUINT8_CLASS: return (ctype)*(unsigned char *)vv;               \
  case mxINT16_CLASS: return (ctype)*(int16_t *)vv;                     \
  case mxUINT16_CLASS: return (ctype)*(uint16_t *)vv;                   \
  case mxINT32_CLASS: return (ctype)*(int32_t *)vv;                     \
  case mxUINT32_CLASS: return (ctype)*(uint32_t *)vv;                   \
  case mxINT64_CLASS: return (ctype)*(int64_t *)vv;                     \
  case mxUINT64_CLASS: return (ctype)*(uint64_t *)vv;                   \
  case mxLOGICAL_CLASS:                                                 \
  case mxVOID_CLASS:                                                    \
    break;                                                              \
  } }

CAST_TO(uint16_t, TY_U16);
CAST_TO(int16_t, TY_S16);
CAST_TO(uint32_t, TY_U32);
CAST_TO(int32_t, TY_S32);
CAST_TO(float, TY_FLOAT);
CAST_TO(double, TY_DOUBLE);

/* to figure out compiler alignment */
struct teststruct {
  double firstfield;
  char testfield;
};

/*! \brief      Construct a struct from a mexArray of structs described by structfields
 */
void * mexArrayToStruct(int nfields, struct_fielddesc_t * sfields, const mxArray * array, mwSize * Pcount)
{
  int ii, jj;
  size_t structbytes = 0;
  size_t structalign = sizeof(struct teststruct) - sizeof(double);
  size_t narrayfields = mxGetNumberOfFields(array);
  if (narrayfields < nfields) {
    mexPrintf("mexArrayStruct: failed, wrong number of fields in array: %zd < %d\n", narrayfields, nfields);
    mexErrMsgTxt("fail");
  }

  /* Construct outdata */
  for (ii = 0; ii < nfields; ii++) {
    int known = 0;

    /*field_names[ii] = sfields[ii].name; */
    /* todo: double-check field name ? */

    /* make sure [ii] isnt already accounted for (ie an alias) */
    for (jj = 0; jj < ii; jj++) {
      if (sfields[ii].offset == sfields[jj].offset)
        known = 1;
    }
    if (!known)
      structbytes += sfields[ii].size;
  }
  /* align bytes */
  structbytes = structalign * ((structbytes + structalign - 1) / structalign);

  /*mexPrintf("mexArrayStruct structbytes: %d align:%d\n", (int)structbytes, (int)structalign);*/

  /* get number of entries from size of first field */
  *Pcount = 0;
  const mwSize *dims = mxGetDimensions(mxGetFieldByNumber(array, 0, 0));
  mwSize count = dims[0];
  void * base = malloc(structbytes * count);
  if (!base) {
    mexErrMsgTxt("mexArrayStruct() malloc failed");
    return NULL;
  }

#ifdef DEBUG
  mexPrintf("mexArrayStruct count: %d structbytes: %d\n", (int)count, structbytes);
#endif

  /* get mxArrays into each field */
  for (ii = 0; ii < nfields; ii++) {
#ifdef DEBUG
    mexPrintf("mexArrayStruct: getting field %s\n", sfields[ii].name);
#endif
    mxArray *field_value = NULL;
    for (jj = 0; jj < narrayfields; jj++)
      if (!strcmp(mxGetFieldNameByNumber(array, jj), sfields[ii].name))
        field_value = mxGetFieldByNumber(array, 0, jj);

    if (!field_value) {
      free(base);
      mexPrintf("mexArrayStruct: unable to find field %s\n", sfields[ii].name);
      mexErrMsgTxt("fail.");
      return NULL;
    }
    if (count != mxGetM(field_value)) {
      free(base);
      mexPrintf("mexArrayStruct: field %s has wrong number of rows: %d / %d\n", 
                sfields[ii].name, (int)mxGetM(field_value), count);
      mexErrMsgTxt("fail.");
      return NULL;
    }
    int array_count;

#define CASE_TYPE(FTYPEID, CTYPE)                                       \
      case FTYPEID: {                                                   \
        array_count = sfields[ii].size / sizeof(CTYPE);                 \
        if (1)                                                          \
          mexPrintf("mexArrayStruct: getting field %s, count=%d array_count=%d\n", \
                    sfields[ii].name, count, array_count);               \
        for (jj = 0; jj < count; jj++) {                                \
          int kk;                                                       \
          CTYPE * start = (CTYPE *)(base + jj * structbytes + sfields[ii].offset); \
          for (kk = 0; kk < array_count; kk++)                          \
            start[kk] = FTYPEID##_read(field_value, kk * count + jj);   \
        }                                                               \
        break;                                                          \
      }

    switch (sfields[ii].type) {
      CASE_TYPE(TY_U32, uint32_t);
      CASE_TYPE(TY_S32, int32_t);
      CASE_TYPE(TY_U16, uint16_t);
      CASE_TYPE(TY_S16, int16_t);
      CASE_TYPE(TY_FLOAT, float);
      CASE_TYPE(TY_DOUBLE, double);
    default:
      mexErrMsgTxt("internal error");
    }
  }
  *Pcount = count;
  return base;
}
