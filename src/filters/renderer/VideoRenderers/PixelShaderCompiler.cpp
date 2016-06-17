/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2016 see Authors.txt
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

#include "stdafx.h"
#include "../../../apps/mplayerc/resource.h"
#include "RenderersSettings.h"
#include "PixelShaderCompiler.h"

HINSTANCE GetD3dcompilerDll()
{
	static HINSTANCE s_hD3dcompilerDll = NULL;

	if (s_hD3dcompilerDll == NULL) {
		s_hD3dcompilerDll = LoadLibrary(L"D3dcompiler_43.dll"); // Microsoft DirectX SDK (June 2010)
	}

	return s_hD3dcompilerDll;
}

CPixelShaderCompiler::CPixelShaderCompiler(IDirect3DDevice9* pD3DDev, bool fStaySilent)
	: m_pD3DDev(pD3DDev)
	, m_fnD3DCompile(NULL)
	, m_fnD3DDisassemble(NULL)
{
	HINSTANCE hDll;
	hDll = GetD3dcompilerDll();

	if (hDll) {
		m_fnD3DCompile = (D3DCompilePtr)GetProcAddress(hDll, "D3DCompile");
		m_fnD3DDisassemble = (D3DDisassemblePtr)GetProcAddress(hDll, "D3DDisassemble");
	}

	if (!fStaySilent) {
		if (!hDll) {
			AfxMessageBox(ResStr(IDS_PIXELSHADERCOMPILER_0), MB_OK);
		} else if (!m_fnD3DCompile || !m_fnD3DDisassemble) {
			AfxMessageBox(ResStr(IDS_PIXELSHADERCOMPILER_1), MB_OK);
		}
	}
}

CPixelShaderCompiler::~CPixelShaderCompiler()
{
}

HRESULT CPixelShaderCompiler::CompileShader(
	LPCSTR pSrcData,
	LPCSTR pFunctionName,
	LPCSTR pProfile,
	DWORD Flags,
	const D3D_SHADER_MACRO* pDefines,
	IDirect3DPixelShader9** ppPixelShader,
	CString* errmsg,
	CString* disasm)
{
	if (!m_fnD3DCompile || !m_fnD3DDisassemble) {
		return E_FAIL;
	}

	HRESULT hr;
	ID3DBlob* pShader, *pErrorMsgs;
	hr = m_fnD3DCompile(pSrcData, (SIZE_T)strlen(pSrcData), NULL, pDefines, NULL, pFunctionName, pProfile, Flags, 0, &pShader, &pErrorMsgs);

	if (FAILED(hr)) {
		if (errmsg) {
			CStringA msg = "Unexpected compiler error";

			if (pErrorMsgs) {
				int len = pErrorMsgs->GetBufferSize();
				memcpy(msg.GetBufferSetLength(len), pErrorMsgs->GetBufferPointer(), len);
			}

			*errmsg = msg;
		}

		return hr;
	}

	if (ppPixelShader) {
		if (!m_pD3DDev) {
			return E_FAIL;
		}
		hr = m_pD3DDev->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), ppPixelShader);
		if (FAILED(hr)) {
			return hr;
		}
	}

	if (disasm) {
		ID3DBlob* pDisAsm;
		hr = m_fnD3DDisassemble(pShader->GetBufferPointer(), pShader->GetBufferSize(), 0, NULL, &pDisAsm);
		if (SUCCEEDED(hr) && pDisAsm) {
			*disasm = CStringA((const char*)pDisAsm->GetBufferPointer());
		}
	}

	return S_OK;
}
