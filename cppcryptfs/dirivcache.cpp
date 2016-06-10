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

#include "stdafx.h"

#include <windows.h>

#include "dirivcache.h"

#include "fileutil.h"

DirIvCacheNode::DirIvCacheNode()
{
}

DirIvCacheNode::~DirIvCacheNode()
{
}



DirIvCache::DirIvCache()
{
	m_lookups = 0;
	m_hits = 0;

	m_map.reserve(DIR_IV_CACHE_ENTRIES);

	InitializeCriticalSection(&m_crit);
}

DirIvCache::~DirIvCache()
{

	for (auto it = m_lru_list.begin(); it != m_lru_list.end(); it++) {
		DirIvCacheNode *node = *it;
		delete node;
	}
}

void DirIvCache::normalize_key(std::wstring& key)
{
	if (key.size() > 0 && key[key.size() - 1] != '\\') {
		key.push_back('\\');
	}
}

bool DirIvCache::lookup(LPCWSTR path, unsigned char *dir_iv)
{
	std::wstring key = path;

	bool found;

	normalize_key(key);

	EnterCriticalSection(&m_crit);

	m_lookups++;

	auto it = m_map.find(key);

	if (it != m_map.end()) {
		DirIvCacheNode *node = it->second;
		memcpy(dir_iv, node->m_dir_iv, DIR_IV_LEN);
		if (node->m_list_it != m_lru_list.begin()) {
			m_lru_list.erase(node->m_list_it);
			m_lru_list.push_front(node);
			node->m_list_it = m_lru_list.begin();
		}
		found = true;
		m_hits++;
	}
	else {
		found = false;
	}

	if (m_lookups && (m_lookups % 1024 == 0)) {
		double ratio = (double)m_hits / (double)m_lookups;
		DbgPrint(L"DirIvCache: %I64d lookups, %I64d hits, %I64d misses, hit ratio %0.2f%%\n", m_lookups, m_hits, m_lookups - m_hits, ratio*100);
	}

	LeaveCriticalSection(&m_crit);

	return found;
}


bool DirIvCache::store(LPCWSTR path, const unsigned char *dir_iv)
{

	EnterCriticalSection(&m_crit);

	DirIvCacheNode *node = NULL;

	if (m_map.size() >= DIR_IV_CACHE_ENTRIES) {
		node = m_lru_list.back();
		m_lru_list.pop_back();
		m_map.erase(node->m_key);
	}

	if (!node)
		node = new DirIvCacheNode;

	memcpy(node->m_dir_iv, dir_iv, DIR_IV_LEN);

	node->m_key = path;

	normalize_key(node->m_key);

	node->m_list_it = m_lru_list.insert(m_lru_list.begin(), node);

	m_map.insert_or_assign(node->m_key, node);

	LeaveCriticalSection(&m_crit);

	return true;
}

void DirIvCache::remove(LPCWSTR path)
{
	std::wstring key = path;

	normalize_key(key);

	EnterCriticalSection(&m_crit);

	auto it = m_map.find(key);

	if (it != m_map.end()) {

		DirIvCacheNode *node = it->second;

		m_map.erase(it);

		m_lru_list.erase(node->m_list_it);

		delete node;
	}

	LeaveCriticalSection(&m_crit);
}