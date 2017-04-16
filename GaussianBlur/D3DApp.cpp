#include "stdafx.h"
#include "D3DApp.h"
#include "Resource.h"
#pragma comment ( lib, "D3D11.lib")
#pragma warning(disable:4996)
#pragma comment(lib, "legacy_stdio_definitions.lib")

#define MAX_LOADSTRING 100
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
namespace
{
	//Global variables
	D3DApp *g_winApp(NULL);			//Application
}

//Message processing wrap function
LRESULT CALLBACK WinAppProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return g_winApp->WinProc(hwnd, uMsg, wParam, lParam);
}

D3DApp::D3DApp( std::wstring title  )
	:driverType_( D3D_DRIVER_TYPE_NULL )
	, featureLevel_( D3D_FEATURE_LEVEL_11_0 )
	, d3dDevice_( 0 ) , d3dContext_( 0 )
	, swapChain_( 0 ) , backBufferTarget_( 0 )
	, pBBufferSurfaceDesc_( 0 ), depthStencilView_(0)
	, pDepthStencil_( 0 ),m_winTitle(title)
	,m_bPaused(false)
{
	g_winApp = this;
	pBBufferSurfaceDesc_ = new DXGI_SURFACE_DESC;
}


D3DApp::~D3DApp()
{

}

void D3DApp::Shutdown()
{
#if defined(DEBUG) || defined(_DEBUG)  
	ID3D11Debug *d3dDebug;
	HRESULT hr = d3dDevice_->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	if (SUCCEEDED(hr))
	{
		hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
	if (d3dDebug != nullptr)            d3dDebug->Release();
#endif  
	if (d3dDevice_ != nullptr)            SAFE_RELEASE(d3dDevice_);

	UnLoadContent();
	if (backBufferTarget_)	SAFE_RELEASE(backBufferTarget_);
	if (swapChain_)			SAFE_RELEASE(swapChain_);
	if (d3dContext_)		SAFE_RELEASE(d3dContext_);
	if (depthStencilView_)	SAFE_RELEASE(depthStencilView_);
	if (pDepthStencil_)		SAFE_RELEASE(pDepthStencil_);
	
	if (pBBufferSurfaceDesc_) SAFE_DELETE(pBBufferSurfaceDesc_);
	pBBufferSurfaceDesc_ = NULL;
	backBufferTarget_ = 0;
	swapChain_ = 0;
	d3dContext_ = 0;
	d3dDevice_ = 0;
	pDepthStencil_ = 0;

}


bool D3DApp::Initialize(HINSTANCE hInstance)
{
	hInstance_ = hInstance;
	if (!InitWindow())
		return false;
	if (!InitD3D())
		return false;
	return true;

}

//Win32 initialization
bool D3DApp::InitWindow()
{
	WNDCLASSEXW wndcls;
	wndcls.cbSize = sizeof(WNDCLASSEX);
	wndcls.cbClsExtra = 0;
	wndcls.cbWndExtra = 0;
	wndcls.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hIcon = LoadIcon(hInstance_, MAKEINTRESOURCE(IDI_GAUSSIANBLUR));
	wndcls.hInstance = hInstance_;
	wndcls.lpfnWndProc = WinAppProc;
	wndcls.lpszClassName = m_winTitle.c_str();
	wndcls.lpszMenuName = MAKEINTRESOURCEW(IDI_GAUSSIANBLUR);
	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	wndcls.hIconSm = LoadIcon(wndcls.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if (!RegisterClassExW(&wndcls))
	{
		MessageBox(NULL, _T("Register class failed!"), _T("ERROR"), MB_OK);
		return false;
	}

	// 初始化全局字符串
	LoadStringW(hInstance_, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance_, IDC_GAUSSIANBLUR, szWindowClass, MAX_LOADSTRING);

	hwnd_ = CreateWindowW(m_winTitle.c_str(), szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance_, nullptr);
	
	if (!hwnd_)
	{
		MessageBox(NULL, _T("Create Window failed!"), _T("ERROR"), MB_OK);
		return false;
	}

	ShowWindow(hwnd_, SW_SHOW);
	UpdateWindow(hwnd_);

	return true;
}

int D3DApp::Run()
{

	MSG msg = { 0 };

	m_timer.Reset();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//Running
			if (!m_bPaused)
			{
				//Timer update
				m_timer.Tick();
				//Frame rate update
				CalculateFPS();
				//Scene update and rendering
				Update(m_timer.DeltaTime());
				Render();
			}
			//Paused
			else
			{
				//Let it sleep to save CPU resource
				Sleep(200);
			}
		}
	}
	Shutdown();
	//Eixt
	return msg.wParam;
}
//Direct3D initialization
bool D3DApp::InitD3D()
{
	RECT	dimensions;
	GetClientRect(hwnd_, &dimensions);

	m_clientWidth = dimensions.right - dimensions.left;
	m_clientHeight = dimensions.bottom - dimensions.top;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE,
		D3D_DRIVER_TYPE_UNKNOWN
	};

	unsigned int totalDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	unsigned int totalfeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC	swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = m_clientWidth;
	swapChainDesc.BufferDesc.Height = m_clientHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd_;
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	unsigned int creationFlags = 0;

