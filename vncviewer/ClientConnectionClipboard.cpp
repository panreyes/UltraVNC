/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2002-2024 UltraVNC Team Members. All Rights Reserved.
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


#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "Exception.h"
extern char sz_C1[64];
extern char sz_C2[64];
extern char sz_C3[64];

// This file contains the code for getting text from, and putting text into
// the Windows clipboard.

//
// ProcessClipboardChange
// Called by ClientConnection::WndProc.
// We've been informed that the local clipboard has been updated.
// If it's text we want to send it to the server.
//

void ClientConnection::ProcessLocalClipboardChange()
{
	vnclog.Print(2, _T("Clipboard changed\n"));
	
	HWND hOwner = GetClipboardOwner();

	//adzm 2010-05-11 - Ignore clipboard while initializing (copying a password, for example, will end up sending a packet and causing a failure)
	if (!m_running)
	{
		vnclog.Print(2, _T("Ignore Clipboard while initializing!\n"));
		//m_initialClipboardSeen = true;
	}
	else if (m_settingClipboardViewer)
	{
		vnclog.Print(2, _T("Ignore Clipboard while setting viewer!\n"));
		//m_initialClipboardSeen = true;
	}
	else if (m_pFileTransfer && (m_pFileTransfer->m_fFileTransferRunning ||m_pFileTransfer->m_fFileUploadRunning || m_pFileTransfer->m_fFileDownloadRunning))
	{
		vnclog.Print(2, _T("Ignore Clipboard while File Transfer is buzy!\n"));
		//m_initialClipboardSeen = true;
	}
	else if (hOwner == m_hwndcn) {
		vnclog.Print(2, _T("We changed it - ignore!\n"));
	/*} else if (!m_initialClipboardSeen) {
		vnclog.Print(2, _T("Don't send initial clipboard!\n"));
		m_initialClipboardSeen = true;*/
	} else if (!m_opts->m_DisableClipboard && !m_opts->m_ViewOnly) {
		UpdateRemoteClipboard();
	}
	// Pass the message to the next window in clipboard viewer chain
	if (m_hwndNextViewer != NULL && m_hwndNextViewer != (HWND)INVALID_HANDLE_VALUE) {
		vnclog.Print(6, _T("Passing WM_DRAWCLIPBOARD to 0x%08x\n"), m_hwndNextViewer);
		// use SendNotifyMessage instead of SendMessage so misbehaving or hung applications
		// (like ourself before this) won't cause our thread to hang.
		::SendNotifyMessage(m_hwndNextViewer, WM_DRAWCLIPBOARD , 0,0); 
	} else {
		vnclog.Print(6, _T("No next window in chain; WM_DRAWCLIPBOARD will not be passed\n"), m_hwndNextViewer);
	}
}


