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

/* Status and control register .*/
#define LHGO       01  /* GO - Start DMA Transfer */
#define LHRST      02  /* Interface Reset */
#define LHA16     060  /* Address bits 17 and 16 for extended unibus xfrs */
#define LHIE     0100  /* Interrupt Enable */
#define LHRDY    0200  /* Device Ready */
#define LHMRE   01000  /* Master Ready Error */
#define LHNXM  040000  /* Non Existant Memory on DMA */
#define LHERR 0100000  /* Error present */

/* Input status and control. */
#define LHHRC      04  /* Host Ready Relay Control */
#define LHSE      010  /* Store Enable */
#define LHIB     0400  /* Input Buffer Full */
#define LHINR   02000  /* IMP not ready */
#define LHHR    04000  /* Host Ready */
#define LHEOM  020000  /* End-of-Message received from IMP */

/* Output status and control. */
#define LHELB      04  /* Send EOM indication to IMP at end of xfr */
#define LHBB      010  /* Bus Back */
#define LHOBE    0400  /* Output Buffer Empty */
#define LHWC0  020000  /* Output Word Count is zero */

uint32 lhics; /* Input control and status */
d10    lhidb; /* Input data buffer */
uint32 lhica; /* Input current address */
uint32 lhiwc; /* Input word count */
uint32 lhocs; /* Output control and status */
d10    lhodb; /* Output data buffer */
uint32 lhoca; /* Output current address */
uint32 lhowc; /* Output word count */

int req_in_int;
int req_out_int;
int ibits;

extern int32 ubmap[UBANUM][UMAP_MEMSIZE];               /* Unibus map */
extern int32 ubcs[UBANUM];

t_stat lhdh_rd (int32 *data, int32 PA, int32 access);
t_stat lhdh_wr (int32 data, int32 PA, int32 access);
int32 lhdh_inta (void);
t_stat lhdh_svc (UNIT *uptr);
t_stat lhdh_reset (DEVICE *dptr);
static t_stat receive_word (void);

DIB lhdh_dib = {
    IOBA_LHDH, IOLN_LHDH, &lhdh_rd, &lhdh_wr,
    1, IVCL (LHDH), VEC_LHDH, { &lhdh_inta }, IOLN_LHDH,
    };

UNIT lhdh_unit[] = {
    { UDATA (&lhdh_svc, UNIT_DISABLE, 0) },
    };

REG lhdh_reg[] = {
    { ORDATAD (LHICS, lhics, 16, "input control/status") },
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
    &lhdh_dib, DEV_UBUS | DEV_DEBUG | DEV_ETHER | DEV_DISABLE | DEV_DIS
    };

IMP imp;

t_stat lhdh_rd (int32 *data, int32 PA, int32 access)
{
  //fprintf (stderr, "LHDH: read from %o\r\n", PA);
  switch (PA) {
  case 03767600: /* Input Control and Status */
    *data = lhics;
    //fprintf (stderr, "LHDH: Read input CS = %o\r\n", *data);
    break;
  case 03767602: /* Input Data Buffer */
    *data = lhidb;
    lhics &= ~LHIB;
    ibits = 0;
    //fprintf (stderr, "LHDH: Read input DB = %o\r\n", *data);
    break;
  case 03767604: /* Input Current Word Address */
    *data = lhica;
    //fprintf (stderr, "LHDH: Read input CA = %o\r\n", *data);
    break;
  case 03767606: /* Input Word Count */
    *data = lhiwc;
    //fprintf (stderr, "LHDH: Read input WC = %o\r\n", *data);
    break;
  case 03767610: /* Output Control and Status */
    *data = lhocs;
    //fprintf (stderr, "LHDH: Read output CS = %o\r\n", *data);
    break;
  case 03767612: /* Output Data Buffer */
    *data = lhodb;
    //fprintf (stderr, "LHDH: Read output DB = %o\r\n", *data);
    break;
  case 03767614: /* Output Current Word Address */
    *data = lhoca;
    //fprintf (stderr, "LHDH: Read output CA = %o\r\n", *data);
    break;
  case 03767616: /* Output Word Count */
    *data = lhowc;
    //fprintf (stderr, "LHDH: Read output WC = %o\r\n", *data);
    break;
  default:
    *data = 0;
    break;
  }
  return SCPE_OK;
}

