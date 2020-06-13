#define TMXR_VALID 0
#define SCPE_STALL 0

#define TT_GET_MODE(x) 0

typedef struct {
  int conn;
  int rcve;
} TMLN;

typedef struct {
  int a, b, c;
  TMLN *d;
} TMXR;

extern t_stat tmxr_show_summ (FILE *st, UNIT *up, int32 v, CONST void *dp);
extern t_stat tmxr_show_cstat (FILE *st, UNIT *up, int32 v, CONST void *dp);
extern t_stat tmxr_dscln (UNIT *, int32, CONST char *, void *);
extern t_stat tmxr_attach (TMXR *, UNIT *, CONST char *);
extern t_stat tmxr_poll_conn (TMXR *);
extern t_stat tmxr_poll_rx (TMXR *);
extern t_stat tmxr_poll_tx (TMXR *);
extern t_stat tmxr_txdone_ln (TMLN *);
extern t_stat tmxr_getc_ln (TMLN *);
extern t_stat tmxr_putc_ln (TMLN *, uint32);
extern uint32 sim_tt_inpcvt (uint32, uint32);
extern uint32 sim_tt_outcvt (uint32, uint32);
