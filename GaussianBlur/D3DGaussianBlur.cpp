#include "stdafx.h"
#include "D3DGaussianBlur.h"
#include "Common.h"
#include "AppUtil.h"

UINT	g_NumApproxPasses = 3;
UINT	g_MaxApproxPasses = 8;
float	g_FilterRadius = 0;
UINT	g_ThreadsPerGroup = 128;

#define SHADER_NOT_READY		0
#define SHADER_RECOMPILE		1
#define SHADER_READY			2
UINT	g_ShaderCompileStatus = SHADER_NOT_READY;


float CalculateBoxFilterWidth(float radius, int pass)
{
	// Calculate standard deviation according to cutoff width

	// We use sigma*3 as the width of filtering window
	float sigma = radius / 3.0f;

	// The width of the repeating box filter
	float box_width = sqrt(sigma * sigma * 12.0f / pass + 1);

	return box_width;
}


CD3DGaussianBlur::CD3DGaussianBlur()
	: m_pSkyBox(NULL), m_pTex_Scene(NULL)
	, m_pRTV_Scene(NULL), m_pSRV_Scene(NULL)
	, m_pTex_Output(NULL), m_pUAV_Output(NULL)
	, m_pSRV_Output(NULL), m_pRTV_Default(NULL)
	, m_pDSV_Default(NULL), m_pFX_GaussianCol(NULL)
	, m_pFX_GaussianRow(NULL), m_pFX_Render(NULL)
	, m_pSRV_Background(NULL), m_bColorBlur(true)
{
	m_d3dCamera.SetPosition(1.0f, -1.0f, -6.0f);
}


CD3DGaussianBlur::~CD3DGaussianBlur()
{
	if (m_pSkyBox)delete m_pSkyBox;
}

