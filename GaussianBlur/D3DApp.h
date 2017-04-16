#ifndef _D3DX11_BASE_H_
#define _D3DX11_BASE_H_


#include "platform.h"
#include "Timer.h"
#include "D3DCamera.h"

class D3DApp
{

public:
	D3DApp(std::wstring title = L"¸ßË¹Ä£ºý");
	virtual ~D3DApp();
	//Basic inline membre functions
	HINSTANCE	AppInstance()	const { return hInstance_; }
	HWND		Window()		const { return hwnd_; }
	void		SetWindowTitle(std::wstring title) { SetWindowText(hwnd_, title.c_str()); }
	//Mouse control function
	//By default, the three functions do nothing. And can be redefined.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	virtual	 bool		LoadContent();

	virtual  void		UnLoadContent() = 0;

	virtual  void		Update(float dt) = 0;

	virtual  void		Render() = 0;
public:

	bool				Initialize( HINSTANCE hInstance );
	
	int					Run();
	
	void				Shutdown();

	

	//virtual  bool	    OnResize() {};						//Window size changed

	virtual LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);		//Main messaeg processing function

protected:
	bool				InitWindow();		//Win32 window initialization

	bool				InitD3D();			//D3D11 initialization

	void				CalculateFPS();		//Frame rate update

	void				UpdateBackBufferDesc();

protected:

	HINSTANCE					hInstance_;

	HWND						hwnd_;


	D3D_DRIVER_TYPE				driverType_;

	D3D_FEATURE_LEVEL			featureLevel_;

	ID3D11Device*				d3dDevice_;

	ID3D11DeviceContext*		d3dContext_;

	IDXGISwapChain*				swapChain_;

	ID3D11RenderTargetView*		backBufferTarget_;

	ID3D11DepthStencilView*		depthStencilView_;

	ID3D11Texture2D*			pDepthStencil_ ;

	XMMATRIX					mWorldViewProjection;

	DXGI_SURFACE_DESC*			pBBufferSurfaceDesc_;

	Camera						m_d3dCamera;

private:
	std::wstring	m_winTitle;			//Title of the application

	unsigned int	m_clientWidth;		//Client window size
	unsigned int	m_clientHeight;

	Timer			m_timer;			//Timer

	bool			m_bPaused;
private:
	//Forbid copying
	D3DApp(const D3DApp&);
	D3DApp& operator = (const D3DApp&);

};

#endif // !_D3DX11_BASE_H_