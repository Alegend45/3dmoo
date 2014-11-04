/*
* Copyright (C) 2014 - plutoo
* Copyright (C) 2014 - ichfly
* Copyright (C) 2014 - Normmatt
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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"
#include "mem.h"
#include "handles.h"
#include "fs.h"


/* ____ Save Data implementation ____ */

static u32 savedatafile_Read(file_type* self, u32 ptr, u32 sz, u64 off, u32* read_out)
{
    FILE* fd = self->type_specific.sysdata.fd;
    *read_out = 0;

    if(off >> 32) {
        ERROR("64-bit offset not supported.\n");
        return -1;
    }

    if(fseek(fd, off, SEEK_SET) == -1) {
        ERROR("fseek failed.\n");
        return -1;
    }

    u8* b = malloc(sz);
    if(b == NULL) {
        ERROR("Not enough mem.\n");
        return -1;
    }

    u32 read = fread(b, 1, sz, fd);
    if(read == 0) {
        ERROR("fread failed\n");
        free(b);
        return -1;
    }

    if(mem_Write(b, ptr, read) != 0) {
        ERROR("mem_Write failed.\n");
        free(b);
        return -1;
    }

    *read_out = read;
    free(b);

    return 0; // Result
}

static u32 savedatafile_Write(file_type* self, u32 ptr, u32 sz, u64 off, u32 flush_flags, u32* written_out)
{
    FILE* fd = self->type_specific.sysdata.fd;
    *written_out = 0;

    if (off >> 32) {
        ERROR("64-bit offset not supported.\n");
        return -1;
    }

    if (fseek(fd, off, SEEK_SET) == -1) {
        ERROR("fseek failed.\n");
        return -1;
    }

    u8* b = malloc(sz);
    if (b == NULL) {
        ERROR("Not enough mem.\n");
        return -1;
    }

    if (mem_Read(b, ptr, sz) != 0) {
        ERROR("mem_Read failed.\n");
        free(b);
        return -1;
    }

    u32 written = fwrite(b, 1, sz, fd);
    if (written == 0) {
        ERROR("fwrite failed\n");
        free(b);
        return -1;
    }

    *written_out = written;
    free(b);

    return 0; // Result
}

static u64 savedatafile_GetSize(file_type* self)
{
    return self->type_specific.sysdata.sz;
}

static u64 savedatafile_SetSize(file_type* self, u64 sz)
{
    FILE* fd = self->type_specific.sysdata.fd;
    u64 current_size = self->type_specific.sysdata.sz;

    if (sz >= current_size)
    {
        if (fseek(fd, sz, SEEK_SET) == -1) {
            ERROR("fseek failed.\n");
            return -1;
        }
    }
    else
    {
        DEBUG("Truncating a file is unsupported.\n");
    }

    return 0;
}

static u32 savedatafile_Close(file_type* self)
{
    // Close file and free yourself
    fclose(self->type_specific.sysdata.fd);
    free(self);

    return 0;
}



/* ____ FS implementation ____ */

static bool savedata_FileExists(archive* self, file_path path)
{
    char p[256], tmp[256];
    struct stat st;

    // Generate path on host file system
    snprintf(p, 256, "savedata/%s",
             fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return false;
    }

    return stat(p, &st) == 0;
}

