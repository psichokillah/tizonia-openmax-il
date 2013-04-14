/**
 * Copyright (C) 2011-2013 Aratelia Limited - Juan A. Rubio
 *
 * This file is part of Tizonia
 *
 * Tizonia is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Tizonia is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tizonia.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   webpdprc.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia OpenMAX IL - WebP Decoder processor class
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <limits.h>
#include <string.h>

#include "tizkernel.h"
#include "tizscheduler.h"

#include "webpdprc.h"
#include "webpdprc_decls.h"

#include "tizosal.h"

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.webp_decoder.prc"
#endif

/*
 * webpdprc
 */

static void *
webpd_proc_ctor (void *ap_obj, va_list * app)
{
  struct webpdprc *p_obj = super_ctor (webpdprc, ap_obj, app);
  TIZ_LOG (TIZ_TRACE, "Constructing webpdprc...[%p]", p_obj);

  p_obj->pinhdr_ = 0;
  p_obj->pouthdr_ = 0;
  p_obj->eos_ = false;

  return p_obj;
}

static void *
webpd_proc_dtor (void *ap_obj)
{
  struct webpdprc *p_obj = ap_obj;
  TIZ_LOG (TIZ_TRACE, "Destructing webpdprc...[%p]", p_obj);
  return super_dtor (webpdprc, ap_obj);
}

static OMX_ERRORTYPE
webpd_proc_transform_buffer (const void *ap_obj)
{
  struct webpdprc *p_obj = (struct webpdprc *) ap_obj;
  const tiz_servant_t *p_parent = ap_obj;
  (void) p_parent;
  (void) p_obj;
  return OMX_ErrorNone;
}

/*
 * from tiz_servant class
 */