t_stat lhdh_wr (int32 data, int32 PA, int32 access)
{
  switch (PA) {
  case 03767600: /* Input Control and Status */
    //fprintf (stderr, "LHDH: Write input CS: %06o\r\n", data);

#if 0
    if (data & LHGO)
      fprintf (stderr, "Start DMA\r\n");
    if (data & LHIE)
      fprintf (stderr, "Interrupt enable\r\n");
    if (data & LHRDY)
      fprintf (stderr, "Ready\r\n");
    if (data & LHMRE)
      fprintf (stderr, "Master ready\r\n");
    if (data & LHSE)
      fprintf (stderr, "Store enable\r\n");
    if (data & LHHRC)
      fprintf (stderr, "Host ready relay\r\n");
    if (data & LHHR)
      fprintf (stderr, "Host ready\r\n");
#endif

    lhics &= ~(LHGO|LHRST|LHA16|LHIE|LHHRC|LHSE|LHHR);
    lhics |= data & (LHGO|LHRST|LHA16|LHIE|LHHRC|LHSE|LHHR);
    if (data & LHHRC)
      lhics |= LHHR;
    if (data & LHGO)
      lhics &= ~(LHEOM|LHRDY);
    if (ibits == 16) {
      //fprintf (stderr, "Receive word in lhdh_wr\r\n");
      receive_word ();
    }
    break;
  case 03767602: /* Input Data Buffer */
    //fprintf (stderr, "LHDH: Write input DB: %06o\r\n", data);
    lhidb = data;
    break;
  case 03767604: /* Input Current Word Address */
    //fprintf (stderr, "LHDH: Write input CA: %06o\r\n", data);
    lhica = data;
    break;
  case 03767606: /* Input Word Count */
    lhiwc = data;
    break;
  case 03767610: /* Output Control and Status */
    //fprintf (stderr, "LHDH: Write output CS: %06o\r\n", data);

#if 0
    if (data & LHGO)
      fprintf (stderr, "Start DMA\r\n");
    if (data & LHELB)
      fprintf (stderr, "Last bit\r\n");
    if (data & LHA16)
      fprintf (stderr, "Address 16-17\r\n");
    if (data & LHIE)
      fprintf (stderr, "Interrupt enable\r\n");
    if (data & LHRDY)
      fprintf (stderr, "Ready\r\n");
    if (data & LHBB)
      fprintf (stderr, "Bus back\r\n");
    if (data & LHOBE)
      fprintf (stderr, "Buffer empty\r\n");
    if (data & LHWC0)
      fprintf (stderr, "Count zero\r\n");
#endif

    lhocs &= ~(LHGO|LHRST|LHA16|LHIE|LHELB|LHBB);
    lhocs |= data & (LHGO|LHRST|LHA16|LHIE|LHELB|LHBB);
    if (data & LHGO)
      lhocs &= ~(LHOBE|LHRDY);

    break;
  case 03767612: /* Output Data Buffer */
    //fprintf (stderr, "LHDH: Write output DB: %06o\r\n", data);
    lhodb = data;
    break;
  case 03767614: /* Output Current Word Address */
    //fprintf (stderr, "LHDH: Write output CA: %06o\r\n", data);
    lhoca = data;
    break;
  case 03767616: /* Output Word Count */
    lhowc = data;
    break;
  default:
    break;
  }

  lhdh_svc (lhdh_unit);

  return SCPE_OK;
}

int32 lhdh_inta (void)
{
  if (req_in_int && (lhics & (LHEOM | LHIB))) {
    lhics &= ~LHIE;
    req_in_int = 0;
    return VEC_LHDH;
  } else if (req_out_int && lhowc == 0200000) {
    //fprintf (stderr, "LHDH: interupt acknowledge OUTPUT\r\n");
    lhocs &= ~LHIE;
    req_out_int = 0;
    return VEC_LHDH + 4;
  } else {
    //fprintf (stderr, "LHDH: interupt acknowledge UNKNOWN\r\n");
    return -1;
  }
}