#ifdef DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DEBUG


	HRESULT result;
	unsigned int driver = 0;
	IDXGIAdapter *pAdapter_Set = NULL;

	for (driver = 0; driver < totalDriverTypes; ++driver)
	{
		result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0, creationFlags,
			featureLevels, totalfeatureLevels, D3D11_SDK_VERSION, &swapChainDesc, &swapChain_,
			&d3dDevice_, &featureLevel_, &d3dContext_);
		if (SUCCEEDED(result))
		{
			driverType_ = driverTypes[driver];
			break;
		}
	}
	if (FAILED(result))
	{
		DXTRACE_MSG(L"创建 Direct3D 设备失败!");
		return false;
	}

	//// set default render state to msaa enabled
	//D3D11_RASTERIZER_DESC drd = {
	//	D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
	//	D3D11_CULL_BACK,//D3D11_CULL_MODE CullMode;
	//	FALSE, //BOOL FrontCounterClockwise;
	//	0, //INT DepthBias;
	//	0.0f,//FLOAT DepthBiasClamp;
	//	0.0f,//FLOAT SlopeScaledDepthBias;
	//	TRUE,//BOOL DepthClipEnable;
	//	FALSE,//BOOL ScissorEnable;
	//	TRUE,//BOOL MultisampleEnable;
	//	FALSE//BOOL AntialiasedLineEnable;        
	//};
	//if (FAILED(result))
	//{
	//	DXTRACE_MSG(L"D3D11CreateDevice");
	//	return DX11_ERR_CREATINGDEVICE;
	//}
	//ID3D11RasterizerState* pRS = NULL;
	//d3dDevice_->CreateRasterizerState(&drd, &pRS);
	//d3dContext_->RSSetState(pRS);


	ID3D11Texture2D*	backBufferTexture;
	result = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
	if (FAILED(result))
	{
		DXTRACE_MSG(L"获取交换链后台缓存失败！");
		return false;
	}
	else if (SUCCEEDED(result))
	{
		UpdateBackBufferDesc();
	}
	D3D11_TEXTURE2D_DESC backBufferSurfaceDesc;
	backBufferTexture->GetDesc(&backBufferSurfaceDesc);
	result = d3dDevice_->CreateRenderTargetView(backBufferTexture, 0, &backBufferTarget_);
	if (backBufferTexture)
		backBufferTexture->Release();

	if (FAILED(result))
	{
		DXTRACE_MSG(L"创建渲染目标视图失败!");
		return false;
	}

	// Create depth stencil texture

	D3D11_TEXTURE2D_DESC descDepth;//深度/模板缓冲区描述
	descDepth.Width = backBufferSurfaceDesc.Width;//宽度、高度
	descDepth.Height = backBufferSurfaceDesc.Height;
	descDepth.MipLevels = 1;//多级渐进纹理层数量
	descDepth.ArraySize = 1; //纹理数量
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//纹理元素格式
	descDepth.SampleDesc.Count = 1;//多重采样
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;//纹理用途
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;//管线绑定标志值
	descDepth.CPUAccessFlags = 0; //CPU对资源访问权限
	descDepth.MiscFlags = 0;//与深度/模板无关的一个标志值
	result = d3dDevice_->CreateTexture2D(&descDepth, NULL, &pDepthStencil_);
	if (FAILED(result))
		return result;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.Flags = 0;
	if (descDepth.SampleDesc.Count > 1)
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	else
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	result = d3dDevice_->CreateDepthStencilView(pDepthStencil_, &descDSV, &depthStencilView_);
	if (FAILED(result))
		return result;

	d3dContext_->OMSetRenderTargets(1, &backBufferTarget_, depthStencilView_);

	D3D11_VIEWPORT		viewPort;
	viewPort.Width = static_cast<float>(pBBufferSurfaceDesc_->Width);
	viewPort.Height = static_cast<float>(pBBufferSurfaceDesc_->Height);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;

	d3dContext_->RSSetViewports(1, &viewPort);


	D3DXVECTOR3 eye(1.0f, -1.0f, -6.0f);
	D3DXVECTOR3 at(0, 0, 0);
	D3DXVECTOR3 up(0, 1, 0);

	// Bound the spotlight tightly to the bunny
	D3DXMATRIX view, proj,world;
	D3DXMatrixIdentity(&world);
	D3DXMatrixLookAtLH(&view, &eye, &at, &up);
	D3DXMatrixPerspectiveFovLH(&proj, 0.25f * D3DX_PI, m_clientWidth * 1.0f/m_clientHeight, 1, 1000);

	//mWorldViewProjection = world * view * proj;


	return LoadContent();
} 



