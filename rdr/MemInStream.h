/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2002-2024 UltraVNC Team Members. All Rights Reserved.
//  Copyright (C) 2002 RealVNC Ltd. All Rights Reserved.
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


#ifndef __RDR_MEMINSTREAM_H__
#define __RDR_MEMINSTREAM_H__

#include "InStream.h"
#include "Exception.h"

namespace rdr {

  class MemInStream : public InStream {

  public:

    MemInStream(const void* data, int len) {
      ptr = start = (const U8*)data;
      end = start + len;
    }

    int pos() { return (int)(ptr - start); }
    void reposition(int pos) { ptr = start + pos; }

  private:

    int overrun(int itemSize, int nItems) { throw EndOfStream("overrun"); }
    const U8* start;
  };

}

#endif