static u32 savedata_OpenFile(archive* self, file_path path, u32 flags, u32 attr)
{
    char p[256], tmp[256];

    // Generate path on host file system
    snprintf(p, 256, "savedata/%s",
             fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if(!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return 0;
    }

    char mode[10];
    memset(mode, 0, 10);

    switch (flags)
    {
        case 1: //R
            strcpy(mode, "rb");
            break;
        case 2: //W
        case 3: //RW
            strcpy(mode, "rb+");
            break;
        case 4: //C
        case 6: //W+C
            strcpy(mode, "wb");
            break;
    }

    FILE* fd = fopen(p, mode);

    if(fd == NULL) {
        ERROR("Failed to open SaveData, path=%s\n", p);
        return 0;
    }

    fseek(fd, 0, SEEK_END);
    u32 sz;

    if((sz=ftell(fd)) == -1) {
        ERROR("ftell() failed.\n");
        fclose(fd);
        return 0;
    }

    // Create file object
    file_type* file = calloc(sizeof(file_type), 1);

    if(file == NULL) {
        ERROR("calloc() failed.\n");
        fclose(fd);
        return 0;
    }

    file->type_specific.sysdata.fd = fd;
    file->type_specific.sysdata.sz = (u64) sz;

    // Setup function pointers.
    file->fnRead = &savedatafile_Read;
    file->fnWrite = &savedatafile_Write;
    file->fnGetSize = &savedatafile_GetSize;
    file->fnSetSize = &savedatafile_SetSize;
    file->fnClose = &savedatafile_Close;

    return handle_New(HANDLE_TYPE_FILE, (uintptr_t) file);
}


u32 savedata_fnReadDir(dir_type* self, u32 ptr, u32 entrycount, u32* read_out)
{
    int current = 0;
    while (current < entrycount) //the first entry is ignored
    {
        if (FindNextFileA(self->hFind, self->ffd) == 0) //no more files
        {
            break;
        }
        for (int i = 0; i < 0x106; i++) //todo non ASCII code
        {
            if (self->ffd->cFileName[i] == NULL) //this is the end
            {
                mem_Write8(ptr + i * 2, 0);
                mem_Write8(ptr + i * 2 + 1, 0);
                break;
            }
            mem_Write8(ptr + i * 2, self->ffd->cFileName[i]);
            mem_Write8(ptr + i * 2 + 1, 0);
        }

        mem_Write8(ptr + 0x216, 0x0);
        mem_Write8(ptr + 0x217, 0x0);
        mem_Write8(ptr + 0x218, 0x0);
        char * pch;
        char * pcho;
        if (self->ffd->cAlternateFileName[0] == 0)
        {
            pch = pcho = self->ffd->cFileName;
        }
        else
        {
            pch = pcho = self->ffd->cAlternateFileName;
        }
        char * pchtemp;
        bool found = false;
        while (true)
        {
            pchtemp = strchr(pch, '.');
            if (!pchtemp)break;
            pch = pchtemp + 1;
            found = true;
        }
        if (found)
        {
            for (int i = 0; i < 3; i++)
            {
                mem_Write8(ptr + 0x216 + i, pch[i]);
                if (pch[i] == 0)break;
            }
        }
        for (int i = 0; i < 8; i++)
        {
            mem_Write8(ptr + 0x20C + i, pcho[i]);
            if (pcho[i] == 0 || &pcho[i] == pch - 1)break;
        }

        //mem_Write(self->ffd->cFileName, ptr, 0x20C); //todo

        mem_Write8(ptr + 0x215, 0xA); //unknown

        //mem_Write(self->ffd->cFileName, ptr, 0x20C); //todo
        mem_Write8(ptr + 0x21A, 0x1); //unknown
        mem_Write8(ptr + 0x21B, 0x0); //unknown
        if (self->ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            mem_Write8(ptr + 0x21C, 0x1);

            mem_Write8(ptr + 0x216, ' '); //8.3 file extension
            mem_Write8(ptr + 0x217, ' ');
            mem_Write8(ptr + 0x218, ' ');
        }
        else
        {
            mem_Write8(ptr + 0x21C, 0x0);
        }
        if (self->ffd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        {
            mem_Write8(ptr + 0x21D, 0x1);
        }
        else
        {
            mem_Write8(ptr + 0x21D, 0x0);
        }
        if (self->ffd->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
        {
            mem_Write8(ptr + 0x21E, 0x1);
        }
        else
        {
            mem_Write8(ptr + 0x21E, 0x0);
        }
        if (self->ffd->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
            mem_Write8(ptr + 0x21F, 0x1);
        }
        else
        {
            mem_Write8(ptr + 0x21F, 0x0);
        }
        if (!(self->ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            mem_Write32(ptr + 0x220, self->ffd->nFileSizeLow);
            mem_Write32(ptr + 0x224, self->ffd->nFileSizeHigh);
        }
        current++;
    }
    *read_out = current;
    return 0;
}


int savedata_createdir(archive* self, file_path path)
{
    char p[256], tmp[256];

    // Generate path on host file system
    snprintf(p, 256, "savedata/%s",
        fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    if (!fs_IsSafePath(p)) {
        ERROR("Got unsafe path.\n");
        return 0;
    }
    return _mkdir(p);
}

static u32 savedata_fnOpenDir(archive* self, file_path path)
{
    // Create file object
    dir_type* dir = (dir_type*)malloc(sizeof(dir_type));

    dir->f_path = path;
    dir->self = self;


    dir->hFind = INVALID_HANDLE_VALUE;

    // Setup function pointers.
    dir->fnRead = &savedata_fnReadDir;

    char p[256], tmp[256];
    snprintf(p, 256, "savedata/%s/*",
        fs_PathToString(path.type, path.ptr, path.size, tmp, 256));

    dir->ffd = (WIN32_FIND_DATAA *)malloc(sizeof(WIN32_FIND_DATAA));

    dir->hFind = FindFirstFileA(p, dir->ffd);

    if (dir->hFind == INVALID_HANDLE_VALUE)
        return 0;

    printf("First file name is %s.\n", dir->ffd->cFileName);

    return handle_New(HANDLE_TYPE_DIR, (uintptr_t)dir);
}


static void savedata_Deinitialize(archive* self)
{
    // Free yourself
    free(self);
}

archive* savedata_OpenArchive(file_path path)
{
    archive* arch = calloc(sizeof(archive), 1);

    if (arch == NULL) {
        ERROR("malloc failed.\n");
        return NULL;
    }

    // Setup function pointers

    arch->fncreateDir = &savedata_createdir;
    arch->fnOpenDir = &savedata_fnOpenDir;
    arch->fnFileExists = &savedata_FileExists;
    arch->fnOpenFile = &savedata_OpenFile;
    arch->fnDeinitialize = &savedata_Deinitialize;

    snprintf(arch->type_specific.sysdata.path,
        sizeof(arch->type_specific.sysdata.path),
        "");
    return arch;
}