// adzm - 2010-07 - Extended clipboard
void ClientConnection::UpdateRemoteClipboard(CARD32 overrideFlags)
{			
	// The clipboard should not be modified by more than one thread at once
	omni_mutex_lock l(m_clipMutex);
	
	if (m_clipboard.settings.m_bSupportsEx) {
		ClipboardData newClipboard;

		if (newClipboard.Load(NULL)) {
			if (newClipboard.m_crc == m_clipboard.m_crc && overrideFlags == 0) {
				vnclog.Print(6, _T("Ignoring extended SendClientCutText due to identical data\n"));
				return;
			}

			m_clipboard.UpdateClipTextEx(newClipboard, overrideFlags);
			
			if (m_clipboard.m_bNeedToProvide) {
				m_clipboard.m_bNeedToProvide = false;
				int actualLen = m_clipboard.extendedClipboardDataMessage.GetDataLength();

				rfbClientCutTextMsg message;
				memset(&message, 0, sizeof(rfbClientCutTextMsg));
				message.type = rfbClientCutText;

				message.length = Swap32IfLE(-actualLen);
				
				//adzm 2010-09
				WriteExactQueue((char*)&message, sz_rfbClientCutTextMsg, rfbClientCutText);
				WriteExact((char*)(m_clipboard.extendedClipboardDataMessage.GetData()), m_clipboard.extendedClipboardDataMessage.GetDataLength());
			
				vnclog.Print(6, _T("Sent extended clipboard\n"));
			}

			m_clipboard.extendedClipboardDataMessage.Reset();

			if (m_clipboard.m_bNeedToNotify) {
				m_clipboard.m_bNeedToNotify = false;
				if (m_clipboard.settings.m_bSupportsEx) {

					int actualLen =m_clipboard.extendedClipboardDataNotifyMessage.GetDataLength();

					rfbClientCutTextMsg message;
					memset(&message, 0, sizeof(rfbClientCutTextMsg));
					message.type = rfbClientCutText;

					message.length = Swap32IfLE(-actualLen);

					WriteExactQueue((char*)&message, sz_rfbClientCutTextMsg, rfbClientCutText);
					WriteExact((char*)(m_clipboard.extendedClipboardDataNotifyMessage.GetData()), m_clipboard.extendedClipboardDataNotifyMessage.GetDataLength());

				}
				m_clipboard.extendedClipboardDataNotifyMessage.Reset();
			}

		} else {
			vnclog.Print(6, _T("Failed to load local clipboard!\n"));
		}
	} else {		
		vnclog.Print(6, _T("Checking clipboard...\n"));
		ClipboardHolder holder(m_hwndcn);
		if (holder.m_bIsOpen) {
			vnclog.Print(6, _T("Opened...\n"));
			HGLOBAL hglb = GetClipboardData(CF_TEXT); 
			if (hglb == NULL) {				
				vnclog.Print(6, _T("No CF_TEXT!\n"));
			} else {
				vnclog.Print(6, _T("Got CF_TEXT!\n"));
				LPSTR lpstr = (LPSTR) GlobalLock(hglb);  
				
				char *contents = new char[strlen(lpstr) + 1];
				char *unixcontents = new char[strlen(lpstr) + 1];
				strcpy_s(contents, strlen(lpstr) + 1, lpstr);
				GlobalUnlock(hglb);  		
				
				// Translate to Unix-format lines before sending
				int j = 0;
				for (int i = 0; contents[i] != '\0'; i++) {
					if (contents[i] != '\x0d') {
						unixcontents[j++] = contents[i];
					}
				}
				unixcontents[j] = '\0';
				try {
					SendClientCutText(unixcontents, strlen(unixcontents));
				} catch (WarningException &e) {
					vnclog.Print(0, _T("Exception while sending clipboard text : %s\n"), e.m_info);
					DestroyWindow(m_hwndcn);
				}
				delete [] contents; 
				delete [] unixcontents;
			}
		} 
	}
}

// adzm - 2010-07 - Extended clipboard
void ClientConnection::UpdateRemoteClipboardCaps(bool bSavePreferences)
{
	omni_mutex_lock l(m_clipMutex);
	if (!m_clipboard.settings.m_bSupportsEx) return;

	ExtendedClipboardDataMessage extendedClipboardDataMessage;
	
	if (m_opts->m_DisableClipboard || m_opts->m_ViewOnly) {
		// messages and formats that we can handle
		extendedClipboardDataMessage.m_pExtendedData->flags = Swap32IfLE(clipCaps | clipText | clipRTF | clipHTML | clipDIB);

		// now include our limits in order of enum value
		extendedClipboardDataMessage.AppendInt(0);
		extendedClipboardDataMessage.AppendInt(0);
		extendedClipboardDataMessage.AppendInt(0);
		extendedClipboardDataMessage.AppendInt(0);
	} else {
		if (bSavePreferences) {
			SaveClipboardPreferences();
		}
		m_clipboard.settings.PrepareCapsPacket(extendedClipboardDataMessage);
	}

	int actualLen = extendedClipboardDataMessage.GetDataLength();

	rfbClientCutTextMsg message;
	memset(&message, 0, sizeof(rfbClientCutTextMsg));
	message.type = rfbClientCutText;

	message.length = Swap32IfLE(-actualLen);
	
	//adzm 2010-09
	WriteExactQueue((char*)&message, sz_rfbClientCutTextMsg, rfbClientCutText);
	WriteExact((char*)(extendedClipboardDataMessage.GetData()), extendedClipboardDataMessage.GetDataLength());
}

