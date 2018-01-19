/*
 * (C) 2006-2018 see Authors.txt
 *
 * This file is part of MPC-BE.
 *
 * MPC-BE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-BE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <atlcoll.h>
#include <deque>
#include <mutex>
#include <mpc_defines.h>

#define PACKET_AAC_RAW 0x0001

class CPacket : public CAtlArray<BYTE>
{
public:
	DWORD TrackNumber      = 0;
	BOOL bDiscontinuity    = FALSE;
	BOOL bSyncPoint        = FALSE;
	REFERENCE_TIME rtStart = INVALID_TIME;
	REFERENCE_TIME rtStop  = INVALID_TIME;
	AM_MEDIA_TYPE* pmt     = nullptr;

	DWORD Flag             = 0;

	virtual ~CPacket() {
		DeleteMediaType(pmt);
	}
	void SetData(const void* ptr, DWORD len) {
		SetCount(len);
		memcpy(GetData(), ptr, len);
	}
};

class CPacketQueue : protected std::deque<CAutoPtr<CPacket>>
{
	size_t m_size = 0;
	std::mutex m_mutex;

public:
	void Add(CAutoPtr<CPacket> p);
	CAutoPtr<CPacket> Remove();
	void RemoveAll();
	const size_t GetCount();
	const size_t GetSize();
	const REFERENCE_TIME GetDuration();
};
