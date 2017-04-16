#ifndef _INPUTS_H_
#define	_INPUTS_H_

#include <Windows.h>
#include <xnamath.h>
#include <D3D11.h>

//Common vertex formats
namespace Vertex
{
	//Only position
	struct Pos
	{
		XMFLOAT3	pos;
	};
	//Pos + normal
	struct PosNormal
	{
		XMFLOAT3	pos;
		XMFLOAT3	normal;
	};
	//position + normal + texcoord(32 bytes)
	struct Basic32
	{
		XMFLOAT3	pos;
		XMFLOAT3	normal;
		XMFLOAT2	tex;
	};
};
//Input layout descriptions
struct InputLayoutDesc
{
	const static D3D11_INPUT_ELEMENT_DESC Pos[1];

	const static D3D11_INPUT_ELEMENT_DESC PosNormal[2];
	
	const static D3D11_INPUT_ELEMENT_DESC Basic32[3];
};
//All input layout interfaces 
struct InputLayouts
{
	static bool InitAll(ID3D11Device *device);
	static void ReleaseAll();

	static ID3D11InputLayout	*pos;
	static ID3D11InputLayout	*posNormal;
	static ID3D11InputLayout	*basic32;
};

#endif	//_INPUTS_H_