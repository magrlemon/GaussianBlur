// GaussianBlur.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "Resource.h"
#include "D3DGaussianBlur.h"
#include "platform.h"




// ȫ�ֱ���: 
HINSTANCE hInst;                                // ��ǰʵ��
HWND  g_hWnd;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // ִ��Ӧ�ó����ʼ��: 
	std::auto_ptr<D3DApp> demo(new CD3DGaussianBlur());//ʹ������ָ��
	//CD3DGaussianBlur demo;							 // Demo��ʼ������   
	if ( !demo->Initialize(hInstance))
		return -1;
	
	return demo->Run();
    
}




////
////  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  Ŀ��:    ���������ڵ���Ϣ��
////
////  WM_COMMAND  - ����Ӧ�ó���˵�
////  WM_PAINT    - ����������
////  WM_DESTROY  - �����˳���Ϣ������
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
//            // �����˵�ѡ��: 
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
//            // TODO: �ڴ˴����ʹ�� hdc ���κλ�ͼ����...
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
