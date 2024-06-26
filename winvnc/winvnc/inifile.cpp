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


#include "stdhdrs.h"
#include "inifile.h"

char *g_szIniFile = 0;

IniFile::IniFile()
{
	if(g_szIniFile)
	{
		strcpy_s(myInifile,g_szIniFile);
	}
	else
	{
     char WORKDIR[MAX_PATH];
	if (GetModuleFileName(NULL, WORKDIR, MAX_PATH))
		{
		char* p = strrchr(WORKDIR, '\\');
		if (p == NULL) return;
		*p = '\0';
		}
	strcpy_s(myInifile,"");
	strcat_s(myInifile,WORKDIR);//set the directory
	strcat_s(myInifile,"\\");
	strcat_s(myInifile,INIFILE_NAME);
}
}

void
IniFile::IniFileSetSecure()
{
	if(g_szIniFile)
	{
		strcpy_s(myInifile,g_szIniFile);
	}
	else
	{
char WORKDIR[MAX_PATH];
	if (GetModuleFileName(NULL, WORKDIR, MAX_PATH))
		{
		char* p = strrchr(WORKDIR, '\\');
		if (p == NULL) return;
		*p = '\0';
		}
	strcpy_s(myInifile,"");
	strcat_s(myInifile,WORKDIR);//set the directory
	strcat_s(myInifile,"\\");
	strcat_s(myInifile,INIFILE_NAME);
}
}

void
IniFile::IniFileSetTemp(char *lpCmdLine)
{
	strcpy_s(myInifile,260,lpCmdLine);
}

void
IniFile::copy_to_secure()
{
	{
		char dir[MAX_PATH];

		char exe_file_name[MAX_PATH];
		GetModuleFileName(0, exe_file_name, MAX_PATH);

		strcpy_s(dir, exe_file_name);
		strcat_s(dir, " -settingshelper");
		strcat_s(dir, ":");
		strcat_s(dir, myInifile);

		STARTUPINFO          StartUPInfo;
		PROCESS_INFORMATION  ProcessInfo;
		HANDLE Token=NULL;
		HANDLE process=NULL;
		ZeroMemory(&StartUPInfo,sizeof(STARTUPINFO));
		ZeroMemory(&ProcessInfo,sizeof(PROCESS_INFORMATION));
		StartUPInfo.wShowWindow = SW_SHOW;
		StartUPInfo.lpDesktop = "Winsta0\\Default";
		StartUPInfo.cb = sizeof(STARTUPINFO);
		HWND tray = FindWindow(("Shell_TrayWnd"), 0);
		if (!tray)
			goto error;

		DWORD processId = 0;
			GetWindowThreadProcessId(tray, &processId);
		if (!processId)
			goto error;
		process = OpenProcess(MAXIMUM_ALLOWED, FALSE, processId);
		if (!process)
			goto error;
		OpenProcessToken(process, MAXIMUM_ALLOWED, &Token);
		CreateProcessAsUser(Token,NULL,dir,NULL,NULL,FALSE,DETACHED_PROCESS,NULL,NULL,&StartUPInfo,&ProcessInfo);
		DWORD error=GetLastError();
		if (process) CloseHandle(process);
		if (Token) CloseHandle(Token);
		if (ProcessInfo.hThread) CloseHandle (ProcessInfo.hThread);
		if (ProcessInfo.hProcess) CloseHandle (ProcessInfo.hProcess);
		if (error == 1314) goto error;
		return;
	}
		error:
		settingsHelpers::Set_settings_as_admin(myInifile);
}

IniFile::~IniFile()
{
}

bool
IniFile::WriteString(char *key1, char *key2,char *value)
{
	//vnclog.Print(LL_INTERR, VNCLOG("%s \n"),myInifile);
	return (FALSE != WritePrivateProfileString(key1,key2, value,myInifile));
}

bool
IniFile::WriteInt(char *key1, char *key2,int value)
{
	char       buf[32];
	wsprintf(buf, "%d", value);
	//vnclog.Print(LL_INTERR, VNCLOG("%s \n"),myInifile);
	int result=WritePrivateProfileString(key1,key2, buf,myInifile);
	if (result==0) return false;
	return true;
}

int
IniFile::ReadInt(char *key1, char *key2,int Defaultvalue)
{
	//vnclog.Print(LL_INTERR, VNCLOG("%s \n"),myInifile);
	return GetPrivateProfileInt(key1, key2, Defaultvalue, myInifile);
}

void
IniFile::ReadString(char *key1, char *key2,char *value,int valuesize)
{
	//vnclog.Print(LL_INTERR, VNCLOG("%s \n"),myInifile);
	GetPrivateProfileString(key1,key2, "",value,valuesize,myInifile);
}

void
IniFile::ReadPassword(char *value,int valuesize)
{
	//int size=ReadInt("UltraVNC", "passwdsize",0);
	//vnclog.Print(LL_INTERR, VNCLOG("%s \n"),myInifilePasswd);
	GetPrivateProfileStruct("UltraVNC","passwd",value,8,myInifile);
}

void //PGM
IniFile::ReadPassword2(char *value,int valuesize) //PGM
{ //PGM
	GetPrivateProfileStruct("UltraVNC","passwd2",value,8,myInifile); //PGM
} //PGM

bool
IniFile::WritePassword(char *value)
{
		//WriteInt("UltraVNC", "passwdsize",sizeof(value));
		//vnclog.Print(LL_INTERR, VNCLOG("%s \n"),myInifile);
		return (FALSE != WritePrivateProfileStruct("UltraVNC","passwd", value,8,myInifile));
}

bool //PGM
IniFile::WritePassword2(char *value) //PGM
{ //PGM
		return (FALSE != WritePrivateProfileStruct("UltraVNC","passwd2", value,8,myInifile)); //PGM
} //PGM

bool IniFile::IsWritable()
{
    bool writable = WriteInt("Permissions", "isWritable",1);
    if (writable)
        WritePrivateProfileSection("Permissions", "", myInifile);

    return writable;
}