//--------------------------------------------------------------------------------------
// Stores back buffer surface desc in GetDXUTState().GetBackBufferSurfaceDesc10()
//--------------------------------------------------------------------------------------
void D3DApp::UpdateBackBufferDesc()
{
	if (swapChain_)
	{
		HRESULT hr;
		ID3D11Texture2D* pBackBuffer;
		hr = swapChain_->GetBuffer(0, __uuidof(*pBackBuffer), (LPVOID*)&pBackBuffer);
		
		ZeroMemory(pBBufferSurfaceDesc_, sizeof(DXGI_SURFACE_DESC));
		if (SUCCEEDED(hr))
		{
			D3D11_TEXTURE2D_DESC TexDesc;
			pBackBuffer->GetDesc(&TexDesc);
			pBBufferSurfaceDesc_->Width = (UINT)TexDesc.Width;
			pBBufferSurfaceDesc_->Height = (UINT)TexDesc.Height;
			pBBufferSurfaceDesc_->Format = TexDesc.Format;
			pBBufferSurfaceDesc_->SampleDesc = TexDesc.SampleDesc;
			SAFE_RELEASE(pBackBuffer);
		}
	}
}


bool D3DApp::LoadContent()
{
	//可以进行重载来丰富相关功能   
	return true;
}



LRESULT CALLBACK D3DApp::WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInstance_, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hwnd);
			break;
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_ACTIVATE:
		//LOWORD(wParam): WA_ACTIVE, WA_CLICKACTIVE, WA_INACTIVE
		//HIWORD(wParam): isMinimezed(bool)
		//lParam: HWND, window being activated
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_bPaused = true;
			m_timer.Stop();
		}
		else
		{
			m_bPaused = false;
			m_timer.Start();
		}
		return 0;
	case WM_SIZE:
		return 0;
		//Begin to resize
	case WM_ENTERSIZEMOVE:
		m_bPaused = true;
		//m_isResizing = true;
		m_timer.Stop();
		return 0;

		//Finished resizing
	case WM_EXITSIZEMOVE:
		return 0;

		//Mouse button down
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		this->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		//Mouse button released
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		//Mouse moving
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

		//Set the minimum window size
	case WM_GETMINMAXINFO:
		/*((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 150;*/
		return 0;

		//Avoid the 'bip' when "Alt+Enter"
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_DESTROY:
		//Exit
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void D3DApp::CalculateFPS()
{
	/*static float begin = m_timer.TotalTime();
	static int frameCounter = 0;
	if (m_timer.TotalTime() - begin >= 1.f)
	{
		std::wostringstream text;
		text << L"      FPS: " << frameCounter << L"    FrameTime: " << 1000.f / frameCounter << L"ms";
		SetWindowTitle(m_winTitle + text.str());

		begin = m_timer.TotalTime();
		frameCounter = 0;
	}
	++frameCounter;*/
}


// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
