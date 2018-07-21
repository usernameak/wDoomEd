// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Handles WAD file header, directory, lump I/O.
//
//-----------------------------------------------------------------------------

static const char rcsid[] = "$Id: w_wad.c,v 1.5 1997/02/03 16:47:57 b1 Exp $";

#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <alloca.h>
#ifndef O_BINARY
#define O_BINARY		0
#endif
#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif
#include "w_wad.h"
#include <wx/wx.h>

#define I_Error(...) wxFAIL_MSG(wxString::Format(__VA_ARGS__))

//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t* lumpinfo;
int numlumps;

void** lumpcache;

#define strcmpi	strcasecmp

void strupr(char* s) {
	while (*s) {
		*s = toupper(*s);
		s++;
	}
}

int filelength(int handle) {
	struct stat fileinfo;

	if (fstat(handle, &fileinfo) == -1)
		wxFAIL_MSG("Error fstating");

	return fileinfo.st_size;
}

void ExtractFileBase(char* path, char* dest) {
	char* src;
	int length;

	src = path + strlen(path) - 1;

	// back up until a \ or the start
	while (src != path && *(src - 1) != '\\' && *(src - 1) != '/') {
		src--;
	}

	// copy up to eight characters
	memset(dest, 0, 8);
	length = 0;

	while (*src && *src != '.') {
		if (++length == 9)
			wxFAIL_MSG(wxString::Format("Filename base of %s >8 chars", path));

		*dest++ = toupper((int) *src++);
	}
}

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// If filename starts with a tilde, the file is handled
//  specially to allow map reloads.
// But: the reload feature is a fragile hack...

int reloadlump;
char* reloadname;

void W_AddFile(char *filename) {
	wadinfo_t header;
	lumpinfo_t* lump_p;
	unsigned i;
	int handle;
	int length;
	int startlump;
	filelump_t* fileinfo;
	filelump_t singleinfo;
	int storehandle;

	// open the file and add to directory

	// handle reload indicator.
	if (filename[0] == '~') {
		filename++;
		reloadname = filename;
		reloadlump = numlumps;
	}

	if ((handle = open(filename, O_RDONLY | O_BINARY)) == -1) {
		wxPrintf(" couldn't open %s\n", filename);
		return;
	}

	wxPrintf(" adding %s\n", filename);
	startlump = numlumps;

	// WAD file
	read(handle, &header, sizeof(header));
	if (strncmp(header.identification, "IWAD", 4)) {
		// Homebrew levels?
		if (strncmp(header.identification, "PWAD", 4)) {
			I_Error("Wad file %s doesn't have IWAD "
					"or PWAD id\n", filename);
		}

		// ???modifiedgame = true;
	}
	header.numlumps = (int) (header.numlumps);
	header.infotableofs = (int) (header.infotableofs);
	length = header.numlumps * sizeof(filelump_t);
	fileinfo = (filelump_t *) alloca(length);
	lseek(handle, header.infotableofs, SEEK_SET);
	read(handle, fileinfo, length);
	numlumps += header.numlumps;

	// Fill in lumpinfo
	lumpinfo = (lumpinfo_t *) realloc(lumpinfo, numlumps * sizeof(lumpinfo_t));

	if (!lumpinfo)
		I_Error("Couldn't realloc lumpinfo");

	lump_p = &lumpinfo[startlump];

	storehandle = reloadname ? -1 : handle;

	for (i = startlump; i < numlumps; i++, lump_p++, fileinfo++) {
		lump_p->handle = storehandle;
		lump_p->position = (int) (fileinfo->filepos);
		lump_p->size = (int) (fileinfo->size);
		strncpy(lump_p->name, fileinfo->name, 8);
	}

	if (reloadname)
		close(handle);
}

//
// W_Reload
// Flushes any of the reloadable lumps in memory
//  and reloads the directory.
//
void W_Reload(void) {
	wadinfo_t header;
	int lumpcount;
	lumpinfo_t* lump_p;
	unsigned i;
	int handle;
	int length;
	filelump_t* fileinfo;

	if (!reloadname)
		return;

	if ((handle = open(reloadname, O_RDONLY | O_BINARY)) == -1)
		I_Error("W_Reload: couldn't open %s", reloadname);

	read(handle, &header, sizeof(header));
	lumpcount = (int) (header.numlumps);
	header.infotableofs = (int) (header.infotableofs);
	length = lumpcount * sizeof(filelump_t);
	fileinfo = (filelump_t *) alloca(length);
	lseek(handle, header.infotableofs, SEEK_SET);
	read(handle, fileinfo, length);

	// Fill in lumpinfo
	lump_p = &lumpinfo[reloadlump];

	for (i = reloadlump; i < reloadlump + lumpcount;
			i++, lump_p++, fileinfo++) {
		if (lumpcache[i])
			free(lumpcache[i]);

		lump_p->position = (int) (fileinfo->filepos);
		lump_p->size = (int) (fileinfo->size);
	}

	close(handle);
}

//
// W_InitMultipleFiles
// Pass a null terminated list of files to use.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
void W_InitMultipleFiles(char** filenames) {
	int size;

	// open all the files, load headers, and count lumps
	numlumps = 0;

	// will be realloced as lumps are added
	lumpinfo = (lumpinfo_t *) malloc(1);

	for (; *filenames; filenames++)
		W_AddFile(*filenames);

	if (!numlumps)
		I_Error("W_InitFiles: no files found");

	// set up caching
	size = numlumps * sizeof(*lumpcache);
	lumpcache = (void **) malloc(size);

	if (!lumpcache)
		I_Error("Couldn't allocate lumpcache");

	memset(lumpcache, 0, size);
}

