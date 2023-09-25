/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2023 see Authors.txt
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

#include <afxtaskdialog.h>
#include "DSUtil/HTTPAsync.h"

// CSaveDlg dialog

class CSaveDlg : public CTaskDialog
{
	DECLARE_DYNAMIC(CSaveDlg)

private:
	const CStringW m_name;
	const std::vector<std::pair<CStringW, CStringW>> m_saveItems;
	CStringW m_ffmpegpath;

	HICON m_hIcon;

	CComPtr<IGraphBuilder>   m_pGB;
	CComQIPtr<IMediaControl> m_pMC;
	CComQIPtr<IMediaSeeking> m_pMS;

	enum protocol {
		PROTOCOL_NONE,
		PROTOCOL_UDP,
		PROTOCOL_HTTP,
	};
	protocol m_protocol = protocol::PROTOCOL_NONE;

	UINT64  m_len = 0;

	struct {
		struct {
			UINT64 len;
			clock_t time;
		} LenTimes[40] = {}; // about 8 seconds
		unsigned idx = 0;

		long AddValuesGetSpeed(const UINT64 len, const long time)
		{
			LenTimes[idx] = { len, time };

			idx++;
			if (idx >= std::size(LenTimes)) {
				idx = 0;
			}

			const auto start_idx = LenTimes[idx].time ? idx : 0;

			long time_interval = time - LenTimes[start_idx].time;
			UINT64 len_increment = len - LenTimes[start_idx].len;

			long speed = time_interval ? (long)(len_increment * 1000 / time_interval) : 0;

			return speed;
		}

		void Reset() {
			ZeroMemory(LenTimes, sizeof(LenTimes));
			idx = 0;
		}
	} m_SaveStats;

	std::thread        m_SaveThread;
	std::atomic_ullong m_pos       = 0;
	std::atomic_int    m_iProgress = -1;
	std::atomic_bool   m_bAbort    = false;

	int m_iPrevState = -1;

	void SaveUDP();
	void SaveHTTP();
	HRESULT DownloadHTTP(CHTTPAsync& httpAsync, const CStringW url, const CStringW filepath);

	SOCKET m_UdpSocket = INVALID_SOCKET;
	WSAEVENT m_WSAEvent = nullptr;
	sockaddr_in m_addr = {};

public:
	CSaveDlg(const CStringW& name, const std::list<std::pair<CStringW, CStringW>>& saveItems, HRESULT& hr);

	void SetFFmpegPath(const CStringW& ffmpegpath);
	bool IsCompleteOk();

private:
	HRESULT OnDestroy() override;
	HRESULT OnTimer(_In_ long lTime) override;
	HRESULT OnCommandControlClick(_In_ int nCommandControlID) override;

	HRESULT InitFileCopy();
};
