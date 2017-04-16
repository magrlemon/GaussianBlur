#ifndef _APP_UTIL_H_
#define _APP_UTIL_H_

#include <Windows.h>
#include <xnamath.h>
#include <vector>
#include <string>

//Release COM interfaces safely
template<typename T>
inline void SafeRelease(T &t)
{
	if(t)
	{
		t->Release();
		t = NULL;
	}
}

//Delete memories safely
template<typename T>
inline void SafeDelete(T &t)
{
	if(t)
	{
		delete t;
		t = NULL;
	}
}

//Clamp 'value' between 'vMin' and 'vMax'
template<typename T>
inline T Clamp(T vMin, T vMax, T value)
{
	value = max(vMin,value);
	value = min(vMax,value);

	return value;
}

inline int KeyDown(int vKey)
{
	return GetAsyncKeyState(vKey) & 0x8000;
}

bool ReadBinaryFile(std::wstring fileName, std::vector<char> &content);

inline XMMATRIX InverseTranspose(CXMMATRIX m)
{
	XMMATRIX tmp = m;
	tmp.r[3] = XMVectorSet(0.f,0.f,0.f,1.f);
	
	return XMMatrixTranspose(XMMatrixInverse(&XMMatrixDeterminant(tmp),tmp));
}

namespace Colors
{
	//Most frequently used colors
	const XMVECTORF32 White     = {1.0f, 1.0f, 1.0f, 1.0f};
	const XMVECTORF32 Black     = {0.0f, 0.0f, 0.0f, 1.0f};
	const XMVECTORF32 Red       = {1.0f, 0.0f, 0.0f, 1.0f};
	const XMVECTORF32 Green     = {0.0f, 1.0f, 0.0f, 1.0f};
	const XMVECTORF32 Blue      = {0.0f, 0.0f, 1.0f, 1.0f};
	const XMVECTORF32 Yellow    = {1.0f, 1.0f, 0.0f, 1.0f};
	const XMVECTORF32 Cyan      = {0.0f, 1.0f, 1.0f, 1.0f};
	const XMVECTORF32 Magenta   = {1.0f, 0.0f, 1.0f, 1.0f};
	const XMVECTORF32 Silver	= {0.75f,0.75f,0.75f,1.0f};
};


#endif