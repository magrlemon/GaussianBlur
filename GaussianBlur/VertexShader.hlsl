


struct VS_INPUT
{
	float4 POS:POSITION;
	float2 UV:TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 POS:POSITION;
	float2 UV:TEXCOORD0;
};

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}