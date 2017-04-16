#include "stdafx.h"
#include "D3DSkyBox.h"

#include "Common.h"
#include "AppUtil.h"

struct SKYBOX_VERTEX
{
	D3DXVECTOR4 pos;
};

const D3D11_INPUT_ELEMENT_DESC g_aVertexLayout[]
{
	{ "POSITION", 0,DXGI_FORMAT_R32G32B32A32_FLOAT , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

CSkyBox::CSkyBox()
{
	m_pEnvironmentMap11 = NULL;
	m_pEnvironmentRV11 = NULL;
	m_pd3dDevice11 = NULL;
	m_pVertexShader = NULL;
	m_pPixelShaderColor = NULL;
	m_pPixelShaderGray = NULL;
	m_pSamplerState = NULL;
	m_pVertexLayout11 = NULL;
	m_pcbVSPerObject = NULL;
	m_pVB11 = NULL;
	m_pDepthStencilState11 = NULL;

	m_fSize = 1.0f;
}


CSkyBox::~CSkyBox()
{

}

void CSkyBox::OnD3D11DestroyDevice()
{
	m_pd3dDevice11 = NULL;
	SAFE_RELEASE(m_pd3dDevice11);
	SAFE_RELEASE(m_pEnvironmentRV11);
	SAFE_RELEASE(m_pEnvironmentMap11);
	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShaderColor);
	SAFE_RELEASE(m_pPixelShaderGray);
	SAFE_RELEASE(m_pVertexLayout11);
	SAFE_RELEASE(m_pcbVSPerObject);
	SAFE_RELEASE(m_pDepthStencilState11);
	SAFE_RELEASE(m_pVB11);
}

HRESULT CSkyBox::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, float fSize,
	ID3D11Texture2D* pCubeTexture, ID3D11ShaderResourceView* pCubeRV)
{
	HRESULT hr;
	m_pd3dDevice11 = pd3dDevice;
	m_pEnvironmentMap11 = pCubeTexture;
	m_pEnvironmentRV11 = pCubeRV;
	m_fSize = fSize;
	
	ID3DBlob*   pBlobVS = NULL;
	ID3DBlob*   pBlobShaderColor = NULL;
	ID3DBlob*	pBlobShaderGray = NULL;

	V_RETURN( CompileShaderFromFile(L"skybox11.hlsl", "SkyboxVS", "vs_4_0", &pBlobVS ) );
	V_RETURN( CompileShaderFromFile(L"skybox11.hlsl", "SkyboxPS_Color", "ps_4_0", &pBlobShaderColor ) );
	V_RETURN( CompileShaderFromFile(L"skybox11.hlsl", "SkyboxPS_Gray", "ps_4_0", &pBlobShaderGray ) );

	V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &m_pVertexShader ) );
	V_RETURN( pd3dDevice->CreatePixelShader( pBlobShaderColor->GetBufferPointer(), pBlobShaderColor->GetBufferSize(), NULL, &m_pPixelShaderColor ) );
	V_RETURN( pd3dDevice->CreatePixelShader( pBlobShaderGray->GetBufferPointer(), pBlobShaderGray->GetBufferSize(), NULL, &m_pPixelShaderGray ) );

	// Create an input layout
	V_RETURN(pd3dDevice->CreateInputLayout(g_aVertexLayout, 1, pBlobVS->GetBufferPointer(),
		pBlobVS->GetBufferSize(), &m_pVertexLayout11));

	SAFE_RELEASE( pBlobVS );
	SAFE_RELEASE( pBlobShaderColor );
	SAFE_RELEASE( pBlobShaderGray );

	// Setup linear sampler
	D3D11_SAMPLER_DESC sampleDesc;
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.MipLODBias = 0.0f;
	sampleDesc.MaxAnisotropy = 1;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampleDesc.BorderColor[0] = sampleDesc.BorderColor[1] = sampleDesc.BorderColor[2] = sampleDesc.BorderColor[3] = 0;
	sampleDesc.MinLOD = 0;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;

	V_RETURN(pd3dDevice->CreateSamplerState( &sampleDesc, &m_pSamplerState ) );

	D3D11_BUFFER_DESC  bufDesc;
	bufDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufDesc.MiscFlags = 0;
	bufDesc.ByteWidth = sizeof(CB_VS_PER_OBJECT);
	V_RETURN(pd3dDevice->CreateBuffer(&bufDesc, NULL, &m_pcbVSPerObject)); 

	D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
	ZeroMemory(&DepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DepthStencilDesc.DepthEnable = FALSE;
	DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DepthStencilDesc.StencilEnable = FALSE;

	V_RETURN(pd3dDevice->CreateDepthStencilState(&DepthStencilDesc, &m_pDepthStencilState11))

	return S_OK;
}

