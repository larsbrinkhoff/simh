/* pdp11_xgp.c: PDP-11 harware interface to Xerox Graphics Printer.

   Copyright (c) 2018, Lars Brinkohff

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

   xgp          Xerox Graphics Printer
*/

#include "pdp11_defs.h"

#define XGP_VEC         0370

/* XGP status bits */
#define FOS     0100000 /* overscan */
#define FSYN    0040000 /* sync error */
#define FOR     0020000 /* dma overrun */
#define FNXM    0010000 /* nxm */
#define FRDYC   0004000 /* dev rdy changed */
#define FBCB    0002000 /* bad control byte in char mode */
#define RDMASK  0000274 /* mask for printer status bits */
#define FRDY    0000100 /* device ready */
#define FACT    0000001 /* active */

/* XGP control bits */
#define FERR    0100000 /* error */
#define FMOT    0002000 /* motion */
#define FDONE   0000200 /* done */
#define FDIE    0000100 /* interupt enable */
#define FCUTI   0000004 /* cut paper now! */
#define FCUT    0000002 /* cut paper */
#define FGO     0000001 /* go */

static int16 xcr = 0;           /* control */
static int16 xsr = 0;           /* status */
static int16 mar = 0;           /* memory address */
static int16 xcut = 0;          /* make a cut */

t_stat xgp_rd (int32 *data, int32 PA, int32 access);
t_stat xgp_wr (int32 data, int32 PA, int32 access);
t_stat xgp_svc (UNIT *uptr);
t_stat xgp_reset (DEVICE *dptr);
t_stat xgp_attach (UNIT *uptr, CONST char *ptr);
t_stat xgp_detach (UNIT *uptr);
t_stat xgp_help (FILE *st, DEVICE *dptr, UNIT *uptr, int32 flag, const char *cptr);
const char *xgp_description (DEVICE *dptr);

/* XGP data structures

   xgp_dev      XGP device descriptor
   xgp_unit     XGP unit descriptor
   xgp_reg      XGP register list
*/

#define IOLN_XGP        004

DIB xgp_dib = {
    IOBA_AUTO, IOLN_XGP, &xgp_rd, &xgp_wr,
    1, IVCL (XGP), VEC_AUTO, { NULL }
    };

UNIT xgp_unit = {
    UDATA (&xgp_svc, UNIT_SEQ+UNIT_ATTABLE+UNIT_TEXT, 0), SERIAL_OUT_WAIT
    };

REG xgp_reg[] = {
    { GRDATA (BUF, xgp_unit.buf, DEV_RDX, 8, 0) },
    { GRDATA (XCR, xcr, DEV_RDX, 16, 0) },
    { GRDATA (XSR, xsr, DEV_RDX, 16, 0) },
    { GRDATA (MAR, mar, DEV_RDX, 16, 0) },
    { GRDATA (XCUT, xcut, DEV_RDX, 16, 0) },
    { NULL }
    };

MTAB xgp_mod[] = {
    { MTAB_XTD|MTAB_VDV|MTAB_VALR, 004, "ADDRESS", "ADDRESS",
      &set_addr, &show_addr, NULL, "Bus address" },
    { MTAB_XTD|MTAB_VDV|MTAB_VALR, 0, "VECTOR", "VECTOR",
      &set_vec, &show_vec, NULL, "Interrupt vector" },
    { 0 }
    };

DEVICE xgp_dev = {
    "XGP", &xgp_unit, xgp_reg, xgp_mod,
    1, 10, 31, 1, DEV_RDX, 8,
    NULL, NULL, &xgp_reset,
    NULL, &xgp_attach, &xgp_detach,
    &xgp_dib, DEV_DISABLE | DEV_UBUS, 0,
    NULL, NULL, NULL, &xgp_help, NULL, NULL, 
    &xgp_description
    };

/* XGP routines

   xgp_rd       I/O page read
   xgp_wr       I/O page write
   xgp_svc      process event (printer ready)
   xgp_reset    process reset
   xgp_attach   process attach
   xgp_detach   process detach
*/

#if 0
	BIT XSR,#FRDY
	BEQ XGPST1

