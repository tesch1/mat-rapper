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

/* to figure out compiler alignment - of course you have to compile this
 * with the same compiler & settings as the caller. */
struct teststruct2 {
  char zerofield;
  double firstfield;
};

/* gather some info about the fields */
static void grok_fielddesc(const struct_fielddesc_t * sfields, 
                           int * Pnfields, size_t * Pstructbytes)
{
  int nfields;
  size_t structbytes = 0;
  size_t structalign = 0;
  int ii, jj;
  for (ii = 0; ii < 100; ii++) {
    if (!sfields[ii].name)
      break;
    structalign = MAX(structalign, sizeof_ftype(sfields[ii].type));
  }

  nfields = ii;
  if (nfields == 100)
    mexWarnMsgTxt("mexCommons: more than 100 fields in struct desc, maybe forgot to terminate?");

  mexPrintf("mexCommons: structalign: %d", structalign);

  /* find last field and its length */
  for (ii = 0; ii < nfields; ii++) {
    size_t fieldend = sfields[ii].offset + sfields[ii].size;
    structbytes = fieldend > structbytes ? fieldend : structbytes;
  }
  /* align bytes */
  structbytes = structalign * ((structbytes + structalign - 1) / structalign);
  /* returns */
  *Pnfields = nfields;
  *Pstructbytes = structbytes;
}

/*! \brief      Construct a mex array from an array of structs described by structfields
 */
mxArray * mexStructToArray(struct_fielddesc_t * sfields, size_t count, void * base)
{
  mxArray * array;
  int ii, jj;
  size_t structbytes;
  int nfields;

  /* get nfields and struct length */
  grok_fielddesc(sfields, &nfields, &structbytes);

  /* Construct outdata */
  const char ** field_names = mxCalloc(nfields, sizeof(char **));
  for (ii = 0; ii < nfields; ii++) {
    field_names[ii] = sfields[ii].name;
  }

  array = mxCreateStructMatrix(1, 1, nfields, field_names);
  mxFree((void *)field_names);

  /* mexWarnMsgTxt("finished making array\n"); */

  /* make mxArrays from each field */
  for (ii = 0; ii < nfields; ii++) {
    mxArray *field_value = NULL;
    int array_count;

#define CASE_TYPE(FTYPEID, CTYPE, MXTYPE)                               \
      case FTYPEID: {                                                   \
        CTYPE * data;                                                   \
        array_count = sfields[ii].size / sizeof(CTYPE);                 \
        field_value = mxCreateNumericMatrix(array_count, count,         \
                                            MXTYPE, mxREAL);            \
        data = mxGetData(field_value);                                  \
        for (jj = 0; jj < count; jj++) {                                \
          int kk;                                                       \
          CTYPE * start = (CTYPE *)(base + jj * structbytes + sfields[ii].offset); \
          for (kk = 0; kk < array_count; kk++) {                        \
            data[count * kk + jj] = start[kk];                          \
          }                                                             \
        }                                                               \
        break;                                                          \
      }

    switch (sfields[ii].type) {
      CASE_TYPE(TY_U32, uint32_t, mxUINT32_CLASS);
      CASE_TYPE(TY_S32, int32_t, mxINT32_CLASS);
      CASE_TYPE(TY_U16, uint16_t, mxUINT16_CLASS);
      CASE_TYPE(TY_S16, int16_t, mxINT16_CLASS);
      CASE_TYPE(TY_FLOAT, float, mxSINGLE_CLASS);
      CASE_TYPE(TY_DOUBLE, double, mxDOUBLE_CLASS);
#undef CASE_TYPE
    default:
      mexErrMsgTxt("mexStructToArray() internal error, field type not recognized");
    }

    mxSetFieldByNumber(array, 0, ii, field_value);
  }
  return array;
}

