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

#ifndef _NINE_RESOURCE9_H_
#define _NINE_RESOURCE9_H_

#include "iunknown.h"

struct pipe_screen;
struct pipe_resource;
struct util_hash_table;
struct NineDevice9;

struct NineResource9
{
    struct NineUnknown base;

    /* resource related */
    struct pipe_screen *screen;
    struct pipe_resource *resource;
    D3DRESOURCETYPE type;
    D3DPOOL pool;
    DWORD priority;

    /* creator device */
    struct NineDevice9 *device;

    /* for [GS]etPrivateData/FreePrivateData */
    struct util_hash_table *pdata;
};
static INLINE struct NineResource9 *
NineResource9( void *data )
{
    return (struct NineResource9 *)data;
}

HRESULT
NineResource9_ctor( struct NineResource9 *This,
                    struct NineUnknownParams *pParams,
                    struct NineDevice9 *pDevice,
                    struct pipe_resource *pResource,
                    D3DRESOURCETYPE Type,
                    D3DPOOL Pool );

void
NineResource9_dtor( struct NineResource9 *This );

/*** Nine private methods ***/

struct pipe_resource *
NineResource9_GetResource( struct NineResource9 *This );

D3DPOOL
NineResource9_GetPool( struct NineResource9 *This );

/*** Direct3D public methods ***/

HRESULT WINAPI
NineResource9_GetDevice( struct NineResource9 *This,
                         IDirect3DDevice9 **ppDevice );

HRESULT WINAPI
NineResource9_SetPrivateData( struct NineResource9 *This,
                              REFGUID refguid,
                              const void *pData,
                              DWORD SizeOfData,
                              DWORD Flags );

HRESULT WINAPI
NineResource9_GetPrivateData( struct NineResource9 *This,
                              REFGUID refguid,
                              void *pData,
                              DWORD *pSizeOfData );

HRESULT WINAPI
NineResource9_FreePrivateData( struct NineResource9 *This,
                               REFGUID refguid );

DWORD WINAPI
NineResource9_SetPriority( struct NineResource9 *This,
                           DWORD PriorityNew );

DWORD WINAPI
NineResource9_GetPriority( struct NineResource9 *This );

void WINAPI
NineResource9_PreLoad( struct NineResource9 *This );

D3DRESOURCETYPE WINAPI
NineResource9_GetType( struct NineResource9 *This );

#endif /* _NINE_RESOURCE9_H_ */