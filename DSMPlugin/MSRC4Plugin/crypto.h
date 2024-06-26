/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2002-2024 UltraVNC Team Members. All Rights Reserved.
//  Copyright (C) 2005 Sean E. Covel All Rights Reserved.
//
//  Created by Sean E. Covel based on UltraVNC's excellent TestPlugin project.
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


#ifndef _CRYPTO_H
#define _CRYPTO_H

#pragma once
#include <tchar.h>

#define KEYLEN_128BIT 0x00800000	// 128-bit RC4 key
#define KEYLEN_56BIT  0x00380000	// 56-bit RC4 key
#define KEYLEN_40BIT  0x00280000	// 40-bit RC4 key
#define BLOBSIZE 76

#define KEYFILENAME_SIZE 256
#define CSP_SIZE 70
//#define CSP_PROV	PROV_DSS_DH

//#define CTX "MSRC4Plugin"	// Key container name

#define CTX "MSRC4Plugin"	// Key container name (Don't need one anymore. Keys are not persisted...)
							// Ah crap. Windows 95/98/98SE/ME/NT still need the container.

#define KEYSize 200

long GetCryptoVersion(void);	// Get crypto version
static const TCHAR * INDEXVAL_KEYGEN = _T("GenKeyFile");

#ifdef _WITH_REGISTRY  
	#define MSRC4_KEY_FILE        _T("Software\\ORL\\WinVNC3\\DSMPlugins\\MSRC4")
#endif  

		// Figure out CSP, Provider, OS Version, Crypto version, and Max keylength
BOOL InitVars(char *szCSPName, long *iWinVer, long *iCryptVer, DWORD * iMaxKey);

BOOL GenKey(char * sDefaultGenKey, DWORD keyLen);

BOOL CreateContainer(char * container);
BOOL DeleteContainer(char * container);


int ResetCrypto(HCRYPTKEY hkey);
int PrepContext(int iWinVer, HCRYPTKEY * hProvider);
void CleanupCryptoKey(HCRYPTKEY hKey);
void CleanupCryptoContext(HCRYPTPROV hProvider);
//int ImportCryptKey(HCRYPTPROV hProvider, const BYTE * pbBuffer, DWORD dwByteCount, HCRYPTKEY * hExchangeKey, HCRYPTKEY * hKey);
int ImportCryptKey(HCRYPTPROV hProvider, HCRYPTKEY * hKey, HANDLE hKeyFile);
int CreateDerivedCryptKey(HCRYPTPROV hProvider, HCRYPTKEY * hKey, char* password);



// An "Exponent of One" key is a key that doesn't actually do anything
// we use it to export the RC4 key plaintext. Otherwise you need to do a full
// public key exchange on the client/server machines to allow the rc4.key key to
// be imported. By default the import/export functions use the public\private key
// pairs to encrypt the RC4 key. Its a great security practice, but a pain
// for what I'm trying to accomplish here.
static BYTE PrivateKeyWithExponentOfOne[] =
{
   0x07, 0x02, 0x00, 0x00, 0x00, 0xA4, 0x00, 0x00,
   0x52, 0x53, 0x41, 0x32, 0x00, 0x02, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00, 0xAB, 0xEF, 0xFA, 0xC6,
   0x7D, 0xE8, 0xDE, 0xFB, 0x68, 0x38, 0x09, 0x92,
   0xD9, 0x42, 0x7E, 0x6B, 0x89, 0x9E, 0x21, 0xD7,
   0x52, 0x1C, 0x99, 0x3C, 0x17, 0x48, 0x4E, 0x3A,
   0x44, 0x02, 0xF2, 0xFA, 0x74, 0x57, 0xDA, 0xE4,
   0xD3, 0xC0, 0x35, 0x67, 0xFA, 0x6E, 0xDF, 0x78,
   0x4C, 0x75, 0x35, 0x1C, 0xA0, 0x74, 0x49, 0xE3,
   0x20, 0x13, 0x71, 0x35, 0x65, 0xDF, 0x12, 0x20,
   0xF5, 0xF5, 0xF5, 0xC1, 0xED, 0x5C, 0x91, 0x36,
   0x75, 0xB0, 0xA9, 0x9C, 0x04, 0xDB, 0x0C, 0x8C,
   0xBF, 0x99, 0x75, 0x13, 0x7E, 0x87, 0x80, 0x4B,
   0x71, 0x94, 0xB8, 0x00, 0xA0, 0x7D, 0xB7, 0x53,
   0xDD, 0x20, 0x63, 0xEE, 0xF7, 0x83, 0x41, 0xFE,
   0x16, 0xA7, 0x6E, 0xDF, 0x21, 0x7D, 0x76, 0xC0,
   0x85, 0xD5, 0x65, 0x7F, 0x00, 0x23, 0x57, 0x45,
   0x52, 0x02, 0x9D, 0xEA, 0x69, 0xAC, 0x1F, 0xFD,
   0x3F, 0x8C, 0x4A, 0xD0,

   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

   0x64, 0xD5, 0xAA, 0xB1,
   0xA6, 0x03, 0x18, 0x92, 0x03, 0xAA, 0x31, 0x2E,
   0x48, 0x4B, 0x65, 0x20, 0x99, 0xCD, 0xC6, 0x0C,
   0x15, 0x0C, 0xBF, 0x3E, 0xFF, 0x78, 0x95, 0x67,
   0xB1, 0x74, 0x5B, 0x60,

   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};



#endif