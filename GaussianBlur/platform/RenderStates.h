#ifndef _RENDER_STATES_H_
#define _RENDER_STATES_H_

#include <D3D11.h>

//Here we define the most frequently used render states
class RenderStates
{
public:
	//Initialize all the states
	static bool InitAll(ID3D11Device* device);
	static void ReleaseAll();

	static ID3D11RasterizerState	*WireFrameRS;			//Wire frame mode
	static ID3D11RasterizerState	*NoCullRS;				//No back face culling
	static ID3D11RasterizerState	*CounterClockFrontRS;	//Set the counter-clock wise as front face
	
	static ID3D11BlendState			*TransparentBS;			//Transparent blending state
	static ID3D11BlendState			*NoColorWriteBS;		//Forbid color writing

	static ID3D11DepthStencilState	*MarkMirrorDSS;			//Mark the mirror area(Used to draw mirrors)
	static ID3D11DepthStencilState	*DrawReflectionDSS;		//Draw reflected objects in the mirror(Used to draw mirrors)
	static ID3D11DepthStencilState	*NoDoubleBlendingDSS;	//Avoid double-blending(Used to draw planar-shadows)
};

#endif