#ifdef USE_DISPLAY
/* pdp11_tv.c: TV, Logo TV raster display

   Copyright (c) 2019, Lars Brinkhoff

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
   THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the names of the authors shall not be
   used in advertising or otherwise to promote the sale, use or other dealings
   in this Software without prior written authorization from the authors.
*/

#include "pdp11_defs.h"
#include "sim_video.h"

t_stat tv_rd(int32 *data, int32 PA, int32 access);
t_stat tv_wr(int32 data, int32 PA, int32 access);
const char *tv_description (DEVICE *dptr);

#define IOLN_TV   064
DIB tv_dib = {
  IOBA_AUTO, IOLN_TV, &tv_rd, &tv_wr,
  0, 0, 0, {NULL}, IOLN_TV
};

UNIT tv_unit = {
  UDATA (NULL, 0, 0), 0
};

REG tv_reg[] = {

};

MTAB tv_mod[] = {
  { MTAB_XTD|MTAB_VDV|MTAB_VALR, 020, "ADDRESS", "ADDRESS",
    &set_addr, &show_addr, NULL, "Bus address" },
  { MTAB_XTD|MTAB_VDV|MTAB_VALR,   0, "VECTOR",  "VECTOR",
    &set_vec,  &show_vec, NULL, "Interrupt vector" },
  { MTAB_XTD|MTAB_VDV, 0, NULL, "AUTOCONFIGURE",
    &set_addr_flt, NULL, NULL, "Enable autoconfiguration of address & vector" },
  { 0 }  };

#define DBG_IO        0001

DEBTAB tv_deb[] = {
  { "IO", DBG_IO, "trace" },
  { NULL, 0 }
};

DEVICE tv_dev = {
    "TV", &tv_unit, tv_reg, tv_mod,
    1, 8, 16, 1, 8, 16,
    NULL, NULL, NULL,
    NULL, NULL, NULL,
    &tv_dib, DEV_DIS | DEV_DISABLE | DEV_UBUS | DEV_DEBUG,
    0, tv_deb, NULL, NULL, NULL, NULL, NULL, 
    &tv_description
};

t_stat
tv_rd(int32 *data, int32 PA, int32 access)
{
  t_stat stat = SCPE_OK;
  switch (PA & 0177) {
  case 0142:
    sim_debug (DBG_IO, &tv_dev, "READ TVSEL\n");
    break;
  default:
    sim_debug (DBG_IO, &tv_dev, "READ %06o\n", PA);
    break;
  }
  return stat;
}

t_stat
tv_wr(int32 data, int32 PA, int32 access)
{
    switch (PA & 0177) {
    case 0142:
      sim_debug (DBG_IO, &tv_dev, "WRITE TVSEL %06o\n", data);
      break;
    default:
      sim_debug (DBG_IO, &tv_dev, "WRITE %06o %06o\n", PA, data);
      break;
    }
  return SCPE_OK;
}

const char *tv_description (DEVICE *dptr)
{
  return "Raster display controller for MIT Logo PDP-11/45";
}

#else  /* USE_DISPLAY not defined */
char pdp11_tv_unused;   /* sometimes empty object modules cause problems */
#endif /* USE_DISPLAY not defined */
