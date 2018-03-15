/* pdp10_tu.c - ACC LH/DH IMP interface simulator

   Copyright (c) 2018, Lars Brinkhoff

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   ROBERT M SUPNIK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name of Robert M Supnik shall not be
   used in advertising or otherwise to promote the sale, use or other dealings
   in this Software without prior written authorization from Robert M Supnik.

   imp          ACC LH/DH IMP interface
*/

#include "pdp10_defs.h"
#include "sim_imp.h"

extern int32 ubmap[UBANUM][UMAP_MEMSIZE];               /* Unibus map */
extern int32 ubcs[UBANUM];

t_stat lhdh_rd (int32 *data, int32 PA, int32 access);
t_stat lhdh_wr (int32 data, int32 PA, int32 access);
int32 lhdh_inta (void);
t_stat lhdh_svc (UNIT *uptr);
t_stat lhdh_reset (DEVICE *dptr);

DIB lhdh_dib = {
    IOBA_LHDH, IOLN_LHDH, &lhdh_rd, &lhdh_wr,
    1, IVCL (LHDH), VEC_LHDH, { &lhdh_inta }, IOLN_LHDH,
    };

UNIT lhdh_unit[] = {
    { UDATA (&lhdh_svc, UNIT_DISABLE, 0) },
    };

REG lhdh_reg[] = {
    { NULL }
    };

MTAB lhdh_mod[] = {
    { MTAB_XTD|MTAB_VDV, 0, "ADDRESS", NULL,
      NULL, &show_addr, NULL },
    { MTAB_XTD|MTAB_VDV, 0, "VECTOR", NULL,
      NULL, &show_vec, NULL },
    { 0 }
    };

DEVICE lhdh_dev = {
    "LHDH", lhdh_unit, lhdh_reg, lhdh_mod,
    1, 8, 31, 1, 8, 8,
    NULL, NULL, &lhdh_reset, NULL, NULL, NULL,
    &lhdh_dib, DEV_UBUS | DEV_DEBUG | DEV_ETHER
    };

t_stat lhdh_rd (int32 *data, int32 PA, int32 access)
{
  fprintf (stderr, "LHDH: read from %o\r\n", PA);
  switch (PA) {
  case 03767600:
    *data = 0200;
    break;
  default:
    *data = 0;
    break;
  }
  return SCPE_OK;
}

t_stat lhdh_wr (int32 data, int32 PA, int32 access)
{
  fprintf (stderr, "LHDH: write to %o\r\n", PA);
  return SCPE_OK;
}

int32 lhdh_inta (void)
{
  return VEC_TU;
}

t_stat lhdh_svc (UNIT *uptr)
{
  return SCPE_OK;
}

t_stat lhdh_reset (DEVICE *dptr)
{
  return SCPE_OK;
}

