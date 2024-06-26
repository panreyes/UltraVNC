/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2002-2024 UltraVNC Team Members. All Rights Reserved.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
//  If the source code for the program is not available from the place from
//  which you received this file, check
//  https://uvnc.com/
//
////////////////////////////////////////////////////////////////////////////

#pragma once

struct mystruct
{
	short counter;
	short locked;
	RECT rect1[2000];
	RECT rect2[2000];
	short type[2000];
	short test;
};

// Class for Inter Process Communication using Memory Mapped Files
class CIPC
{
public:
	CIPC();
	virtual ~CIPC();

	/**unsigned char * CreateBitmap();
	unsigned char * CreateBitmap(int size);
	void CloseBitmap();*/
	bool CreateIPCMMF(void);
	bool OpenIPCMMF(void);
	void CloseIPCMMF(void);
	mystruct*  listall();

	bool CreateIPCMMFBitmap(int size);
	bool OpenIPCMMFBitmap(void);
	void CloseIPCMMFBitmap(void);

	bool IsOpen(void) const {return (m_hFileMap != NULL);}

protected:
	HANDLE m_hFileMap;
	LPVOID m_FileView;

	HANDLE m_hFileMapBitmap;
	LPVOID m_FileViewBitmap;

	mystruct list;
	mystruct *plist;
	unsigned char *pBitmap;
};
