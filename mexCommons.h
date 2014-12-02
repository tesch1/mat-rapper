/*
 * Copyright 2013 Regents of the University of Minnesota.
 * This file subject to terms of Creative Commons CC BY-NC 4.0, see LICENSE.html
 *
 * author(s): Michael Tesch (tesch1@gmail.com),
 */
#ifndef MEX_COMMONS_H
#define MEX_COMMONS_H
#include "mex.h"

#ifdef __cplusplus
extern "C" {
#endif

enum _ftype {TY_U32, TY_U16, TY_S16, TY_FLOAT, TY_S32, TY_DOUBLE};

/*! descriptions of a field in a structure
 */
typedef struct {
  const char * name;    /*!<    field name */
  size_t offset;        /*!<    byte offset from start of struct */
  size_t size;          /*!<    field size in bytes */
  enum _ftype type;     /*!<    type of data stored in field */
} struct_fielddesc_t;

mxArray * mexStructToArray(int nfields, struct_fielddesc_t * sfields, size_t count, void * data);
void * mexArrayToStruct(int nfields, struct_fielddesc_t * sfields, const mxArray * array, mwSize * count);

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

/* class for redirecting cout to matlab's output stream */
class mexstream : public std::streambuf {
public:
  /* call at beginning of mexFunction() */
  static void install() {
    oldoutbuf(std::cout.rdbuf(&getmexout()));
  }
  static void restore() {
    std::cout.rdbuf(oldoutbuf());
  }
 private:
  static mexstream & getmexout() {
    static mexstream mexout;
    return mexout;
  }
  static std::streambuf * oldoutbuf(std::streambuf * oldbuf = NULL) {
    static std::streambuf * oldoutbuf;
    std::streambuf * tmp;
    tmp = oldoutbuf;
    oldoutbuf = oldbuf;
    return tmp;
  }
protected:
  virtual std::streamsize xsputn(const char *s, std::streamsize n) {
    mexPrintf("%.*s",n,s);
    return n;
  }
  virtual int overflow(int c = EOF) {
    if (c != EOF) {
      mexPrintf("%.1s",&c);
    }
    return 1;
  }
};

#endif
#endif /* MEX_COMMON_H */
