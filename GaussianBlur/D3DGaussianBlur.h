#ifndef _D3D_UTILITY_H_  
#define _D3D_UTILITY_H_ 
#include "platform.h"
#include "D3DApp.h"
#include "D3DSkyBox.h"

#include "d3dx11effect.h"



struct SCREEN_VERTEX
{
	D3DXVECTOR4 pos;
	D3DXVECTOR2 tex;
};

class CD3DGaussianBlur : public D3DApp
{
public:
	CD3DGaussianBlur();
	virtual ~CD3DGaussianBlur();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
public:
	////载入具体的Demo自己的内容，如纹理，几何体，着色器
	virtual bool			LoadContent();
	//释放具体Demo中的内容
	virtual void			UnLoadContent();

	virtual void			Update(float dt);

	virtual void			Render();

	HRESULT  OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);

protected:

	void ShowImage(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT CompileGaussianFilterEffects(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);

	void ApplyGaussianFilter(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT LoadEffectFromFile(ID3D11Device* pd3dDevice, WCHAR* szFileName, ID3DX11Effect** ppEffect);

	HRESULT CreateResources(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);

	void RestoreDefaultStates(ID3D11DeviceContext* pd3dImmediateContext);

private:
	CSkyBox*							m_pSkyBox;

	// D3DX11 Effects
	ID3DX11Effect*				m_pFX_GaussianCol ;
	ID3DX11Effect*				m_pFX_GaussianRow ;
	ID3DX11Effect*				m_pFX_Render ;

	ID3D11Buffer*               m_pScreenQuadVB ;
	ID3D11InputLayout*          m_pQuadLayout ;

	// RW texture as intermediate and output buffer
	ID3D11Texture2D*            m_pTex_Output ;
	ID3D11UnorderedAccessView*  m_pUAV_Output ;
	ID3D11ShaderResourceView*   m_pSRV_Output ;

	// Default render targets in the swap chain
	ID3D11RenderTargetView*		m_pRTV_Default ;
	ID3D11DepthStencilView*		m_pDSV_Default ;
	D3D11_VIEWPORT				m_VP_Default[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];

	// Render target texture for scene
	ID3D11Texture2D*            m_pTex_Scene = NULL;
	ID3D11RenderTargetView*     m_pRTV_Scene = NULL;
	ID3D11ShaderResourceView*   m_pSRV_Scene = NULL;

	ID3D11ShaderResourceView*	m_pSRV_Background;



private:
	bool						m_bColorBlur;
	POINT						m_lastPos;
};

#endif