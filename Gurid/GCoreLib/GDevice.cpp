#include "GDevice.h"
ID3D11Device*			g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext=NULL;
HRESULT GDevice::InitDevice( HWND hWnd, UINT iWidth, UINT iHeight, BOOL IsFullScreen )
{	
	HRESULT hr = S_OK;	
	if( FAILED( hr = CreateDevice() ))
	{
		MessageBox( 0, _T("CreateDevice  실패"), _T("Fatal error"), MB_OK );
		return hr;
	}	
	if( FAILED( hr = CreateSwapChain(hWnd,iWidth, iHeight, IsFullScreen) ) )
	{
		MessageBox( 0, _T("CreateSwapChain  실패"), _T("Fatal error"), MB_OK );
		return hr;
	}	
	if( FAILED( hr = SetRenderTargetView() ) )
	{
		MessageBox( 0, _T("SetRenderTargetView  실패"), _T("Fatal error"), MB_OK );
		return hr;
	}
	if( FAILED( hr = SetViewPort() ) )
	{
		MessageBox( 0, _T("SetViewPort  실패"), _T("Fatal error"), MB_OK );
		return hr;
	}	
	// 각종 뷰 리소스 적용
	if( FAILED( hr = CreateDxResource() ))
	{
		return hr;
	}
	// Alt + Enter 키를 막는다.
	if( FAILED( hr = GetGIFactory()->MakeWindowAssociation( hWnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER  )))
	{
		return hr;
	}
	return hr;
}

BOOL GDevice::GetFullScreenFlag()
{
	return m_IsFullScreenMode;
}
void GDevice::SetFullScreenFlag( BOOL bFlag )
{
	m_IsFullScreenMode = bFlag;
}
// New
HRESULT GDevice::ResizeDevice( UINT iWidth, UINT iHeight)
{
	if( m_pd3dDevice == NULL ) return true;
	HRESULT hr;
	// 바인딩되어 있는 각종 뷰리소스를 해제한다.
	if( FAILED( hr= DeleteDxResource()))
	{
		return hr;
	}
	//--------------------------------------------------------------------------------------
	// 랜더타켓과 깊이스텐실 버퍼를 해제한다.
	//--------------------------------------------------------------------------------------
	m_pImmediateContext->OMSetRenderTargets( 0, NULL, NULL );
	m_DefaultRT.m_pRenderTargetView.ReleaseAndGetAddressOf();	

	//--------------------------------------------------------------------------------------
	// 백버퍼의 크기를 조정한다.
	//--------------------------------------------------------------------------------------
	m_pSwapChain->ResizeBuffers(m_SwapChainDesc.BufferCount, iWidth, iHeight, m_SwapChainDesc.BufferDesc.Format, m_SwapChainDesc.Flags );

	// 랜더타켓뷰 생성 및 적용한다.
    if( FAILED( hr = SetRenderTargetView() ) ) 
	{
		return hr;
	}
	// 뷰포트를 세팅하고 적용한다.
	if( FAILED( hr = SetViewPort() ) )
	{
		return hr;
	}

	if( FAILED( hr= CreateDxResource()))
	{
		return hr;
	}
	return S_OK;
}
ID3D11Device* GDevice::GetDevice()
{
	assert( m_pd3dDevice );
	return m_pd3dDevice; 
}
ID3D11DeviceContext* GDevice::GetContext()
{
	assert( m_pImmediateContext );
	return m_pImmediateContext; 
}
IDXGISwapChain* GDevice::GetSwapChain()
{ 
	assert( m_pSwapChain );
	return m_pSwapChain; 
}
ID3D11RenderTargetView* GDevice::GetRenderTargetView()	
{ 
	return m_DefaultRT.m_pRenderTargetView.Get();
}
IDXGIFactory* GDevice::GetGIFactory()
{
	assert(m_pGIFactory);
	return m_pGIFactory;
}

