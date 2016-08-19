#include "_StdAfx.h"


GControlUI* GGameMenu::AddRect(GUI_TYPE type)
{
	GControlUI* pUIControl = NULL;
	switch (type)
	{
	case T_BUTTON: {
		pUIControl = new GButtonCtl();
		pUIControl->Create(g_pMain->GetDevice());
		pUIControl->Scale(50, 50, 0);
	}
	case T_EDIT: {
		pUIControl = new GEditCtl();
		pUIControl->Create(g_pMain->GetDevice());
		pUIControl->Scale(50, 50, 0);
	}
	}
	pUIControl->Set(g_pMain->m_SwapChainDesc.BufferDesc.Width, g_pMain->m_SwapChainDesc.BufferDesc.Height);
	m_pUIList.push_back(pUIControl);
	return pUIControl;
}
GControlUI* GGameMenu::SelectRect()
{
	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(g_pMain->m_hWnd, &mouse);
	for (int iRect = 0; iRect < m_pUIList.size(); iRect++)
	{
		GControlUI* pRect = m_pUIList[iRect];
		if (I_Input.m_MouseState[0] == KEY_HOLD)
		{
			RECT rt = pRect->m_rt;
			// 뒤집어 진 경우
			if (pRect->m_rt.left > pRect->m_rt.right)
			{
				rt.left = pRect->m_rt.right;
				rt.right = pRect->m_rt.left;
			}
			if (pRect->m_rt.top > pRect->m_rt.bottom)
			{
				rt.top = pRect->m_rt.bottom;
				rt.bottom = pRect->m_rt.top;
			}
			if (rt.left <= mouse.x && rt.right >= mouse.x)
			{
				if (rt.top <= mouse.y && rt.bottom >= mouse.y)
				{
					return pRect;
				}
			}
		}
	}
	return NULL;
}
bool GGameMenu::Init()
{
	I_Ime.ImeUi_Initialize(g_pMain->m_hWnd);

	GImageCtl* pImageCtl = new GImageCtl();
	pImageCtl->Set(g_pMain->m_SwapChainDesc.BufferDesc.Width, g_pMain->m_SwapChainDesc.BufferDesc.Height);
	pImageCtl->Create(g_pMain->GetDevice(), nullptr, L"data/menu/menu.png");
	pImageCtl->Scale(400 - 1.0f, 300 - 1.0f, 1.0f - 1.0f);
	pImageCtl->Move(0, 0, 100);
	m_pUIList.push_back(pImageCtl);

	//TButtonCtl* pBoxCtl = new TButtonCtl();
	//pBoxCtl->Set(m_SwapChainDesc.BufferDesc.Width, m_SwapChainDesc.BufferDesc.Height);
	//pBoxCtl->Create(GetDevice(), nullptr, L"_bak/ui/exit_lg.bmp");
	//pBoxCtl->Scale(100 - 1.0f, 50 - 1.0f, 1 - 1.0f);
	//pBoxCtl->Move(0, 0, 0);
	//m_pUIList.push_back(pBoxCtl);

#pragma region g_pTextureSRV_g_pSamplerState
	m_pTextureSRV[0].Attach(CreateShaderResourceView(g_pMain->GetDevice(), L"_bak/ui/Background.bmp"));
	m_pTextureSRV[1].Attach(CreateShaderResourceView(g_pMain->GetDevice(), L"_bak/ui/_connect.bmp"));
	m_pTextureSRV[2].Attach(CreateShaderResourceView(g_pMain->GetDevice(), L"_bak/ui/_exit_lg.bmp"));
#pragma endregion
	return true;
}
bool GGameMenu::Frame()
{
	if (I_Input.KeyCheck(DIK_P) == KEY_UP)
	{
		m_pSelectPlane = AddRect(T_BUTTON);
	}
	if (I_Input.KeyCheck(DIK_O) == KEY_UP)
	{
		m_pSelectPlane = AddRect(T_EDIT);
	}

	GControlUI* pSelect = SelectRect();

	if (pSelect != NULL && m_pSelectPlane != pSelect)
	{
		m_pSelectPlane = pSelect;
	}

	if (m_pSelectPlane)
	{
		D3DXVECTOR3 vPos(0, 0, 0);
		D3DXVECTOR3 vScale(0, 0, 0);
		if (g_InputData.bQKey)
		{
			vScale.x += 50 * g_fSecPerFrame;
		}
		if (g_InputData.bEKey)
		{
			vScale.x += -50 * g_fSecPerFrame;
		}
		if (g_InputData.bZKey)
		{
			vScale.y += 50 * g_fSecPerFrame;
		}
		if (g_InputData.bCKey)
		{
			vScale.y += -50 * g_fSecPerFrame;
		}
		if (g_InputData.bWKey)
		{
			vPos.y = 50 * g_fSecPerFrame;
		}
		if (g_InputData.bSKey)
		{
			vPos.y = -50 * g_fSecPerFrame;
		}
		if (g_InputData.bAKey)
		{
			vPos.x = -50 * g_fSecPerFrame;
		}
		if (g_InputData.bDKey)
		{
			vPos.x = 50 * g_fSecPerFrame;
		}
		m_pSelectPlane->Move(vPos.x, vPos.y, vPos.z);
		m_pSelectPlane->Scale(vScale.x, vScale.y, vScale.z);
	}

	for (int iPlane = 0; iPlane < m_pUIList.size(); iPlane++)
	{
		GControlUI* pRect = m_pUIList[iPlane];
		pRect->Update();
	}
	return true;
}
bool GGameMenu::Render()
{
	D3DXMATRIX matTrans, matRotation, matZ;
	// 2초당 1회전( 1 초 * D3DX_PI = 3.14 )
	float t = g_pMain->m_Timer.GetElapsedTime() * D3DX_PI;

	//D3DXMatrixTranspose(&m_cbData.matProj, &m_Projection[1]);
	for (int iPlane = 0; iPlane < m_pUIList.size(); iPlane++)
	{
		GControlUI* pRect = m_pUIList[iPlane];
		pRect->Render(g_pMain->m_pImmediateContext);
	}
	return true;
}
bool GGameMenu::Release()
{
	for (int iPlane = 0; iPlane < m_pUIList.size(); iPlane++)
	{
		GControlUI* pRect = m_pUIList[iPlane];
		pRect->Release();
		delete pRect;
	}
	I_Ime.Uninitialize();
	return true;
}

//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
HRESULT GGameMenu::CreateResource()
{
	HRESULT hr;
	// 투영행렬도 윈도우 사이즈가 변경되면 반드시 업테이트 해야 한다.
	int iRectWidth = g_pMain->m_SwapChainDesc.BufferDesc.Width;
	int iRectHeight = g_pMain->m_SwapChainDesc.BufferDesc.Height;
	for (int iPlane = 0; iPlane < m_pUIList.size(); iPlane++)
	{
		GControlUI* pRect = m_pUIList[iPlane];
		if (pRect) pRect->CreateResource(iRectWidth, iRectHeight);
	}
	return S_OK;
}
//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
HRESULT GGameMenu::DeleteResource()
{
	HRESULT hr = S_OK;
	if (g_pMain->m_pImmediateContext) g_pMain->m_pImmediateContext->ClearState();
	return S_OK;
}
GGameMenu::GGameMenu(void)
{
	m_pSelectPlane = NULL;
}

GGameMenu::~GGameMenu(void)
{
}