bool CD3DGaussianBlur::LoadContent()
{
	// Load background cubemap
	HRESULT hr;
	WCHAR strPath[MAX_PATH];
	V_RETURN(FindDXSDKMediaFileCch(strPath, MAX_PATH, L"..\\Media\\GaussianBlur\\snowcube1024.dds"));

	ID3D11Texture2D* pCubeTexture = NULL;
	ID3D11ShaderResourceView* pCubeRV = NULL;
	UINT SupportCaps = 0;

	d3dDevice_->CheckFormatSupport(DXGI_FORMAT_R16G16B16A16_FLOAT, &SupportCaps);
	if (SupportCaps & D3D11_FORMAT_SUPPORT_TEXTURECUBE &&
		SupportCaps & D3D11_FORMAT_SUPPORT_RENDER_TARGET &&
		SupportCaps & D3D11_FORMAT_SUPPORT_TEXTURE2D)
	{
		D3DX11_IMAGE_LOAD_INFO LoadInfo;
		LoadInfo.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		V_RETURN(D3DX11CreateShaderResourceViewFromFile(d3dDevice_, strPath, &LoadInfo, NULL, &pCubeRV, NULL));
		pCubeRV->GetResource((ID3D11Resource**)&pCubeTexture);

		m_pSkyBox = new CSkyBox;
		if (m_pSkyBox)
			V_RETURN(m_pSkyBox->OnD3D11CreateDevice(d3dDevice_, 50, pCubeTexture, pCubeRV));
	}
	else
		return false;

	// Request to recompile the shaders
	g_ShaderCompileStatus = SHADER_NOT_READY;
	// Load effect
	V_RETURN(LoadEffectFromFile(d3dDevice_, TEXT("gaussian_vs_ps.fx"), &m_pFX_Render));

	// Layout for full screen quad rendering
	const D3D11_INPUT_ELEMENT_DESC quad_layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	D3DX11_PASS_DESC pass_desc;
	m_pFX_Render->GetTechniqueByName("Tech_ShowImage")->GetPassByName("Pass_ShowMonoImage")->GetDesc(&pass_desc);
	V_RETURN(d3dDevice_->CreateInputLayout(quad_layout, 2, pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, &m_pQuadLayout));

	// Vertex buffer for full screen quad rendering
	SCREEN_VERTEX svQuad[4];
	svQuad[0].pos = D3DXVECTOR4(-1.0f, 1.0f, 0.5f, 1.0f);
	svQuad[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	svQuad[1].pos = D3DXVECTOR4(1.0f, 1.0f, 0.5f, 1.0f);
	svQuad[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	svQuad[2].pos = D3DXVECTOR4(-1.0f, -1.0f, 0.5f, 1.0f);
	svQuad[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	svQuad[3].pos = D3DXVECTOR4(1.0f, -1.0f, 0.5f, 1.0f);
	svQuad[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbdesc =
	{
		4 * sizeof(SCREEN_VERTEX),
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = svQuad;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	V_RETURN(d3dDevice_->CreateBuffer(&vbdesc, &InitData, &m_pScreenQuadVB));

	OnD3D11ResizedSwapChain(d3dDevice_, swapChain_, pBBufferSurfaceDesc_, NULL);
	return true;
}

void CD3DGaussianBlur::UnLoadContent()
{
	SAFE_RELEASE(m_pTex_Scene);
	SAFE_RELEASE(m_pRTV_Scene);
	SAFE_RELEASE(m_pSRV_Scene);
	SAFE_RELEASE(m_pTex_Output);
	SAFE_RELEASE(m_pUAV_Output);
	SAFE_RELEASE(m_pSRV_Output);

	// The compute shader for Gaussian filtering is re-compiled every time the
	// size of input image changed.
	SAFE_RELEASE(m_pFX_GaussianCol);
	SAFE_RELEASE(m_pFX_GaussianRow);

	SAFE_RELEASE(m_pFX_Render);
	SAFE_RELEASE(m_pSRV_Background);
	SAFE_RELEASE(m_pScreenQuadVB);
	SAFE_RELEASE(m_pQuadLayout);
	SAFE_RELEASE(m_pRTV_Default);
	SAFE_RELEASE(m_pDSV_Default);

	m_pSkyBox->OnD3D11DestroyDevice();
}

void CD3DGaussianBlur::Update(float dt)
{
	HRESULT hr = S_OK;
	dt = 0.018f;
	// Compile the shaders if haven't.
	if (g_ShaderCompileStatus == SHADER_RECOMPILE)
	{
		SAFE_RELEASE(m_pFX_GaussianCol);
		SAFE_RELEASE(m_pFX_GaussianRow);

		V(CompileGaussianFilterEffects( d3dDevice_, pBBufferSurfaceDesc_));
		g_ShaderCompileStatus = SHADER_READY;
	}

	if (KeyDown('A'))
	{
		m_d3dCamera.Strafe(-6.f*dt);
	}
	else if (KeyDown('D'))
	{
		m_d3dCamera.Strafe(6.f*dt);
	}
	if (KeyDown('W'))
	{
		m_d3dCamera.Walk(6.f*dt);
	}
	else if (KeyDown('S'))
	{
		m_d3dCamera.Walk(-6.f*dt);
	}
	else if (KeyDown(VK_CONTROL))
	{
		g_FilterRadius = g_FilterRadius + 0.25f;
	}
	else if (KeyDown(VK_MENU))
	{
		g_FilterRadius = g_FilterRadius >= 0.25f ? g_FilterRadius - 0.25f :0;
	}
	else if (KeyDown('0'))
	{
		g_FilterRadius = 0;
	}

	static float angle1(0.f), angle2(0.f);
	angle1 += XM_PI*0.5f*dt;
	angle2 += XM_PI*0.5f*dt;
	if (angle1 > XM_PI*2.f)
		angle1 = 0;
	if (angle2 > XM_PI*2.f)
		angle2 = 0;


	m_d3dCamera.UpdateView();
}

HRESULT  CD3DGaussianBlur::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	//HRESULT hr;

	// Background cubemap
	if(m_pSkyBox)
		m_pSkyBox->OnD3D11ResizedSwapChain(pBackBufferSurfaceDesc);

	// Create render targets, shader resource views and unordered access views
	CreateResources(pd3dDevice, pBackBufferSurfaceDesc);

	// Request to recompile the shaders
	g_ShaderCompileStatus = SHADER_NOT_READY;

	// Save render target and viewport
	ID3D11DeviceContext* pd3dImmediateContext = NULL;
	pd3dDevice->GetImmediateContext(&pd3dImmediateContext);
	pd3dImmediateContext->OMGetRenderTargets(1, &m_pRTV_Default, &m_pDSV_Default);
	UINT nViewPorts = 1;
	pd3dImmediateContext->RSGetViewports(&nViewPorts, m_VP_Default);
	SAFE_RELEASE(pd3dImmediateContext);

	return S_OK;
}

void CD3DGaussianBlur::Render()
{
	if (d3dContext_ == 0)
		return;
	float clearColor[4] = {0.f, 1.f, 0.f, 1.6f}; 
	d3dContext_->ClearRenderTargetView(m_pRTV_Default, clearColor);
	d3dContext_->ClearDepthStencilView(m_pDSV_Default, D3D11_CLEAR_DEPTH, 1.0, 0);

	// Request to recompile the shader if it hasn't been compiled.
	if (g_ShaderCompileStatus == SHADER_NOT_READY)
	{
		g_ShaderCompileStatus = SHADER_RECOMPILE;
		return;
	}
	// Set FP16x4 texture as the output RT
	ID3D11RenderTargetView* rtv_array[1];
	rtv_array[0] = m_pRTV_Scene;
	//将m_pRTV_Scene视图绑定到管线输出合并阶段，使这些资源成为管线的渲染目标和深度 / 模板缓冲区。
	d3dContext_->OMSetRenderTargets(1, rtv_array, m_pDSV_Default);
	d3dContext_->ClearRenderTargetView(m_pRTV_Scene, clearColor);


	// Get the projection & view matrix from the camera class


	XMMATRIX matWorld = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	mWorldViewProjection = XMMatrixMultiply(matWorld, m_d3dCamera.ViewProjection());
	// Render background cubemap
	if( m_pSkyBox )
		m_pSkyBox->D3D11Render( &mWorldViewProjection, m_bColorBlur, d3dContext_ );

	// Resore default render target so that g_pRTV_Downscale can be unbound.
	rtv_array[0] = m_pRTV_Default;
	d3dContext_->OMSetRenderTargets(1, rtv_array, m_pDSV_Default);

	// Perform Gaussian filtering with repeated box filters
	ApplyGaussianFilter(d3dContext_);

	// Display the filtering result
	ShowImage(d3dContext_);

	// The D3D states must be restored at the end of frame. Otherwise the runtime
	// will complain unreleased resource due to D3DX11Effect.
	RestoreDefaultStates(d3dContext_);
	// Show the frame on the primary surface.
	swapChain_->Present(0, 0);
}  

void CD3DGaussianBlur::RestoreDefaultStates(ID3D11DeviceContext* pd3dImmediateContext)
{
	pd3dImmediateContext->ClearState();

	// Render target
	ID3D11RenderTargetView* rtv_array[1] = { m_pRTV_Default };
	pd3dImmediateContext->OMSetRenderTargets(1, rtv_array, m_pDSV_Default);

	// Viewport
	pd3dImmediateContext->RSSetViewports(1, m_VP_Default);
}

void CD3DGaussianBlur::ApplyGaussianFilter(ID3D11DeviceContext* pd3dImmediateContext)
{
	D3D11_TEXTURE2D_DESC tex_desc;
	m_pTex_Scene->GetDesc(&tex_desc);

	float box_width = CalculateBoxFilterWidth(g_FilterRadius, g_NumApproxPasses);
	float half_box_width = box_width * 0.5f;
	float frac_half_box_width = (half_box_width + 0.5f) - (int)(half_box_width + 0.5f);
	float inv_frac_half_box_width = 1.0f - frac_half_box_width;
	float rcp_box_width = 1.0f / box_width;

	// Step 1. Vertical passes: Each thread group handles a colomn in the image
	//在得到Effect框架接口后，我们就要绑定HLSL和Directx中的相关变量
	ID3DX11EffectTechnique* pTech = m_pFX_GaussianCol->GetTechniqueByName("Tech_GaussianFilter");

	// Input texture
	m_pFX_GaussianCol->GetVariableByName("g_texInput")->AsShaderResource()->SetResource(m_pSRV_Scene);
	// Output texture
	m_pFX_GaussianCol->GetVariableByName("g_rwtOutput")->AsUnorderedAccessView()->SetUnorderedAccessView(m_pUAV_Output);
	m_pFX_GaussianCol->GetVariableByName("g_NumApproxPasses")->AsScalar()->SetInt(g_NumApproxPasses - 1);
	m_pFX_GaussianCol->GetVariableByName("g_HalfBoxFilterWidth")->AsScalar()->SetFloat(half_box_width);
	m_pFX_GaussianCol->GetVariableByName("g_FracHalfBoxFilterWidth")->AsScalar()->SetFloat(frac_half_box_width);
	m_pFX_GaussianCol->GetVariableByName("g_InvFracHalfBoxFilterWidth")->AsScalar()->SetFloat(inv_frac_half_box_width);
	m_pFX_GaussianCol->GetVariableByName("g_RcpBoxFilterWidth")->AsScalar()->SetFloat(rcp_box_width);

	// Select pass
	ID3DX11EffectPass* pPass = m_bColorBlur ? pTech->GetPassByName("Pass_GaussianColor") : pTech->GetPassByName("Pass_GaussianMono");
	//通过Apply调用相应pass的shader状态
	pPass->Apply(0, pd3dImmediateContext);
	pd3dImmediateContext->Dispatch(tex_desc.Width, 1, 1);

	// Unbound CS resource and output
	ID3D11ShaderResourceView* srv_array[] = { NULL, NULL, NULL, NULL };
	pd3dImmediateContext->CSSetShaderResources(0, 4, srv_array);
	ID3D11UnorderedAccessView* uav_array[] = { NULL, NULL, NULL, NULL };
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 4, uav_array, NULL);

	// Step 2. Horizontal passes: Each thread group handles a row in the image
	pTech = m_pFX_GaussianRow->GetTechniqueByName("Tech_GaussianFilter");
	// Input texture
	m_pFX_GaussianRow->GetVariableByName("g_texInput")->AsShaderResource()->SetResource(m_pSRV_Scene);
	// Output texture
	m_pFX_GaussianRow->GetVariableByName("g_rwtOutput")->AsUnorderedAccessView()->SetUnorderedAccessView(m_pUAV_Output);
	m_pFX_GaussianRow->GetVariableByName("g_NumApproxPasses")->AsScalar()->SetInt(g_NumApproxPasses - 1);
	m_pFX_GaussianRow->GetVariableByName("g_HalfBoxFilterWidth")->AsScalar()->SetFloat(half_box_width);
	m_pFX_GaussianRow->GetVariableByName("g_FracHalfBoxFilterWidth")->AsScalar()->SetFloat(frac_half_box_width);
	m_pFX_GaussianRow->GetVariableByName("g_InvFracHalfBoxFilterWidth")->AsScalar()->SetFloat(inv_frac_half_box_width);
	m_pFX_GaussianRow->GetVariableByName("g_RcpBoxFilterWidth")->AsScalar()->SetFloat(rcp_box_width);

	// Select pass
	pPass = m_bColorBlur ? pTech->GetPassByName("Pass_GaussianColor") : pTech->GetPassByName("Pass_GaussianMono");
	//通过Apply调用相应pass的shader状态
	pPass->Apply(0, pd3dImmediateContext);
	pd3dImmediateContext->Dispatch(tex_desc.Height, 1, 1);

	// Unbound CS resource and output
	pd3dImmediateContext->CSSetShaderResources(0, 4, srv_array);
	pd3dImmediateContext->CSSetUnorderedAccessViews(0, 4, uav_array, NULL);
}


void CD3DGaussianBlur::ShowImage(ID3D11DeviceContext* pd3dImmediateContext)
{
	// Output to default render target
	ID3D11RenderTargetView* rtv_array[1] = { m_pRTV_Default };
	pd3dImmediateContext->OMSetRenderTargets(1, rtv_array, m_pDSV_Default);
	pd3dImmediateContext->RSSetViewports(1, m_VP_Default);

	ID3DX11EffectTechnique* pTech = m_pFX_Render->GetTechniqueByName("Tech_ShowImage");
	ID3DX11EffectPass* pPass = m_bColorBlur ? pTech->GetPassByName("Pass_ShowColorImage") : pTech->GetPassByName("Pass_ShowMonoImage");

	if (m_bColorBlur)
		m_pFX_Render->GetVariableByName("g_texColorInput")->AsShaderResource()->SetResource(m_pSRV_Output);
	else
		m_pFX_Render->GetVariableByName("g_texMonoInput")->AsShaderResource()->SetResource(m_pSRV_Output);

	UINT strides = sizeof(SCREEN_VERTEX);
	UINT offsets = 0;
	ID3D11Buffer* pBuffers[1] = { m_pScreenQuadVB };

	pd3dImmediateContext->IASetInputLayout(m_pQuadLayout);
	pd3dImmediateContext->IASetVertexBuffers(0, 1, pBuffers, &strides, &offsets);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//通过Apply调用相应pass的shader状态
	pPass->Apply(0, pd3dImmediateContext);
	pd3dImmediateContext->Draw(4, 0);
}


HRESULT CD3DGaussianBlur::CompileGaussianFilterEffects(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;

	// The compute shaders for column and row filtering are created from the same
	// effect file, separated by different macro definitions.

	// Find the file
	WCHAR str[MAX_PATH];
	if (m_bColorBlur)
	{
		V_RETURN(FindDXSDKMediaFileCch(str, MAX_PATH, TEXT("gaussian_color_cs.fx")));
	}
	else
	{
		V_RETURN(FindDXSDKMediaFileCch(str, MAX_PATH, TEXT("gaussian_mono_cs.fx")));
	}

	D3D10_SHADER_MACRO defines[8];

	// Software emulation mode requires different shader code
	char str_ref_mode[8];
	sprintf_s(str_ref_mode, "%u",  0);
	defines[0].Name = "USE_D3D_REF_MODE";
	defines[0].Definition = str_ref_mode;

	// Input image height
	UINT num_rows = pBackBufferSurfaceDesc->Height;
	char str_num_rows[8];
	sprintf_s(str_num_rows, "%u", num_rows);
	defines[1].Name = "NUM_IMAGE_ROWS";
	defines[1].Definition = str_num_rows;

	// Input image width
	UINT num_cols = pBackBufferSurfaceDesc->Width;
	char str_num_cols[8];
	sprintf_s(str_num_cols, "%u", num_cols);
	defines[2].Name = "NUM_IMAGE_COLS";
	defines[2].Definition = str_num_cols;


	// ------------------------ Column filtering shader -----------------------

	// Row scan or column scan
	defines[3].Name = "SCAN_COL_PASS";
	defines[3].Definition = "1";

	// Allocate shared memory according to the size of input image
	char str_data_length[8];
	sprintf_s(str_data_length, "%u", max(pBackBufferSurfaceDesc->Height, g_ThreadsPerGroup * 2));
	defines[4].Name = "SCAN_SMEM_SIZE";
	defines[4].Definition = str_data_length;

	// Number of texels per thread handling
	UINT texels_per_thread = (pBackBufferSurfaceDesc->Height + g_ThreadsPerGroup - 1) / g_ThreadsPerGroup;
	char str_texels_per_thread[8];
	sprintf_s(str_texels_per_thread, "%u", texels_per_thread);
	defines[5].Name = "TEXELS_PER_THREAD";
	defines[5].Definition = str_texels_per_thread;

	char str_threads_per_group[8];
	sprintf_s(str_threads_per_group, "%u", g_ThreadsPerGroup);
	defines[6].Name = "THREADS_PER_GROUP";
	defines[6].Definition = str_threads_per_group;

	defines[7].Name = NULL;
	defines[7].Definition = NULL;

	// Compile the effect file
	//在Direct11中先用D3DX11CompileFromFile()编译HLSL代码，
	//再用D3DX11CreateEffectFromMemory生成Effect的接口。
	//
	ID3DBlob* pBlobFX = NULL;
	ID3DBlob* pErrorBlob = NULL;
	hr = D3DX11CompileFromFile(str, defines, NULL, NULL, "fx_5_0", NULL, NULL, NULL, &pBlobFX, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		SAFE_RELEASE(pErrorBlob);
		return hr;
	}

	// Create the effect for column Gaussian filtering
	V_RETURN(D3DX11CreateEffectFromMemory(pBlobFX->GetBufferPointer(), pBlobFX->GetBufferSize(), 0, pd3dDevice, &m_pFX_GaussianCol));

	SAFE_RELEASE(pBlobFX);
	SAFE_RELEASE(pErrorBlob);

	// ------------------------- Row filtering shader -------------------------

	defines[3].Name = "SCAN_COL_PASS";
	defines[3].Definition = "0";

	// Allocate shared memory according to the size of input image
	sprintf_s(str_data_length, "%u", max(pBackBufferSurfaceDesc->Width, g_ThreadsPerGroup * 2));
	defines[4].Name = "SCAN_SMEM_SIZE";
	defines[4].Definition = str_data_length;

	// Number of texels per thread handling
	texels_per_thread = (pBackBufferSurfaceDesc->Width + g_ThreadsPerGroup - 1) / g_ThreadsPerGroup;
	sprintf_s(str_texels_per_thread, "%u", texels_per_thread);
	defines[5].Name = "TEXELS_PER_THREAD";
	defines[5].Definition = str_texels_per_thread;
	//值得注意的是D3DX11CompileFromFile()中的"fx_5_0",在Directx11中一定要用"fx_5_0"
	hr = D3DX11CompileFromFile(str, defines, NULL, NULL, "fx_5_0", NULL, NULL, NULL, &pBlobFX, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		SAFE_RELEASE(pErrorBlob);
		return hr;
	}

	// Create the effect for row Gaussian filtering
	V_RETURN(D3DX11CreateEffectFromMemory(pBlobFX->GetBufferPointer(), pBlobFX->GetBufferSize(), 0, pd3dDevice, &m_pFX_GaussianRow));

	SAFE_RELEASE(pBlobFX);
	SAFE_RELEASE(pErrorBlob);

	return hr;
}


HRESULT CD3DGaussianBlur::LoadEffectFromFile(ID3D11Device* pd3dDevice, WCHAR* szFileName, ID3DX11Effect** ppEffect)
{
	HRESULT hr = S_OK;

	// find the filed
	WCHAR str[MAX_PATH];
	V_RETURN(FindDXSDKMediaFileCch(str, MAX_PATH, szFileName));

	// Compile the effect file
	ID3DBlob* pBlobFX = NULL;
	ID3DBlob* pErrorBlob = NULL;
	hr = D3DX11CompileFromFile(str, NULL, NULL, NULL, "fx_5_0", NULL, NULL, NULL, &pBlobFX, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		SAFE_RELEASE(pErrorBlob);
		return hr;
	}

	// Create the effect
	hr = D3DX11CreateEffectFromMemory(pBlobFX->GetBufferPointer(), pBlobFX->GetBufferSize(), 0, pd3dDevice, ppEffect);
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("Failed to load effect file."));
		return hr;
	}

	SAFE_RELEASE(pBlobFX);
	SAFE_RELEASE(pErrorBlob);

	return S_OK;
}


HRESULT CD3DGaussianBlur::CreateResources(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	//该纹理需要绑定到管线的两个阶段：Render Target和Shader Resource。
	// Render target texture for scene rendering
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;

	ZeroMemory(&tex_desc, sizeof(D3D11_TEXTURE2D_DESC));
	ZeroMemory(&rtv_desc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	ZeroMemory(&srv_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	ZeroMemory(&uav_desc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

	tex_desc.ArraySize = 1; //该纹理数组中包含1张纹理  
	//指定两个绑定阶段：Render Target 和 Shader Resource 
	tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.Format = m_bColorBlur ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R32_FLOAT;
	tex_desc.Width = pBackBufferSurfaceDesc->Width;
	tex_desc.Height = pBackBufferSurfaceDesc->Height;
	tex_desc.MipLevels = 1; 
	tex_desc.SampleDesc.Count = 1;
	
	V_RETURN(pd3dDevice->CreateTexture2D(&tex_desc, NULL, &m_pTex_Scene));
	//需要针对Render Target和Shader Resource分别对其创建相应的视图：
	rtv_desc.Format = tex_desc.Format;
	rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtv_desc.Texture2D.MipSlice = 0;//每个视图只使用最高层的mip链 
	V_RETURN(pd3dDevice->CreateRenderTargetView(m_pTex_Scene, &rtv_desc, &m_pRTV_Scene));
	//然后是Shader Resource视图：
	srv_desc.Format = tex_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = -1;	//-1 表示使用其所有的mip链（有多少使用多少）  
	srv_desc.Texture2D.MostDetailedMip = 0;//指定最精细的mip层，0表示高层 
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_pTex_Scene, &srv_desc, &m_pSRV_Scene));

	// RW texture for output
	tex_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.Format = m_bColorBlur ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	V_RETURN(pd3dDevice->CreateTexture2D(&tex_desc, NULL, &m_pTex_Output));

	uav_desc.Format = tex_desc.Format;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uav_desc.Texture2D.MipSlice = 0;
	V_RETURN(pd3dDevice->CreateUnorderedAccessView(m_pTex_Output, &uav_desc, &m_pUAV_Output));

	srv_desc.Format = tex_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = 1;
	srv_desc.Texture2D.MostDetailedMip = 0;
	V_RETURN(pd3dDevice->CreateShaderResourceView(m_pTex_Output, &srv_desc, &m_pSRV_Output));

	return S_OK;
}


void CD3DGaussianBlur::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_lastPos.x = x;
	m_lastPos.y = y;
	SetCapture(hwnd_);
}

void CD3DGaussianBlur::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CD3DGaussianBlur::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * (x - m_lastPos.x));
		float dy = XMConvertToRadians(0.25f * (y - m_lastPos.y));

		m_d3dCamera.Pitch(dy);
		m_d3dCamera.RotateY(dx);
	}
	m_lastPos.x = x;
	m_lastPos.y = y;
}