t_stat lhdh_svc (UNIT *uptr)
{
  if ((lhics & LHRST) || (lhocs & LHRST))
    lhdh_reset (find_dev_from_unit (uptr));

  if (lhocs & LHGO) {
    //fprintf (stderr, "LHDH: output DMA, %o %o\r\n", lhoca, lhowc);

    if (lhowc < 0200000) {
      int last = (lhowc == 0177777);
      int page, map;
      int x;

      //fprintf (stderr, "%o -> %o -> %o\n\r",
      //         lhoca, lhoca >> 2, ubmap[1][lhoca >> 11]);

      page = lhoca >> 11;
      map = ubmap[1][page];
      if (page >= UMAP_MEMSIZE || !(map & UMAP_VLD))
        fprintf (stderr, "LHDH: invalid Unibus mapping\r\n");

      x = (map & 03777) + ((lhoca >> 2) & 0777);
      lhodb = Read (x, 1);
      //fprintf (stderr, "address %o, data %012llo\r\n", x, lhodb);
      
      if ((lhowc & 1) == 0)
        lhodb >>= 18;
      imp_send_bits (&imp, lhodb & 0377, 8, 0);
      imp_send_bits (&imp, (lhodb >> 8) & 0377, 8, last);
      lhowc++;
      lhoca += 2;
    } else {
      lhocs &= ~LHGO;
      lhocs |= LHRDY;
      if (lhocs & LHIE) {
        int_req |= INT_LHDH;
        //fprintf (stderr, "LHDH: request OUTPUT interrupt (DMA)\r\n");
        req_out_int = 1;
      }
    }
  }

  sim_activate (uptr, 10000);
  return SCPE_OK;
}

static t_stat receive_word (void)
{
  //fprintf (stderr, "LHDH: receive word\r\n");
  if (lhics & LHGO) {
    int page, x, map;

    //fprintf (stderr, "LHDH: receive through DMA\r\n");

    page = lhica >> 11;
    map = ubmap[1][page];
    if (page >= UMAP_MEMSIZE || !(map & UMAP_VLD))
      fprintf (stderr, "LHDH: invalid Unibus mapping\r\n");

    lhidb = ((lhidb & 0377) << 8) + ((lhidb >> 8) & 0377);

    x = (map & 03777) + ((lhica >> 2) & 0777);
    if (lhiwc & 1) {
      lhidb += ReadP (x) & 0777777000000LL;
      WriteP (x, lhidb);
    } else {
      lhidb = (lhidb << 18) + (ReadP (x) & 0777777);
      WriteP (x, lhidb);
    }

    lhica += 2;
    lhiwc++;
    ibits = 0;
    lhidb = 0;
  } else {
    //fprintf (stderr, "LHDH: input buffer full\r\n");
    lhics |= LHIB;
    if (lhics & LHIE) {
      //fprintf (stderr, "LHDH: request INPUT interrupt (buf full)\r\n");
      int_req |= INT_LHDH;
      req_in_int = 1;
    }
  }
}

t_stat ldhd_receive_bit (int bit, int last)
{
  //fprintf (stderr, "IMP wants host to receive a bit.\r\n");

  if ((lhics & LHSE) == 0) {
    //fprintf (stderr, "LHDH: don't want it\r\n");
    return SCPE_ARG;
  };

  if (ibits == 16) {
    //fprintf (stderr, "LHDH: buffer full\r\n");
    return SCPE_ARG;
  }

  lhidb <<= 1;
  lhidb |= bit & 1;
  ibits++;

  if (last && ibits < 16) {
    lhidb <<= 16 - ibits;
    ibits = 16;
  }

  if (ibits == 16)
    receive_word ();
    
  if (last) {
    lhics |= (LHEOM|LHRDY);
    lhics &= ~LHGO;
    if (lhics & LHIE) {
      //fprintf (stderr, "LHDH: request INPUT interrupt (DMA)\r\n");
      int_req |= INT_LHDH;
      req_in_int = 1;
    }
  }

  return SCPE_OK;
}

t_stat lhdh_reset (DEVICE *dptr)
{
  //fprintf (stderr, "LHDH: reset\r\n");

  imp_reset (&imp);
  imp.receive_bit = ldhd_receive_bit;

  req_in_int = 0;
  req_out_int = 0;

  ibits = 0;
  lhidb = 0;

  lhics = LHRDY; /* Ready */
  lhidb = 0;
  lhica = 0;
  lhiwc = 0;
  lhocs = LHRDY; /* Ready */
  lhodb = 0;
  lhoca = 0;
  lhowc = 0;

  //sim_cancel (lhdh_unit);
  sim_activate (lhdh_unit, 10000);
  return SCPE_OK;
}