//--------------------------------------------------------------------------------------
// DXGIFactory 인터페이스로부터 IDXGISwapChain 인터페이스를 생성한다.
//--------------------------------------------------------------------------------------
HRESULT GDevice::CreateSwapChain( HWND hWnd, UINT iWidth, UINT iHeight, BOOL IsFullScreen)
{	
	HRESULT hr = S_OK;
	SetFullScreenFlag( IsFullScreen );
	if( m_pGIFactory == NULL ) return S_FALSE;
	DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width		= iWidth;
    sd.BufferDesc.Height	= iHeight;
    sd.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
     sd.Windowed = !m_IsFullScreenMode;
	// 추가
	//sd.Flags	= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	if( FAILED( hr = m_pGIFactory->CreateSwapChain( m_pd3dDevice, &sd, &m_pSwapChain ) ))
	{
		return hr;
	}	
	if (FAILED(hr = m_pSwapChain->GetDesc(&m_SwapChainDesc)))
	{
		return hr;
	}
	return hr;
}
//--------------------------------------------------------------------------------------
// DXGIFactory 인터페이스로부터 IDXGISwapChain 인터페이스를 생성한다.
//--------------------------------------------------------------------------------------
HRESULT GDevice::CreateDeviceAndSwapChain(HWND hWnd, UINT iWidth, UINT iHeight, BOOL IsFullScreen)
{	
	HRESULT hr = S_OK;

    UINT createDeviceFlags =  D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

	D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount				= 1;
    sd.BufferDesc.Width			= iWidth;
    sd.BufferDesc.Height		= iHeight;
    sd.BufferDesc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage				= DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow				= hWnd;
    sd.SampleDesc.Count			= 1;
    sd.SampleDesc.Quality		= 0;
    sd.Windowed = !m_IsFullScreenMode;
	// 추가
	sd.Flags	= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        m_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, m_driverType, NULL, createDeviceFlags,featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &m_FeatureLevel, &m_pImmediateContext );
        if( SUCCEEDED( hr ) )
		{
            if( FAILED( hr ) || m_FeatureLevel < D3D_FEATURE_LEVEL_11_0)
			{
				if( m_pImmediateContext ) m_pImmediateContext->Release();
				if( m_pd3dDevice ) m_pd3dDevice->Release();
				continue;
			}
			break;
		}
    }
    if( FAILED( hr ) )
	{
        return hr;
	}
	if (FAILED(hr = m_pSwapChain->GetDesc(&m_SwapChainDesc)))
	{
		return hr;
	}
	g_pd3dDevice = m_pd3dDevice;
	g_pImmediateContext = m_pImmediateContext;
	return hr;
}

//--------------------------------------------------------------------------------------
// ID3D11Device 인터페이스를 생성한다.
//--------------------------------------------------------------------------------------
HRESULT GDevice::CreateDevice()
{
	HRESULT hr;
	  UINT createDeviceFlags =  D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	// win10->설정->시스템/앱 및 기능/선택적 기능 관리/기능 추가/그래픽 도구(DirectX Graphics Tools지원 추가) 추가 선택하면 된다.
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

	D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        m_driverType = driverTypes[driverTypeIndex];
        if( SUCCEEDED( hr = D3D11CreateDevice(	NULL, m_driverType, NULL, createDeviceFlags,
												featureLevels, numFeatureLevels,
												D3D11_SDK_VERSION, &m_pd3dDevice, &m_FeatureLevel, &m_pImmediateContext ) ))
		{			
			if( FAILED( hr ) || m_FeatureLevel < D3D_FEATURE_LEVEL_11_0)
			{
				if( m_pImmediateContext ) m_pImmediateContext->Release();
				if( m_pd3dDevice ) m_pd3dDevice->Release();
				continue;
			}
			break;
		}
    }
    if( FAILED( hr ) )       return hr;

	g_pd3dDevice = m_pd3dDevice;
	g_pImmediateContext = m_pImmediateContext;

	if (FAILED( hr=GDxState::SetState(m_pd3dDevice))) return hr;

	return CreateGIFactory();
}
//--------------------------------------------------------------------------------------
// DXGIFactory 인터페이스를 생성한다.
//--------------------------------------------------------------------------------------
HRESULT GDevice::CreateGIFactory()
{	
	if(m_pd3dDevice==NULL) return E_FAIL;
	HRESULT hr;// = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&m_pGIFactory) );
	IDXGIDevice * pDXGIDevice;
	hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&pDXGIDevice);
      
	IDXGIAdapter * pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&pDXGIAdapter);

	IDXGIFactory * pIDXGIFactory;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&m_pGIFactory);
	
	pDXGIDevice->Release();
	pDXGIAdapter->Release();

	return S_OK;
}
//--------------------------------------------------------------------------------------
// DXGIFactory 인터페이스로부터 IDXGISwapChain 인터페이스를 생성한다.
//--------------------------------------------------------------------------------------

HRESULT GDevice::SetRenderTargetView()
{
	HRESULT hr = S_OK;
	 // Create a render target view
    ID3D11Texture2D* pBackBuffer;
    hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, m_DefaultRT.m_pRenderTargetView.GetAddressOf() );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;       	
	return S_OK;
}

