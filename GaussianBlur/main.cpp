// GaussianBlur.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Resource.h"
#include "D3DGaussianBlur.h"
#include "platform.h"




// 全局变量: 
HINSTANCE hInst;                                // 当前实例
HWND  g_hWnd;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // 执行应用程序初始化: 
	std::auto_ptr<D3DApp> demo(new CD3DGaussianBlur());//使用智能指针
	//CD3DGaussianBlur demo;							 // Demo初始化工作   
	if ( !demo->Initialize(hInstance))
		return -1;
	
	return demo->Run();
    
}




////
////  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  目的:    处理主窗口的消息。
////
////  WM_COMMAND  - 处理应用程序菜单
////  WM_PAINT    - 绘制主窗口
////  WM_DESTROY  - 发送退出消息并返回
////
////
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	//g_winApp->WinProc(hWnd, message, wParam, lParam);
//    switch (message)
//    {
//    case WM_COMMAND:
//        {
//            int wmId = LOWORD(wParam);
//            // 分析菜单选择: 
//            switch (wmId)
//            {
//            case IDM_ABOUT:
//                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
//                break;
//            case IDM_EXIT:
//                DestroyWindow(hWnd);
//                break;
//            default:
//                return DefWindowProc(hWnd, message, wParam, lParam);
//            }
//        }
//        break;
//    case WM_PAINT:
//        {
//            PAINTSTRUCT ps;
//            HDC hdc = BeginPaint(hWnd, &ps);
//            // TODO: 在此处添加使用 hdc 的任何绘图代码...
//            EndPaint(hWnd, &ps);
//        }
//        break;
//	case WM_SIZE:
//		{
//
//		}
//		break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