//
// W_InitFile
// Just initialize from a single file.
//
void W_InitFile(char* filename) {
	char* names[2];

	names[0] = filename;
	names[1] = NULL;
	W_InitMultipleFiles(names);
}

//
// W_NumLumps
//
int W_NumLumps(void) {
	return numlumps;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//

int W_CheckNumForName(const char* name) {
	union {
		char s[9];
		int x[2];

	} name8;

	int v1;
	int v2;
	lumpinfo_t* lump_p;

	// make the name into two integers for easy compares
	strncpy(name8.s, name, 8);

	// in case the name was a fill 8 chars
	name8.s[8] = 0;

	// case insensitive
	strupr(name8.s);

	v1 = name8.x[0];
	v2 = name8.x[1];

	// scan backwards so patch lump files take precedence
	lump_p = lumpinfo + numlumps;

	while (lump_p-- != lumpinfo) {
		if (*(int *) lump_p->name == v1 && *(int *) &lump_p->name[4] == v2) {
			return lump_p - lumpinfo;
		}
	}

	// TFB. Not found.
	return -1;
}

//
// W_CheckNextNumForName
//
//

int W_CheckNextNumForName(int prev, const char *name) {
	union {
		char s[9];
		int x[2];

	} name8;

	int v1;
	int v2;
	lumpinfo_t* lump_p;

	// make the name into two integers for easy compares
	strncpy(name8.s, name, 8);

	// in case the name was a fill 8 chars
	name8.s[8] = 0;

	// case insensitive
	strupr(name8.s);

	v1 = name8.x[0];
	v2 = name8.x[1];

	lump_p = lumpinfo + prev;

	while (++lump_p != lumpinfo + numlumps) {
		if (*(int *) lump_p->name == v1 && *(int *) &lump_p->name[4] == v2) {
			return lump_p - lumpinfo;
		}
	}

	// TFB. Not found.
	return -1;
}

//
// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName(const char* name) {
	int i;

	i = W_CheckNumForName(name);

	if (i == -1)
		I_Error("W_GetNumForName: %s not found!", name);

	return i;
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength(int lump) {
	if (lump >= numlumps)
		I_Error("W_LumpLength: %i >= numlumps", lump);

	return lumpinfo[lump].size;
}

//
//
//
//

void W_ReadLumpN(int lump, void *dest, int size) {
	int c;
	lumpinfo_t* l;
	int handle;

	if (lump >= numlumps)
		I_Error("W_ReadLump: %i >= numlumps", lump);

	l = lumpinfo + lump;

	// ??? I_BeginRead ();

	if (size > l->size) {
		I_Error("W_ReadLump: (size = %i) > (l->size = %i)", size, l->size);
	}

	if (l->handle == -1) {
		// reloadable file, so use open / read / close
		if ((handle = open(reloadname, O_RDONLY | O_BINARY)) == -1)
			I_Error("W_ReadLump: couldn't open %s", reloadname);
	} else
		handle = l->handle;

	lseek(handle, l->position, SEEK_SET);
	c = read(handle, dest, size);

	if (c < size)
		if(c >= 0) {
			I_Error("W_ReadLump: only read %i of %i on lump %i", c, size, lump);
		} else {
			I_Error("W_ReadLump: error \"%s\" on lump %i", strerror(errno), lump);
		}

	if (l->handle == -1)
		close(handle);

	// ??? I_EndRead ();
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//
void W_ReadLump(int lump, void* dest) {
	/*int c;
	lumpinfo_t* l;
	int handle;

	if (lump >= numlumps)
		I_Error("W_ReadLump: %i >= numlumps", lump);

	l = lumpinfo + lump;

	// ??? I_BeginRead ();

	if (l->handle == -1) {
		// reloadable file, so use open / read / close
		if ((handle = open(reloadname, O_RDONLY | O_BINARY)) == -1)
			I_Error("W_ReadLump: couldn't open %s", reloadname);
	} else
		handle = l->handle;

	lseek(handle, l->position, SEEK_SET);
	c = read(handle, dest, l->size);

	if (c < l->size)
		I_Error("W_ReadLump: only read %i of %i on lump %i", c, l->size, lump);

	if (l->handle == -1)
		close(handle);

	// ??? I_EndRead ();*/
	W_ReadLumpN(lump, dest, W_LumpLength(lump));
}

//
// W_CacheLumpNum
//
void* W_CacheLumpNum(int lump) {
	char* ptr;

	if ((unsigned) lump >= numlumps)
		I_Error("W_CacheLumpNum: %i >= numlumps", lump);

	// TODO: safe dealloc

	if (!lumpcache[lump]) {
		lumpcache[lump] = (char *) malloc(W_LumpLength(lump));
		W_ReadLump(lump, lumpcache[lump]);
	}

	return lumpcache[lump];
}

//
// W_CleanCache
//
void W_CleanCache() {
	for(int i = 0; i < numlumps; i++) {
		if(lumpcache[i]) {
			free(lumpcache[i]);
		}
	}
}

//
// W_CacheLumpName
//
void* W_CacheLumpName(const char* name) {
	return W_CacheLumpNum(W_GetNumForName(name));
}
