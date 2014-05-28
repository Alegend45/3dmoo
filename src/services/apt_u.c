/*
 * Copyright (C) 2014 - plutoo
 * Copyright (C) 2014 - ichfly
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"
#include "handles.h"
#include "mem.h"
#include "arm11.h"

u32 lock_handle;
u32 event_handles[2];


#define CPUsvcbuffer 0xFFFF0000

u32 apt_u_SyncRequest()
{
    u32 cid = mem_Read32(0xFFFF0080);

    // Read command-id.
    switch(cid) {

    case 0x10040:
        (void) 0;

        u32 flags = mem_Read32(0xFFFF0084);
        DEBUG("apt_u_GetLockHandle, flags=%08x\n", flags);
        PAUSE();

        mem_Write32(0xFFFF0084, 0); // result

        // XXX: fix handle
        lock_handle = handle_New(HANDLE_TYPE_MUTEX, HANDLE_MUTEX_APTMUTEX);

        handleinfo* h = handle_Get(lock_handle);
        h->locked = false;
        h->locktype = LOCK_TYPE_ONESHOT;

        mem_Write32(0xFFFF0094, lock_handle); // lock_handle
        return 0;

    case 0x20080:
        (void) 0;

        u32 app_id = mem_Read32(0xFFFF0084);
        DEBUG("apt_u_RegisterApp, app_id=%08x\n", app_id);
        PAUSE();

        // XXX: fix handles
        event_handles[0] = handle_New(HANDLE_TYPE_EVENT, HANDLE_SUBEVENT_APTMENUEVENT);
        h = handle_Get(event_handles[0]);
        h->locked = true;
        h->locktype = LOCK_TYPE_ONESHOT;
        event_handles[1] = handle_New(HANDLE_TYPE_EVENT, HANDLE_SUBEVENT_APTPAUSEEVENT);
        h = handle_Get(event_handles[1]);
        h->locked = false; //fire start event
        h->locktype = LOCK_TYPE_ONESHOT;

        mem_Write32(0xFFFF008C, event_handles[0]); // some event handles
        mem_Write32(0xFFFF0090, event_handles[1]); // some event handles

        mem_Write32(0xFFFF0084, 0); // result
        return 0;

    case 0x30040: {
        (void) 0;

        u32 unk = mem_Read32(0xFFFF0084);
        DEBUG("apt_u_Enable, unk=%08x\n", unk);
        PAUSE();

        mem_Write32(0xFFFF0084, 0);
        return 0;
    }
    case 0x3E0080: {
        u32 unk = mem_Read32(0xFFFF0084);
        DEBUG("apt_u_ReplySleepQuery, unk=%08x\n", unk);
        PAUSE();

        mem_Write32(0xFFFF0084, 0);
        return 0;
    }
    case 0x430040:
        (void) 0;

        app_id = mem_Read32(0xFFFF0084);
        DEBUG("apt_u_NotifyToWait, app_id=%08x\n", app_id);
        PAUSE();

        mem_Write32(0xFFFF0084, 0);
        return 0;

    case 0x4b00c2: { //AppletUtility
        u32 unk = mem_Read32(CPUsvcbuffer + 0x84);
        u32 pointeresize = mem_Read32(CPUsvcbuffer + 0x88);
        u32 pointerzsize = mem_Read32(CPUsvcbuffer + 0x8C);
        u32 pointere = mem_Read32(CPUsvcbuffer + 0x94);
        u32 pointerz = mem_Read32(CPUsvcbuffer + 0x184);
        u8* data = (u8*)malloc(pointeresize + 1);
        DEBUG("AppletUtility %08X (%08X %08X,%08X %08X)\n", unk, pointere, pointeresize, pointerz, pointerzsize);
        mem_Read(data, pointere, pointeresize);
        for (unsigned int i = 0; i < pointeresize; i++) {
            if (i % 16 == 0) printf("\n");
            printf("%02X ", data[i]);
        }
        free(data);
        mem_Write32(CPUsvcbuffer + 0x84, 0); //worked
        //arm11_Dump();
        return 0;
    }

    case 0xb0040: { //InquireNotification
        u32 appID = mem_Read32(0xFFFF0088);
        DEBUG("apt_u_InquireNotification, appID=%08x\n", appID);
        PAUSE();

        mem_Write32(0xFFFF008C, 0); //signal type

        mem_Write32(0xFFFF0084, 0);
        return 0;
    }

    case 0xe0080: //GlanceParameter
    {
        u32 appID = mem_Read32(0xFFFF0084);
        u32 bufSize = mem_Read32(0xFFFF0088);
        DEBUG("apt_u_GlanceParameter, appID=%08x, bufSize=%08x\n", appID, bufSize);
        PAUSE();

        mem_Write32(0xFFFF0084, 0); // result
        mem_Write32(0xFFFF0088, 0); // appid of triggering process

        // signal type (1=app just started, 0xb=returning to app, 0xc=exiting app)
        mem_Write32(0xFFFF008C, 1);

        mem_Write32(0xFFFF0090, 0x10);
        mem_Write32(0xFFFF0094, 0); // some handle
        mem_Write32(0xFFFF0098, 0); // (bufsize<<14) | 2
        mem_Write32(0xFFFF009C, 0); // bufptr
        
        return 0;
    }

    }

    ERROR("NOT IMPLEMENTED, cid=%08x\n", cid);
    arm11_Dump();
    PAUSE();
    return 0;
}