XGPRDY:	BIT #RDMASK,XSR
	BEQ XRDYX	;EVERYTHING OK
ABORT:	BIS XSR,XGPE	;ERROR FLAGS INTO XGPE

XGPERR:	MOV XSR,R0
	BIS R0,XGPE
	BIT #FRDYC,R0	;READY CHANGED?
	BEQ XGPD1	;NOPE, IGNORE ERROR
	BIT #FRDY,R0	;READY UP?
#endif

t_stat xgp_rd (int32 *data, int32 PA, int32 access)
{
    switch (PA & 060) {
    case 00:
        *data = xcr;
        break;
    case 02:
        *data = xsr;
        break;
    case 04:
        *data = mar;
        break;
    case 06:
        *data = xcut;
        break;
    }
    return SCPE_OK;
}

#if 0
	CLR XCR 	;STOP THE XGP

	MOV #FMOT,XCR	;START IT UP

	MOV #FDIE+FGO+FMOT,XCR	;ENABLE INTERRUPTS, AND GO

	TST XCR
	BLT XGPERR	;SIGN BIT OF XCR IS ERROR FLAG


#endif

t_stat xgp_wr (int32 data, int32 PA, int32 access)
{
    switch (PA & 060) {
    case 00:
        xcr = data;
        if (xcr & FDIE)
            SET_INT (XGP);
        else
            CLR_INT (XGP);
        if (xcr & FMOT)
            xsr |= ~FRDY;
        else
            xsr &= ~FRDY;
        if (xcr & FGO) {
            if (xsr & FRDY)
                sim_activate (&xgp_unit, 1000);
            else {
                sim_cancel (&xgp_unit);
                xcr |= FERR;
            }
        } else
            sim_cancel (&xgp_unit);
        break;
    case 02:
        xsr = data;
        break;
    case 04:
        mar = data;
        break;
    case 06:
        if (data & FCUTI)
            ; // Write page.
        CLR_INT (XGP);
        xcr &= ~(FMOT|FDONE|FGO);
        xsr &= ~FRDY;
        sim_cancel (&xgp_unit);
        break;
    }
    return SCPE_OK;
}

t_stat xgp_svc (UNIT *uptr)
{
    int data;

    /* It takes 7.75 milliseconds to print one scan line. */
    for (;;) {
        if (!ADDR_IS_MEM (mar)) {
            xsr |= FNXM;
            xcr |= FERR;
            return SCPE_NXM;
        }

        data = RdMemW (mar) & 0177777;
        mar += 2;

        fputc ((data >> 8) & 0377, uptr->fileref);
        fputc (data & 0377, uptr->fileref);

        //uptr->pos = ftell (uptr->fileref);
        //if (ferror (uptr->fileref)) {

        mar += 2;
    }

    sim_cancel (&xgp_unit);
    xcr |= FDONE;
    xcr &= ~(FMOT|FGO);
    xsr &= ~FRDY;
    if (xcr & FDIE)
        SET_INT (XGP);

    return SCPE_OK;
}

t_stat xgp_reset (DEVICE *dptr)
{
    xcr = 0;
    xsr = 0;
    mar = 0;
    xcut = 0;
    CLR_INT (XGP);
    if ((xgp_unit.flags & UNIT_ATT) == 0)
        xcr |= FERR;
    sim_cancel (&xgp_unit);
    return SCPE_OK;
}

t_stat xgp_attach (UNIT *uptr, CONST char *cptr)
{
    t_stat reason = attach_unit (uptr, cptr);
    xgp_reset (&xgp_dev);
    return reason;
}

t_stat xgp_detach (UNIT *uptr)
{
    return detach_unit (uptr);
}

t_stat xgp_help (FILE *st, DEVICE *dptr, UNIT *uptr, int32 flag, const char *cptr)
{
    fprintf (st, "Xerox Grapihcs Printer (XGP)\n\n");
    fprintf (st, "XGP writes data to a disk file.\n");
    fprint_set_help (st, dptr);
    fprint_show_help (st, dptr);
    fprint_reg_help (st, dptr);
    return SCPE_OK;
}

const char *xgp_description (DEVICE *dptr)
{
    return "XGP Xerox Graphics Printer";
}
