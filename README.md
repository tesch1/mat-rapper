usage:

```
#include "mexCommons.h"

/*! just some c structure */
typedef struct {
  double d1;
  float f1;
  int i1;
  float f2[2];
} demo_struct;

/*! descriptions of all fields in some structure
 */
static struct_fielddesc_t ds_fields[] = {
  MC_FIELD_DEF(d1, TY_DOUBLE, demo_struct),
  MC_FIELD_DEF(f1, TY_FLOAT, demo_struct),
  MC_FIELD_DEF(i1, TY_S32, demo_struct),
  MC_FIELD_DEF(f2, TY_FLOAT, demo_struct),
  {NULL}
};

/* [a] = commonsdemo(b) */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  const char * usage = "usage: [a] = commonsdemo(b)\n";

  if (nrhs != 1 || nlhs != 1 || !mxIsStruct(prhs[0]))
    mexErrMsgTxt(usage);

  /* convert mxArray prhs[0] to c-struct in dp */
  mwSize count;
  demo_struct * dp = mexArrayToStruct(ds_fields, prhs[0], &count);

  /* for the demo, just truncate one entry */
  count--;

  /* convert it back */
  plhs[0] = mexStructToArray(ds_fields, count, dp);

  /* dp was allocated with malloc() in mexArrayToStruct(), free it */
  free(dp);
}
```
and to call that from matlab...
```
>> d
d = 

    d1: [3x1 double]
    f1: [3x1 double]
    i1: [3x1 double]
    f2: [3x2 double]


>> d.f2

ans =

    10    13
    11    14
    12    15

>> f = commonsdemo(d);
>> f

f = 

    d1: [2x1 double]
    f1: [2x1 single]
    i1: [2x1 int32]
    f2: [2x2 single]

>> f.f2

ans =

    10    13
    11    14

>> 
```