#define CAST_READ(ctype, mctype)                                        \
  ctype mctype##_read(mxArray * array, mwSize index) {                  \
    void * vv = (((void *)mxGetData(array)) + mxGetElementSize(array) * index); \
    switch (mxGetClassID(array)) {                                      \
    case mxCHAR_CLASS: return (ctype)*(char *)vv;                       \
    case mxDOUBLE_CLASS: return (ctype)*(double *)vv;                   \
    case mxSINGLE_CLASS: return (ctype)*(float *)vv;                    \
    case mxINT8_CLASS: return (ctype)*(signed char *)vv;                \
    case mxUINT8_CLASS: return (ctype)*(unsigned char *)vv;             \
    case mxINT16_CLASS: return (ctype)*(int16_t *)vv;                   \
    case mxUINT16_CLASS: return (ctype)*(uint16_t *)vv;                 \
    case mxINT32_CLASS: return (ctype)*(int32_t *)vv;                   \
    case mxUINT32_CLASS: return (ctype)*(uint32_t *)vv;                 \
    case mxINT64_CLASS: return (ctype)*(int64_t *)vv;                   \
    case mxUINT64_CLASS: return (ctype)*(uint64_t *)vv;                 \
    case mxLOGICAL_CLASS:                                               \
    case mxVOID_CLASS:                                                  \
      break;                                                            \
    }                                                                   \
  }

CAST_READ(uint16_t, TY_U16);
CAST_READ(int16_t, TY_S16);
CAST_READ(uint32_t, TY_U32);
CAST_READ(int32_t, TY_S32);
CAST_READ(float, TY_FLOAT);
CAST_READ(double, TY_DOUBLE);

#undef CAST_READ

/*! \brief      Construct a struct from a mexArray of structs described by structfields
 */
void * mexArrayToStruct(struct_fielddesc_t * sfields, const mxArray * array, mwSize * Pcount)
{
  int ii, jj;
  size_t structbytes = 0;
  int nfields;
  size_t narrayfields = mxGetNumberOfFields(array);

  /* get nfields and struct length */
  grok_fielddesc(sfields, &nfields, &structbytes);

  if (narrayfields < nfields) {
    mexPrintf("mexArrayToStruct: failed, wrong number of fields in array: %zd < %d\n",
              narrayfields, nfields);
    mexErrMsgTxt("fail");
  }

  /* Construct outdata */
  /*mexPrintf("mexArrayToStruct structbytes: %d align:%d\n", (int)structbytes, (int)structalign);*/

  /* get number of entries from size of first field */
  *Pcount = 0;
  const mwSize *dims = mxGetDimensions(mxGetFieldByNumber(array, 0, 0));
  mwSize count = dims[0];
  void * base = malloc(structbytes * count);
  if (!base) {
    mexErrMsgTxt("mexArrayToStruct() malloc failed");
    return NULL;
  }

  //#ifdef DEBUG
  mexPrintf("mexArrayToStruct count: %d structbytes: %d\n", (int)count, structbytes);
  //#endif

  /* get mxArrays into each field */
  for (ii = 0; ii < nfields; ii++) {
#ifdef DEBUG
    mexPrintf("mexArrayToStruct: getting field %s\n", sfields[ii].name);
#endif
    mxArray *field_value = NULL;
    for (jj = 0; jj < narrayfields; jj++)
      if (!strcmp(mxGetFieldNameByNumber(array, jj), sfields[ii].name))
        field_value = mxGetFieldByNumber(array, 0, jj);

    if (!field_value) {
      free(base);
      mexPrintf("mexArrayToStruct: unable to find field %s\n", sfields[ii].name);
      mexErrMsgTxt("fail.");
      return NULL;
    }
    if (count != mxGetM(field_value)) {
      free(base);
      mexPrintf("mexArrayToStruct: field %s has wrong number of rows: %d / %d\n", 
                sfields[ii].name, (int)mxGetM(field_value), count);
      mexErrMsgTxt("fail.");
      return NULL;
    }
    int array_count;

#define CASE_TYPE(FTYPEID, CTYPE)                                       \
      case FTYPEID: {                                                   \
        array_count = sfields[ii].size / sizeof(CTYPE);                 \
        if (0)                                                          \
          mexPrintf("mexArrayToStruct: getting field %s, count=%d array_count=%d\n", \
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
#undef CASE_TYPE
    default:
      mexErrMsgTxt("mexArrayToStruct() internal error, field type not recognized");
    }
  }
  *Pcount = count;
  return base;
}
