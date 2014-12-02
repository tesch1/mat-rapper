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

/*! descriptions of all fields in an varian main header
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
  char * usage = 
    "usage: [a] = commonsdemo(b)\n";

  if (nrhs != 1 || nlhs != 1 || !mxIsStruct(prhs[0]))
    mexErrMsgTxt(usage);

  /* convert mxArray prhs[0] to c-struct in dp */
  mwSize count;
  demo_struct * dp = mexArrayToStruct(ds_fields, prhs[0], &count);

  /* for the demo, just truncate one entry */
  count--;

  /* convert it back */
  plhs[0] = mexStructToArray(ds_fields, count, dp);
}
```