#include "sim_defs.h"
#include "sim_tmxr.h"
#include "sim_video.h"

int32 Fgetc (FILE *f)
{
  return 0;
}

uint32 sim_switches;

t_stat sim_messagef (t_stat x, CONST char *y, ...)
{
  return SCPE_OK;
}

t_value get_uint (CONST char *s, int a, unsigned b, t_stat *c)
{
  return SCPE_OK;
}

t_stat sim_set_idle (UNIT *uptr, int32 val, CONST char *cptr, void *desc)
{
  return SCPE_OK;
}

t_stat sim_clr_idle (UNIT *uptr, int32 val, CONST char *cptr, void *desc)
{
  return SCPE_OK;
}

t_stat sim_show_idle (FILE *st, UNIT *up, int32 v, CONST void *dp)
{
  return SCPE_OK;
}

t_stat sim_debug (int a, DEVICE *dptr, CONST char *b, ...)
{
  return SCPE_OK;
}

t_stat sim_process_event (void)
{
  return SCPE_OK;
}

t_stat sim_brk_test (t_addr a, uint32 b)
{
  return SCPE_OK;
}

t_bool (*sim_vm_is_subroutine_call) (t_addr **);

t_stat sim_activate_abs (UNIT *uptr, uint32 n)
{
  return SCPE_OK;
}

t_stat sim_activate_after (UNIT *uptr, uint32 n)
{
  return SCPE_OK;
}

t_stat sim_cancel (UNIT *uptr)
{
  return SCPE_OK;
}

t_bool sim_is_active (UNIT *uptr)
{
  return SCPE_OK;
}

t_stat sim_poll_kbd (void)
{
  return SCPE_OK;
}

t_stat set_cmd (uint32 n, CONST char *x)
{
  return SCPE_OK;
}

uint32 sim_fread (void *a, int b, int c, FILE *f)
{
  return 0;
}

t_stat attach_unit (UNIT *uptr, CONST char *s)
{
  return SCPE_OK;
}

t_stat detach_unit (UNIT *uptr)
{
  return SCPE_OK;
}
