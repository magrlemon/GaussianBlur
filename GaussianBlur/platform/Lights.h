#ifndef _LIGHTS_H_
#define _LIGHTS_H_

#include <Windows.h>
#include <xnamath.h>

namespace Lights
{
	//These types of lights are defined exactly the same with those in HLSL
	//Directional Light
	struct DirLight
	{
		XMFLOAT4	ambient;
		XMFLOAT4	diffuse;
		XMFLOAT4	specular;

		XMFLOAT3	dir;
		float		unused;			//Not used, only for '4D Vector' alignment
	};

	//Point light
	struct PointLight
	{
		XMFLOAT4	ambient;
		XMFLOAT4	diffuse;
		XMFLOAT4	specular;

		XMFLOAT3	pos;
		float		range;

		XMFLOAT3	att;			//Three factors of attenuation
		float		unused;			//Not used, only for '4D Vector' alignment
	};

	//Spot light
	struct SpotLight
	{
		XMFLOAT4	ambient;
		XMFLOAT4	diffuse;
		XMFLOAT4	specular;

		XMFLOAT3	dir;		//Direction of spot light
		float		range;

		XMFLOAT3	pos;
		float		spot;			//Spot factor

		XMFLOAT3	att;			//Three factors of attenuation
		float		theta;			//Max divergence angle
	};

	//Material
	struct Material
	{
		XMFLOAT4	ambient;
		XMFLOAT4	diffuse;
		XMFLOAT4	specular;		//Not used, only for '4D Vector' alignment
		XMFLOAT4	reflection;
	};
};

#endif