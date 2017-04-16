#ifndef _SKY_BOX_H_  
#define _SKY_BOX_H_ 


#include "platform.h"
class CSkyBox
{
public:
	CSkyBox();
	virtual ~CSkyBox();
public:

	HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, float fSize,
		ID3D11Texture2D* pCubeTexture, ID3D11ShaderResourceView* pCubeRV);

	void OnD3D11ResizedSwapChain(const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);

	void OnD3D11DestroyDevice();

	void D3D11Render(XMMATRIX* pmWorldViewProj, BOOL bColor, ID3D11DeviceContext* pd3dImmediateContext);

	ID3D11Texture2D* GetD3D10EnvironmentMap() {
		return m_pEnvironmentMap11;
	}
	ID3D11ShaderResourceView* GetD3D10EnvironmentMapRV()
	{
		return m_pEnvironmentRV11;
	}
	void SetD3D10EnviromentMap11( ID3D11Texture2D* pEnvironment ) {
		m_pEnvironmentMap11 = pEnvironment;
	};

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

protected:

	//-----------------------------------------------------------------------------
	//要用到World矩阵变量 这在fx文件里定义成Constant buffer类型。。。
	//那在cpp文件里也要相应建立Constant Buffer;所以
	//-----------------------------------------------------------------------------
	struct CB_VS_PER_OBJECT
	{
		XMMATRIX m_WorldViewProj;
	};
private:
	ID3D11Texture2D*			m_pEnvironmentMap11;

	ID3D11ShaderResourceView*   m_pEnvironmentRV11;

	ID3D11Device*				m_pd3dDevice11;

	ID3D11VertexShader*			m_pVertexShader;

	ID3D11PixelShader*			m_pPixelShaderColor;

	ID3D11PixelShader*			m_pPixelShaderGray;

	ID3D11SamplerState*			m_pSamplerState;

	ID3D11InputLayout*			m_pVertexLayout11;

	ID3D11DepthStencilState*	m_pDepthStencilState11;

	//constant buffer
	ID3D11Buffer*				m_pcbVSPerObject;

	ID3D11Buffer*				m_pVB11;

	float m_fSize;
};

#endif
