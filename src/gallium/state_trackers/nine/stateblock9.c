/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "stateblock9.h"

#define DBG_CHANNEL DBG_STATEBLOCK

HRESULT
NineStateBlock9_ctor( struct NineStateBlock9 *This,
                      struct NineUnknownParams *pParams,
                      struct NineDevice9 *pDevice,
                      enum nine_stateblock_type type )
{
    This->device = pDevice;
    This->type = type;

    return D3D_OK;
}

void
NineStateBlock9_dtor( struct NineStateBlock9 *This )
{
    struct nine_state *state = &This->state;
    unsigned i;

    nine_reference(&state->idxbuf, NULL);

    if (state->changed.vbuf) {
        for (i = 0; i < PIPE_MAX_ATTRIBS; ++i)
            nine_reference(&state->stream[i], NULL);
    }

    nine_reference(&state->vs, NULL);
    nine_reference(&state->fs, NULL);

    nine_reference(&state->vdecl, NULL);

    if (nine->vs_const_f)
        FREE(nine->vs_const_f);
    if (nine->ps_const_f)
        FREE(nine->ps_const_f);
}

HRESULT WINAPI
NineStateBlock9_GetDevice( struct NineStateBlock9 *This,
                           IDirect3DDevice9 **ppDevice )
{
    user_assert(ppDevice, E_POINTER);
    NineUnknown_AddRef(NineUnknown(This->device));
    *ppDevice = (IDirect3DDevice9 *)This->device;
    return D3D_OK;
}

/* Fast path for all state. */
static void
nine_state_transfer_all(struct nine_state *dst,
                        struct nine_state *src)
{
   /* TODO */
}

/* Copy state marked changed in @mask from @src to @dst.
 * If @apply is false, updating dst->changed can be omitted.
 * TODO: compare ?
 */