static OMX_ERRORTYPE
webpd_proc_allocate_resources (void *ap_obj, OMX_U32 a_pid)
{
  struct webpdprc *p_obj = ap_obj;
  const tiz_servant_t *p_parent = ap_obj;
  assert (ap_obj);

  (void) p_parent;
  (void) p_obj;

  TIZ_LOG_CNAME (TIZ_TRACE,
                 TIZ_CNAME (p_parent->p_hdl_),
                 TIZ_CBUF (p_parent->p_hdl_),
                 "Resource allocation complete..." "pid = [%d]", a_pid);

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
webpd_proc_deallocate_resources (void *ap_obj)
{
  struct webpdprc *p_obj = ap_obj;
  const tiz_servant_t *p_parent = ap_obj;
  assert (ap_obj);

  (void) p_parent;
  (void) p_obj;

  TIZ_LOG_CNAME (TIZ_TRACE,
                 TIZ_CNAME (p_parent->p_hdl_),
                 TIZ_CBUF (p_parent->p_hdl_),
                 "Resource deallocation complete...");

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
webpd_proc_prepare_to_transfer (void *ap_obj, OMX_U32 a_pid)
{
  const tiz_servant_t *p_parent = ap_obj;
  assert (ap_obj);

  TIZ_LOG_CNAME (TIZ_TRACE,
                 TIZ_CNAME (p_parent->p_hdl_),
                 TIZ_CBUF (p_parent->p_hdl_),
                 "Transfering buffers...pid [%d]", a_pid);

  return OMX_ErrorNone;

}

static OMX_ERRORTYPE
webpd_proc_transfer_and_process (void *ap_obj, OMX_U32 a_pid)
{
  assert (ap_obj);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
webpd_proc_stop_and_return (void *ap_obj)
{
  struct webpdprc *p_obj = ap_obj;
  const tiz_servant_t *p_parent = ap_obj;

  assert (ap_obj);

  (void) p_obj;
  (void) p_parent;

  return OMX_ErrorNone;
}

/*
 * from tiz_proc class
 */

static bool
claim_input (const void *ap_obj)
{
  const tiz_servant_t *p_parent = ap_obj;
  struct webpdprc *p_obj = (struct webpdprc *) ap_obj;
  tiz_pd_set_t ports;
  void *p_krn = tiz_get_krn (p_parent->p_hdl_);

  TIZ_PD_ZERO (&ports);
  tiz_check_omx_err (tiz_kernel_select (p_krn, 2, &ports));

  /* We need one input buffers */
  if (TIZ_PD_ISSET (0, &ports))
    {
      tiz_check_omx_err (tiz_kernel_claim_buffer
                         (p_krn, 0, 0, &p_obj->pinhdr_));
      TIZ_LOG_CNAME (TIZ_TRACE, TIZ_CNAME (p_parent->p_hdl_),
                     TIZ_CBUF (p_parent->p_hdl_),
                     "Claimed INPUT HEADER [%p]...", p_obj->pinhdr_);
      return true;
    }

  TIZ_LOG_CNAME (TIZ_TRACE,
                 TIZ_CNAME (p_parent->p_hdl_),
                 TIZ_CBUF (p_parent->p_hdl_),
                 "COULD NOT CLAIM AN INPUT HEADER...");

  return false;
}

static bool
claim_output (const void *ap_obj)
{
  const tiz_servant_t *p_parent = ap_obj;
  struct webpdprc *p_obj = (struct webpdprc *) ap_obj;
  tiz_pd_set_t ports;
  void *p_krn = tiz_get_krn (p_parent->p_hdl_);

  TIZ_PD_ZERO (&ports);
  tiz_check_omx_err (tiz_kernel_select (p_krn, 2, &ports));

  /* We need one output buffers */
  if (TIZ_PD_ISSET (1, &ports))
    {
      tiz_check_omx_err (tiz_kernel_claim_buffer
                         (p_krn, 1, 0, &p_obj->pouthdr_));
      TIZ_LOG_CNAME (TIZ_TRACE, TIZ_CNAME (p_parent->p_hdl_),
                     TIZ_CBUF (p_parent->p_hdl_),
                     "Claimed OUTPUT HEADER [%p] BUFFER [%p] "
                     "nFilledLen [%d]...", p_obj->pouthdr_,
                     p_obj->pouthdr_->pBuffer, p_obj->pouthdr_->nFilledLen);
      return true;
    }

  return false;
}

static OMX_ERRORTYPE
webpd_proc_buffers_ready (const void *ap_obj)
{
  struct webpdprc *p_obj = (struct webpdprc *) ap_obj;
  const tiz_servant_t *p_parent = ap_obj;
  void *p_krn = tiz_get_krn (p_parent->p_hdl_);

  TIZ_LOG_CNAME (TIZ_TRACE,
                 TIZ_CNAME (p_parent->p_hdl_),
                 TIZ_CBUF (p_parent->p_hdl_), "Buffers ready...");

  while (1)
    {

      if (!p_obj->pinhdr_)
        {
          if (!claim_input (ap_obj) || !p_obj->pinhdr_)
            {
              break;
            }
        }

      if (!p_obj->pouthdr_)
        {
          if (!claim_output (ap_obj))
            {
              break;
            }
        }

      tiz_check_omx_err (webpd_proc_transform_buffer (ap_obj));
      if (p_obj->pinhdr_ && (0 == p_obj->pinhdr_->nFilledLen))
        {
          p_obj->pinhdr_->nOffset = 0;
          tiz_kernel_relinquish_buffer (p_krn, 0, p_obj->pinhdr_);
          p_obj->pinhdr_ = NULL;
        }
    }

  if (p_obj->eos_ && p_obj->pouthdr_)
    {
      /* EOS has been received and all the input data has been consumed
       * already, so its time to propagate the EOS flag */
      TIZ_LOG_CNAME (TIZ_TRACE,
                     TIZ_CNAME (p_parent->p_hdl_),
                     TIZ_CBUF (p_parent->p_hdl_),
                     "p_obj->eos OUTPUT HEADER [%p]...", p_obj->pouthdr_);
      p_obj->pouthdr_->nFlags |= OMX_BUFFERFLAG_EOS;
      tiz_kernel_relinquish_buffer (p_krn, 1, p_obj->pouthdr_);
      p_obj->pouthdr_ = NULL;
    }

  return OMX_ErrorNone;
}

/*
 * initialization
 */

const void *webpdprc;

void
init_webpdprc (void)
{

  if (!webpdprc)
    {
      TIZ_LOG (TIZ_TRACE, "Initializing webpdprc...");
      tiz_proc_init ();
      webpdprc =
        factory_new
        (tizproc_class,
         "webpdprc",
         tizproc,
         sizeof (struct webpdprc),
         ctor, webpd_proc_ctor,
         dtor, webpd_proc_dtor,
         tiz_proc_buffers_ready, webpd_proc_buffers_ready,
         tiz_servant_allocate_resources, webpd_proc_allocate_resources,
         tiz_servant_deallocate_resources, webpd_proc_deallocate_resources,
         tiz_servant_prepare_to_transfer, webpd_proc_prepare_to_transfer,
         tiz_servant_transfer_and_process, webpd_proc_transfer_and_process,
         tiz_servant_stop_and_return, webpd_proc_stop_and_return, 0);
    }

}
