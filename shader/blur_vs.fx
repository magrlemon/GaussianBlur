//// Copyright (c) 2011 NVIDIA Corporation. All rights reserved.
////
//// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
//// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
//// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, NONINFRINGEMENT,IMPLIED WARRANTIES OF
//// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA 
//// OR ITS SUPPLIERS BE  LIABLE  FOR  ANY  DIRECT, SPECIAL,  INCIDENTAL,  INDIRECT,  OR  
//// CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS 
//// OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY 
//// OTHER PECUNIARY LOSS) ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, 
//// EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
////
//// Please direct any bugs or questions to SDKFeedback@nvidia.com
//
////-----------------------------------------------------------------------------
//// Macros and constant values
////-----------------------------------------------------------------------------
//
////-----------------------------------------------------------------------------
//// Shader constant buffers
////-----------------------------------------------------------------------------
//cbuffer cbPerObject : register(b0)
//{
//	//row_major matrix g_mWorldProjView £º packoffect(c0);
//}
////-----------------------------------------------------------------------------
//// Buffers, textures & samplers
////-----------------------------------------------------------------------------
//
//// FP32 texture containing the filtered image
//Texture2D<float>  BlurTexture;
//
////-----------------------------------------------------------------------------
//// WorldProjectView
////-----------------------------------------------------------------------------
//
//
////-----------------------------------------------------------------------------
//// Name: cbArrayGaussianFilter                                        
//// Type: ¸ßË¹¹ýÂË¾ØÕó
////-----------------------------------------------------------------------------
//cbuffer cbArrayGaussianFilter : register(b1)
//{
//	//row_major matrix g_cbArrayGaussianFilter £º packoffect(c1);
//}
//
////-----------------------------------------------------------------------------
//// Name: g_blursampler
//// Type: SamplerState                                      
//// Desc: 
////-----------------------------------------------------------------------------
//SamplerState g_blursampler : register(s0);
//
//
////-----------------------------------------------------------------------------
//// Name: VS_INPUT                                        
//// Type: Vertex shader
////-----------------------------------------------------------------------------
//struct VS_INPUT
//{
//	float POS : POSITION;
//	float UV : TEXCOORD0;
//};
//
////-----------------------------------------------------------------------------
//// Name: VS_OUTPUT                                        
//// Type: Vertex shader
////-----------------------------------------------------------------------------
//struct VS_OUTPUT
//{
//	float POS : POSITION;
//	float UV : TEXCOORD0;
//};
//
//
////-----------------------------------------------------------------------------
//// Name: VS_MAIN                                        
//// Type: Vertex shader
////-----------------------------------------------------------------------------
//VS_OUTPUT VS_Main(VS_INPUT input)
//{
//	VS_OUTPUT output;
//	output.POS = mul(input.POS, g_mWorldProjView);
//	output.UV = input.UV;
//	return output;
//}
//
////-----------------------------------------------------------------------------
//// State blocks
////-----------------------------------------------------------------------------
//
//
//
////-----------------------------------------------------------------------------
//// Name: Tech_FSQuad
//// Type: Technique
//// Desc: Post processing with a full screen quad
////-----------------------------------------------------------------------------