static void
nine_state_transfer(struct nine_state *dst,
                    const struct nine_state *src,
                    const struct nine_state *mask,
                    const boolean apply)
{
    unsigned i, j;

    if (mask->changed.group & NINE_STATE_VIEWPORT)
        dst->viewport = src->viewport;
    if (mask->changed.group & NINE_STATE_SCISSOR)
        dst->scissor = src->scissor;
    if (mask->changed.group & NINE_STATE_VS)
        nine_reference(&dst->vs, src->vs);
    if (mask->changed.group & NINE_STATE_PS)
        nine_reference(&dst->ps, src->ps);
    if (mask->changed.group & NINE_STATE_VDECL)
        nine_reference(&dst->vdecl, src->vdecl);
    if (mask->changed.group & NINE_STATE_IDXBUF)
        nine_reference(&dst->idxbuf, src->idxbuf);

    if (apply)
       dst->changed.group |= mask->changed.group;

    /* Vertex constants.
     *
     * Various possibilities for optimization here, like creating a per-SB
     * constant buffer, or memcmp'ing for changes.
     * Will do that later depending on what works best for specific apps.
     */
    if (mask->changed.group & NINE_STATE_VS_CONST) {
       for (i = 0; i < Elements(mask->changed.vs_const_f); ++i) {
          if (!mask->changed.vs_const_f[i])
             continue;
          if (apply)
             dst->changed.vs_const_f[i] |= mask->changed.vs_const_f[i];
          m = mask->changed.vs_const_f[i];

          if (m == 0xFFFFFFFF) {
             memcpy(dst->vs_const_f[i * 32 * 4],
                    src->vs_const_f[i * 32 * 4], 32 * 4 * sizeof(float));
             continue;
          }
          for (j = ffs(m) - 1, m >>= j; m; ++j, m >>= 1)
             if (m & 1)
                memcpy(dst->vs_const_f[(i * 32 + j) * 4],
                       src->vs_const_f[(i * 32 + j) * 4], 4 * sizeof(float));
       }
    }
    if (mask->changed.vs_const_i) {
       uint16_t m = mask->changed.vs_const_i;
       for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
          if (m & 1)
             dst->vs_const_i[i] = src->vs_const_i[i];
       if (apply)
          dst->changed.vs_const_i |= mask->changed.vs_const_i;
    }
    if (mask->changed.vs_const_b) {
       uint16_t m = mask->changed.vs_const_b;
       for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
          if (m & 1)
             dst->vs_const_b[i] = src->vs_const_b[i];
       if (apply)
          dst->changed.vs_const_b |= mask->changed.vs_const_b;
    }

    /* Pixel constants. */
    if (mask->changed.group & NINE_STATE_PS_CONST) {
       for (i = 0; i < Elements(mask->changed.ps_const_f); ++i) {
          if (!mask->changed.ps_const_f[i])
             continue;
          if (apply)
             dst->changed.ps_const_f[i] |= mask->changed.ps_const_f[i];
          m = mask->changed.ps_const_f[i];
          for (j = ffs(m) - 1, m >>= j; m; ++j, m >>= 1)
             if (m & 1)
                memcpy(dst->ps_const_f[(i * 32 + j) * 4],
                       src->ps_const_f[(i * 32 + j) * 4], 4 * sizeof(float));
       }
    }
    if (mask->changed.ps_const_i) {
       uint16_t m = mask->changed.ps_const_i;
       for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
          if (m & 1)
             dst->ps_const_i[i] = src->ps_const_i[i];
       if (apply)
          dst->changed.ps_const_i |= mask->changed.ps_const_i;
    }
    if (mask->changed.ps_const_b) {
       uint16_t m = mask->changed.ps_const_b;
       for (i = ffs(m) - 1, m >>= i; m; ++i, m >>= 1)
          if (m & 1)
             dst->ps_const_b[i] = src->ps_const_b[i];
       if (apply)
          dst->changed.ps_const_b |= mask->changed.ps_const_b;
    }

    /* Render states. */
    /* TODO: speed this up */
    for (i = 0; i <= NINED3DRS_LAST; ++i) {
        if (!mask->changed.rs[i / 32]) {
            i += 31;
            continue;
        }
        if (i % 32 == 0)
           i += ffs(mask->changed.mask[i / 32]) - 1;
        if (mask->changed.rs[i / 32] & (1 << (i % 32)))
            dst->rs[i] = src->rs[i];
    }
    if (apply) {
       for (i = 0; i < Elements(dst->changed.rs); ++i)
          dst->changed.rs[i] |= mask->changed.rs[i];
    }

    /* Vertex streams. */
    if (mask->changed.vbuf | mask->changed.stream_freq) {
        for (i = 0; i < PIPE_MAX_ATTRIBS) {
            if (mask->changed.vtxbuf & (1 << i))
                nine_reference(&dst->stream[i], src->stream[i]);
            if (mask->changed.stream_freq & (1 << i))
                dst->stream_freq[i] = src->stream_freq[i];
        }
        if (apply) {
           dst->changed.vtxbuf      |= mask->changed.vtxbuf;
           dst->changed.stream_freq |= mask->changed.stream_freq;
        }
    }

    /* Clip planes. */
    if (mask->changed.ucp) {
        for (i = 0; i < PIPE_MAX_CLIP_PLANES; ++i)
            if (mask->changed.ucp & (1 << i))
                dst->clip.ucp[i] = src->clip.ucp[i];
        if (apply)
           dst->changed.ucp |= mask->changed.ucp;
    }

    /* Texture and sampler state. */
    /* TODO */

    /* Fixed function state. */
    /* TODO */

    return D3D_OK;
}

/* Capture those bits of current device state that have been changed between
 * BeginStateBlock and EndStateBlock.
 */
HRESULT WINAPI
NineStateBlock9_Capture( struct NineStateBlock9 *This )
{
    nine_state_transfer(&This->state, &This->device->state, &This->state,
                        FALSE);
    return D3D_OK;
}

/* Set state managed by this StateBlock as current device state. */
HRESULT WINAPI
NineStateBlock9_Apply( struct NineStateBlock9 *This )
{
    if (This->type = NINESBT_ALL && 0) /* TODO */
       nine_state_transfer_all(&This->device->state, &This->state);
    nine_state_transfer(&This->device->state, &This->state, &This->state, TRUE);
    return D3D_OK;
}

IDirect3DStateBlock9Vtbl NineStateBlock9_vtable = {
    (void *)NineUnknown_QueryInterface,
    (void *)NineUnknown_AddRef,
    (void *)NineUnknown_Release,
    (void *)NineStateBlock9_GetDevice,
    (void *)NineStateBlock9_Capture,
    (void *)NineStateBlock9_Apply
};

static const GUID *NineStateBlock9_IIDs[] = {
    &IID_IDirect3DStateBlock9,
    &IID_IUnknown,
    NULL
};

HRESULT
NineStateBlock9_new( struct NineDevice9 *pDevice,
                     struct NineStateBlock9 **ppOut,
                     enum nine_stateblock_type type)
{
    NINE_NEW(NineStateBlock9, ppOut, pDevice, type);
}