void CSkyBox::OnD3D11ResizedSwapChain(const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	if (m_pd3dDevice11 == NULL)
		return;
	SKYBOX_VERTEX* pVertex = new SKYBOX_VERTEX[4];
	if (!pVertex)
		return;
	float fHeightW = -1.0f -(1.0f / (float)pBackBufferSurfaceDesc->Width);
	float fHeightH = -1.0f -(1.0f / (float)pBackBufferSurfaceDesc->Height);
	float fLowW = 1.0f + ( 1.0f / (float)pBackBufferSurfaceDesc->Width);
	float fLowH = 1.0f + ( 1.0f / (float)pBackBufferSurfaceDesc->Height);

	pVertex[0].pos = D3DXVECTOR4( fLowW, fLowH, 1.0f, 1.0f );
	pVertex[1].pos = D3DXVECTOR4( fLowW, fHeightH, 1.0f, 1.0f );
	pVertex[2].pos = D3DXVECTOR4( fHeightH, fLowH, 1.0f, 1.0f ); 
	pVertex[3].pos = D3DXVECTOR4( fHeightH, fHeightH, 1.0f, 1.0f );

	UINT uiVertBufSize = 4 * sizeof(SKYBOX_VERTEX);
	//vertex Buffer
	D3D11_BUFFER_DESC vbdesc;
	vbdesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbdesc.ByteWidth = uiVertBufSize;
	vbdesc.MiscFlags = 0;
	vbdesc.CPUAccessFlags = 0;
	vbdesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pVertex;
	V( m_pd3dDevice11->CreateBuffer( &vbdesc, &InitData, &m_pVB11 ) );
	SAFE_DELETE_ARRAY(pVertex);
}

void CSkyBox::D3D11Render(XMMATRIX* pmWorldViewProj, BOOL bColor,
	ID3D11DeviceContext* pd3dImmediateContext)
{
	HRESULT hr;
	pd3dImmediateContext->IASetInputLayout( m_pVertexLayout11 );
	 
	UINT uStrides = sizeof( SKYBOX_VERTEX );
	UINT uOffsets = 0;
	ID3D11Buffer* pBuffers[1] = { m_pVB11 };

	pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &uStrides, &uOffsets );
	pd3dImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pd3dImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	pd3dImmediateContext->PSSetShader(bColor ? m_pPixelShaderColor : m_pPixelShaderGray, NULL, 0);
	//获取subresource中数据的指针, 同时拒绝GPU读取
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	V(pd3dImmediateContext->Map(m_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	CB_VS_PER_OBJECT* pVSPerObject = (CB_VS_PER_OBJECT*)MappedResource.pData;
	pVSPerObject->m_WorldViewProj= InverseTranspose(*pmWorldViewProj);
	//D3DXMatrixInverse(&pVSPerObject->m_WorldViewProj, NULL, pmWorldViewProj);
	pd3dImmediateContext->Unmap(m_pcbVSPerObject, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pcbVSPerObject);

	pd3dImmediateContext->PSSetSamplers(0, 1, &m_pSamplerState);
	//着色器的资源的数组绑定到像素着色器级
	pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pEnvironmentRV11);

	ID3D11DepthStencilState* pDepthStencilStateStored11 = NULL;
	UINT StencilRef;
	pd3dImmediateContext->OMGetDepthStencilState(&pDepthStencilStateStored11, &StencilRef);
	pd3dImmediateContext->OMSetDepthStencilState(m_pDepthStencilState11, 0);

	pd3dImmediateContext->Draw( 4, 0 );

	pd3dImmediateContext->OMSetDepthStencilState(pDepthStencilStateStored11, StencilRef);
}

HRESULT CSkyBox::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	// find the file
	WCHAR str[MAX_PATH];
	V_RETURN(FindDXSDKMediaFileCch(str, MAX_PATH, szFileName));

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(str, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		SAFE_RELEASE(pErrorBlob);
		return hr;
	}
	SAFE_RELEASE(pErrorBlob);

	return S_OK;
}
 