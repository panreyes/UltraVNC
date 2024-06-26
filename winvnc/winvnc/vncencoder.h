/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2002-2024 UltraVNC Team Members. All Rights Reserved.
//  Copyright (C) 2015 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2000-2002 Const Kaplinsky. All Rights Reserved.
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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


// vncEncoder object

// The vncEncoder object encodes regions of a display buffer for sending
// to a client

class vncEncoder;

#if !defined(RFBENCODER_DEFINED)
#define RFBENCODER_DEFINED
#pragma once

#include "vncbuffer.h"
#include "translate.h"
#include "vsocket.h"
#include "vncmemcpy.h"

#define NUM_SUBSAMPOPT 6
enum subsamp_type
{
	SUBSAMP_1X = 0, SUBSAMP_4X, SUBSAMP_2X, SUBSAMP_GRAY, SUBSAMP_8X,
	SUBSAMP_16X
};

// Class definition

class vncEncoder
{
// Fields
public:

// Methods
public:
	// Create/Destroy methods
	vncEncoder();
	virtual ~vncEncoder();

	// Initialisation
	virtual void Init();

	// Encoder stats used by the buffer object
	virtual UINT RequiredBuffSize(UINT width, UINT height);
	virtual UINT NumCodedRects(const rfb::Rect &rect);
	virtual UINT NumCodedRects(RECT &rect); // For Tight


	// Translation & encoding routines
	//  - Source is the base address of the ENTIRE SCREEN buffer.
	//    The Translate routine compensates automatically for the desired rectangle.
	//  - Dest is the base address to encode the rect to. The rect will be encoded
	//    into a contiguous region of the buffer.
	virtual void Translate(BYTE *source, BYTE *dest, const rfb::Rect &rect);
	virtual UINT EncodeRect(BYTE *source, BYTE *dest, const rfb::Rect &rect);
	virtual UINT EncodeRect(BYTE *source, VSocket *outConn, BYTE *dest, const rfb::Rect &rect, bool queue_enabled);
	virtual UINT EncodeRect(BYTE *source,VSocket *outConn, BYTE *dest, const rfb::Rect &rect);
	virtual UINT EncodeRect(BYTE *source, VSocket *outConn, BYTE *dest, const RECT &rect); // sf@2002 - For Tight...
	virtual UINT EncodeBulkRects(const rfb::RectVector &rects, BYTE *source, BYTE *dest, VSocket *outConn) { return 0; };

	// CURSOR HANDLING
	void Translate(BYTE *source, BYTE *dest, int w, int h, int bytesPerRow);

	// Translation handling
	BOOL SetLocalFormat(rfbPixelFormat &pixformat, int width, int height);
	BOOL SetRemoteFormat(rfbPixelFormat &pixformat);

	// Colour map handling
	BOOL GetRemotePalette(RGBQUAD *quadlist, UINT ncolours);
	void SetBufferOffset(int x,int y);

	// Tight
	void SetCompressLevel(int level);
	void SetQualityLevel(int level);
	void SetFineQualityLevel(int level);
	void SetSubsampling(subsamp_type subsamp);
	void EnableLastRect(BOOL enable) { m_use_lastrect = enable; }

	// Tight - CURSOR HANDLING
	void EnableXCursor(BOOL enable) { m_use_xcursor = enable; }
	void EnableRichCursor(BOOL enable) { m_use_richcursor = enable; }
	BOOL SendEmptyCursorShape(VSocket *outConn);
	BOOL SendCursorShape(VSocket *outConn, vncDesktop *desktop);
	BOOL IsXCursorSupported();

	virtual void LastRect(VSocket *outConn); //xorzlib
	void set_use_zstd(bool use_zstd);

protected:
	BOOL SetTranslateFunction();

	// Tight - CURSOR HANDLING
	BOOL SendXCursorShape(VSocket *outConn, BYTE *mask, int xhot,int yhot,int width,int height);
	BOOL SendRichCursorShape(VSocket *outConn, BYTE *mbits, BYTE *cbits, int xhot,int yhot,int width,int height);
	void FixCursorMask(BYTE *mbits, BYTE *cbits, int width, int height, int width_bytes);

// Implementation
protected:
	rfbTranslateFnType	m_transfunc;			// Translator function
	char*				m_transtable;			// Colour translation LUT
	char*				m_localpalette;			// Palette info if client is palette-based
	rfbPixelFormat		m_localformat = {};			// Pixel Format info
	rfbPixelFormat		m_remoteformat = {};			// Client pixel format info
	rfbPixelFormat		m_transformat = {};			// Internal format used for translation (usually == client format)
	int					m_bytesPerRow;			// Number of bytes per row locally
	int					framebufferWidth;
	int					framebufferHeight;
	int					monitor_Offsetx;
	int					monitor_Offsety;
	int					dataSize;				// Total size of raw data encoded
	int					rectangleOverhead;		// Total size of rectangle header data
	int					encodedSize;			// Total size of encoded data
	int					transmittedSize;		// Total amount of data sent

	// Tight
	int					m_compresslevel;		// Encoding-specific compression level (if needed).
	int					m_qualitylevel;			// Image quality level for lossy JPEG compression.
	int					m_finequalitylevel;		// Fine-grained image quality level for lossy JPEG compression.
	int					m_subsampling;			// Chroma subsampling level for lossy JPEG compression.
	BOOL				m_use_lastrect;
	// Tight - CURSOR HANDLING
	BOOL				m_use_xcursor;			// XCursor cursor shape updates allowed.
	BOOL				m_use_richcursor;		// RichCursor cursor shape updates allowed.
	bool				m_use_zstd;
//	UltraVNCmemcpy mem;
};

#endif // vncENCODER_DEFINED
