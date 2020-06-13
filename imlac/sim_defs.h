#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE 1

#define CONST const

#define SCPE_OK      1
#define SCPE_IOERR  10
#define SCPE_CSUM   11
#define SCPE_ARG    12
#define SCPE_STEP   13
#define SCPE_NXM    14
#define SCPE_STOP   15
#define SCPE_BREAK  16
#define SCPE_KFLAG  0x10000

#define UDATA(a,b,c)
#define ORDATAD(a,b,c,d)

#define SWMASK(X) (1<<((X)&037))

#define MTAB_XTD 0
#define MTAB_VDV 0
#define MTAB_VALR 0
#define MTAB_NMO 0
#define UNIT_ATT 0

#define DEV_DEBUG 0
#define DEV_DISABLE 0
#define DEV_DIS 0

#define AIO_CHECK_EVENT do { } while (0)

typedef unsigned short uint16;
typedef unsigned int uint32;
typedef int int32;
typedef uint32 t_addr;
typedef int32 t_value;
typedef int t_bool;
typedef int t_stat;

typedef struct {
  FILE *fileref;
  uint32 flags;
} UNIT;

typedef struct {
  uint32 *loc;
} REG;

typedef struct {
  int a, b;
  CONST char *c, *d;
  t_stat (*e) (UNIT *uptr, int32 val, CONST char *cptr, void *desc);
  t_stat (*f) (FILE *st, UNIT *up, int32 v, CONST void *dp);
  void *g;
  CONST char *h;
} MTAB;

typedef struct {
  CONST char *a;
  int b;
} DEBTAB;

typedef struct DEVICE DEVICE;

struct DEVICE {
  CONST char *a;
  UNIT *b;
  REG *c;
  MTAB *d;
  int v, w, e, f, g, h;
  t_stat (*i) (t_value *vptr, t_addr ea, UNIT *uptr, int32 sw);
  t_stat (*j) (t_value val, t_addr ea, UNIT *uptr, int32 sw);
  t_stat (*k) (DEVICE *dptr);
  void *l, *m, *n;
  void *ctxt;
  uint32 flags;
  DEBTAB *p;
  void *q, *r, *s, *t, *u;
};

extern int32 Fgetc (FILE *);
extern uint32 sim_switches;
extern t_stat sim_messagef (t_stat, CONST char *, ...);
extern t_value get_uint (CONST char *, int, unsigned, t_stat *);
extern t_stat sim_set_idle (UNIT *uptr, int32 val, CONST char *cptr, void *desc);
extern t_stat sim_clr_idle (UNIT *uptr, int32 val, CONST char *cptr, void *desc);
extern t_stat sim_show_idle (FILE *st, UNIT *up, int32 v, CONST void *dp);
extern t_stat sim_debug (int, DEVICE *, CONST char *, ...);
extern t_stat sim_process_event (void);
extern t_stat sim_brk_test (t_addr, uint32);
extern t_bool (*sim_vm_is_subroutine_call) (t_addr **);
extern t_stat sim_activate_abs (UNIT *, uint32);
extern t_stat sim_activate_after (UNIT *, uint32);
extern t_stat sim_cancel (UNIT *);
extern t_bool sim_is_active (UNIT *);
extern t_stat sim_poll_kbd (void);
extern t_stat set_cmd (uint32, CONST char *);
extern REG *sim_PC;
extern uint32 sim_fread (void *, int, int, FILE *);
extern t_stat attach_unit (UNIT *, CONST char *);
extern t_stat detach_unit (UNIT *);

extern uint32 sim_interval;
extern uint32 sim_brk_summ;
extern uint32 sim_brk_types;
extern uint32 sim_brk_dflt;
extern uint32 sim_step;