void ClientConnection::RequestRemoteClipboard()
{
	if (!m_clipboard.settings.m_bSupportsEx) return;

	ExtendedClipboardDataMessage extendedClipboardDataMessage;

	int actualLen = extendedClipboardDataMessage.GetDataLength();
	extendedClipboardDataMessage.AddFlag(clipRequest | clipText | clipRTF | clipHTML | clipDIB);

	rfbClientCutTextMsg message;
	memset(&message, 0, sizeof(rfbClientCutTextMsg));
	message.type = rfbClientCutText;

	message.length = Swap32IfLE(-actualLen);
	
	//adzm 2010-09
	WriteExactQueue((char*)&message, sz_rfbClientCutTextMsg, rfbClientCutText);
	WriteExact((char*)(extendedClipboardDataMessage.GetData()), extendedClipboardDataMessage.GetDataLength());
}

// We've read some text from the remote server, and
// we need to copy it into the local clipboard.
// Called by ClientConnection::ReadServerCutText()
// adzm - 2010-07 - Extended clipboard
void ClientConnection::UpdateLocalClipboard(char *buf, int len)
{	
	if (m_opts->m_DisableClipboard || m_opts->m_ViewOnly)
		return;

	// Copy to wincontents replacing LF with CR-LF
	char *wincontents = new char[len * 2 + 1];

	int j = 0;;
	for (int i = 0; buf[i] != 0; i++, j++) {
		if (buf[i] == '\x0a') {
			wincontents[j++] = '\x0d';
			len++;
		}
		wincontents[j] = buf[i];
	}
	wincontents[j] = '\0';

    // The clipboard should not be modified by more than one thread at once
    {
        omni_mutex_lock l(m_clipMutex);
		ClipboardHolder holder(m_hwndcn);

		if (!holder.m_bIsOpen) {
			vnclog.Print(2, "UpdateLocalClipboard: Failed to open clipboard! Last error 0x%08x", GetLastError());
			delete [] wincontents;
			return;
        }
        if (! ::EmptyClipboard()) {
			vnclog.Print(2, "UpdateLocalClipboard: Failed to empty clipboard! Last error 0x%08x", GetLastError());
			delete [] wincontents;
			return;
        }
			
		int finalLen = strlen(wincontents) + 1;

        // Allocate a global memory object for the text. 
        HGLOBAL hglbCopy = GlobalAlloc(GMEM_DDESHARE, finalLen); // in bytes
        if (hglbCopy != NULL) { 
	        // Lock the handle and copy the text to the buffer.  
	        LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hglbCopy); 
			memcpy(lptstrCopy, wincontents, finalLen); // in bytes
	        lptstrCopy[finalLen - 1] = 0;    // null character 
	        GlobalUnlock(hglbCopy);          // Place the handle on the clipboard.  
			
			m_clipboard.m_strLastCutText = wincontents;

	        SetClipboardData(CF_TEXT, hglbCopy); 
        }
		
        delete [] wincontents;
    }
}

void ClientConnection::SaveClipboardPreferences()
{
	omni_mutex_lock l(m_clipMutex);

	{
		DWORD dwClipboardPrefs = 0;
		if (m_clipboard.settings.m_nLimitText > 0) {
			dwClipboardPrefs |= clipText;
		}
		if (m_clipboard.settings.m_nLimitRTF > 0) {
			dwClipboardPrefs |= clipRTF;
		}
		if (m_clipboard.settings.m_nLimitHTML > 0) {
			dwClipboardPrefs |= clipHTML;
		}
		//ofnInit();
		vnclog.Print(1, "Saving to %s\n", m_opts->getDefaultOptionsFileName());
		char buf[32];
		sprintf_s(buf, "%d", dwClipboardPrefs);
		WritePrivateProfileString("connection", "ClipboardPrefs", buf, m_opts->getDefaultOptionsFileName());
	}
}

bool ClientConnection::LoadClipboardPreferences()
{
	omni_mutex_lock l(m_clipMutex);
	DWORD dwClipboardPrefs = 0;
//	ofnInit();
	vnclog.Print(1, "Saving to %s\n", m_opts->getDefaultOptionsFileName());
	dwClipboardPrefs = clipText | clipRTF | clipHTML;
	dwClipboardPrefs = GetPrivateProfileInt("connection", "ClipboardPrefs", dwClipboardPrefs, m_opts->getDefaultOptionsFileName());
	dwClipboardPrefs |= clipText;
	if (!(dwClipboardPrefs & clipRTF)) {
		m_clipboard.settings.m_nLimitRTF = 0;
	}
	if (!(dwClipboardPrefs & clipHTML)) {
		m_clipboard.settings.m_nLimitHTML = 0;
	}
	return true;
}
