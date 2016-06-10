/*
cppcryptfs : user-mode cryptographic virtual overlay filesystem.

Copyright (C) 2016 - Bailey Brown (github.com/bailey27/cppcryptfs)

cppcryptfs is based on the design of gocryptfs (github.com/rfjakob/gocryptfs)

The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#include <windows.h>
#include <vector>


class CryptConfig
{
public:
	int m_N;
	int m_R;
	int m_P;
	int m_KeyLen;

	bool m_PlaintextNames;
private:
	bool m_DirIV;
public:
	bool DirIV() { return m_DirIV; };
	bool m_EMENames;
	bool m_GCMIV128;
	bool m_LongNames;

	int m_Version;
	std::wstring m_VolumeName;

	std::vector<unsigned char> m_encrypted_key_salt;
	std::vector<unsigned char> m_encrypted_key;
	unsigned char * m_key;

	std::wstring m_basedir;

	char m_driveletter;

	const unsigned char *GetKey() { return m_key; };
	WCHAR GetDriveLetter() { return m_driveletter; };
	const WCHAR *GetBaseDir() { return &m_basedir[0]; }

	CryptConfig();
	bool read();
	bool decrypt_key(LPCTSTR password);

	bool create(const WCHAR *path, const WCHAR *password, bool eme, bool plaintext, bool longfilenames, const WCHAR *volume_name, std::wstring& error_mes);

	bool check_config(std::wstring& mes);

	bool write_volume_name();

	virtual ~CryptConfig();
};