HRESULT GDevice::SetViewPort()
{
	HRESULT hr = S_OK;
	m_DefaultRT.m_vp.Width	= m_SwapChainDesc.BufferDesc.Width;
	m_DefaultRT.m_vp.Height	= m_SwapChainDesc.BufferDesc.Height;
	m_DefaultRT.m_vp.MinDepth = 0.0f;
	m_DefaultRT.m_vp.MaxDepth = 1.0f;
	m_DefaultRT.m_vp.TopLeftX = 0;
	m_DefaultRT.m_vp.TopLeftY = 0;
	return S_OK;
}
void GDevice::ClearD3D11DeviceContext( ID3D11DeviceContext* pd3dDeviceContext )
{
    // Unbind all objects from the immediate context
    if (pd3dDeviceContext == NULL) return;

    ID3D11ShaderResourceView* pSRVs[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    ID3D11RenderTargetView* pRTVs[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    ID3D11DepthStencilView* pDSV = NULL;
    ID3D11Buffer* pBuffers[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    ID3D11SamplerState* pSamplers[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    UINT StrideOffset[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

    // Shaders
    pd3dDeviceContext->VSSetShader( NULL, NULL, 0 );
    pd3dDeviceContext->HSSetShader( NULL, NULL, 0 );
    pd3dDeviceContext->DSSetShader( NULL, NULL, 0 );
    pd3dDeviceContext->GSSetShader( NULL, NULL, 0 );
    pd3dDeviceContext->PSSetShader( NULL, NULL, 0 );

    // IA clear
    pd3dDeviceContext->IASetVertexBuffers( 0, 16, pBuffers, StrideOffset, StrideOffset );
    pd3dDeviceContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
    pd3dDeviceContext->IASetInputLayout( NULL );

    // Constant buffers
    pd3dDeviceContext->VSSetConstantBuffers( 0, 14, pBuffers );
    pd3dDeviceContext->HSSetConstantBuffers( 0, 14, pBuffers );
    pd3dDeviceContext->DSSetConstantBuffers( 0, 14, pBuffers );
    pd3dDeviceContext->GSSetConstantBuffers( 0, 14, pBuffers );
    pd3dDeviceContext->PSSetConstantBuffers( 0, 14, pBuffers );

    // Resources
    pd3dDeviceContext->VSSetShaderResources( 0, 16, pSRVs );
    pd3dDeviceContext->HSSetShaderResources( 0, 16, pSRVs );
    pd3dDeviceContext->DSSetShaderResources( 0, 16, pSRVs );
    pd3dDeviceContext->GSSetShaderResources( 0, 16, pSRVs );
    pd3dDeviceContext->PSSetShaderResources( 0, 16, pSRVs );

    // Samplers
    pd3dDeviceContext->VSSetSamplers( 0, 16, pSamplers );
    pd3dDeviceContext->HSSetSamplers( 0, 16, pSamplers );
    pd3dDeviceContext->DSSetSamplers( 0, 16, pSamplers );
    pd3dDeviceContext->GSSetSamplers( 0, 16, pSamplers );
    pd3dDeviceContext->PSSetSamplers( 0, 16, pSamplers );

    // Render targets
    pd3dDeviceContext->OMSetRenderTargets( 8, pRTVs, pDSV );

    // States
    FLOAT blendFactor[4] = { 0,0,0,0 };
    pd3dDeviceContext->OMSetBlendState( NULL, blendFactor, 0xFFFFFFFF );
    pd3dDeviceContext->OMSetDepthStencilState( NULL, 0 );
    pd3dDeviceContext->RSSetState( NULL );
}

bool GDevice::CleanupDevice()
{
	ClearD3D11DeviceContext(m_pImmediateContext);
	// 바인딩되어 있는 각종 뷰리소스를 해제한다.
	if( FAILED( DeleteDxResource()))
	{
		return false;
	}
       // Clear state and flush
	// ComPtr에 Attach 이후에는 Release하면 안된다.
    if( m_pImmediateContext )    m_pImmediateContext->ClearState();
    if( m_pImmediateContext )    m_pImmediateContext->Flush();  
    if( m_pSwapChain ) m_pSwapChain->Release();
	if( m_pImmediateContext ) m_pImmediateContext->Release();
    if( m_pd3dDevice ) m_pd3dDevice->Release();
	if( m_pGIFactory ) m_pGIFactory->Release();
	m_pd3dDevice		= NULL;
	m_pSwapChain		= NULL;
	m_pImmediateContext = NULL;
	m_pGIFactory		= NULL;
	return true;
}
HRESULT GDevice::CreateDxResource()
{
	return S_OK;
}
HRESULT GDevice::DeleteDxResource()
{
	return S_OK;
}
GDevice::GDevice(void)
{
	m_driverType		= D3D_DRIVER_TYPE_NULL;
	m_FeatureLevel		= D3D_FEATURE_LEVEL_11_0;
	m_pd3dDevice		= NULL;
	m_pSwapChain		= NULL;
	m_pImmediateContext = NULL;
}

GDevice::~GDevice(void)
{
}
