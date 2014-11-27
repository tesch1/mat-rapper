/*
 * Copyright 2013 Regents of the University of Minnesota.
 * This file subject to terms of Creative Commons CC BY-NC 4.0, see LICENSE.html
 *
 * author(s): Michael Tesch (tesch1@gmail.com),
 */
#ifndef MEX_COMMONS_H
#define MEX_COMMONS_H

#ifdef __cplusplus
extern "C" {
#endif

enum _ftype {TY_U32, TY_U16, TY_FLOAT, TY_S32, TY_DOUBLE};

/*! descriptions of a field in a structure
 */
typedef struct {
  const char * name;    /*!<    field name */
  size_t offset;        /*!<    byte offset from start of struct */
  size_t size;          /*!<    field size in bytes */
  enum _ftype type;     /*!<    type of data stored in field */
} struct_fielddesc_t;

mxArray * mexStructArray(int nfields, struct_fielddesc_t * sfields, size_t count, void * data);
void * mexArrayStruct(int nfields, struct_fielddesc_t * sfields, const mxArray * array, mwSize * count);

int mexCommonsPrintfCallback(const char *format, ...);

#define MC_FIELD_DEF(fname, ftype, structtype) {#fname, (size_t)&((structtype*)NULL)->fname, \
      sizeof(((structtype*)NULL)->fname), ftype}

#ifndef NARRAY
#define NARRAY(a) ((sizeof a) / sizeof (a)[0])
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifdef __cplusplus
}
#endif
#endif /* MEX_COMMON_H */
