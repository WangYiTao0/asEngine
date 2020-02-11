//#include <asEngine.h>
//#include "resource.h"
//
int main()
{

}

//#define MAX_LOADSTRING 100
//
//// Global Variables:
//HINSTANCE hInst;                                // current instance
//WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
//WCHAR szwindowClass[MAX_LOADSTRING];            // the main as::asndow class name
//as::MainComponent mainC;								// as::ascked Engine Main Runtime Component
//
//// Forward declarations of functions included in this code module:
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
//
//
//int APIENTRY wwinMain(_In_ HINSTANCE hInstance,
//    _In_opt_ HINSTANCE hPrevInstance,
//    _In_ LPWSTR    lpCmdLine,
//    _In_ int       nCmdShow)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//
//    // TODO: Place code here.
//
//    as::asStartupArguments::Parse(lpCmdLine); // if you as::assh to use command line arguments, here is a good place to parse them...
//
//    // Initialize global strings
//    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//    LoadStringW(hInstance, IDC_TEMPLATEWINDOWS, szwindowClass, MAX_LOADSTRING);
//    MyRegisterClass(hInstance);
//
//    // Perform application initialization:
//    if (!InitInstance(hInstance, nCmdShow))
//    {
//        return FALSE;
//    }
//
//    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEMPLATEWINDOWS));
//
//    // just show some basic info:
//    mainC.infoDisplay.active = true;
//    mainC.infoDisplay.watermark = true;
//    mainC.infoDisplay.resolution = true;
//    mainC.infoDisplay.fpsinfo = true;
//
//    MSG msg = { 0 };
//    while (msg.message != WM_QUIT)
//    {
//        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//        else {
//
//            mainC.Run(); // run the update - render loop (mandatory)
//
//        }
//    }
//
//    return (int)msg.wParam;
//}
//
//
//
////
////  FUNCTION: MyRegisterClass()
////
////  PURPOSE: Registers the as::asndow class.
////
//ATOM MyRegisterClass(HINSTANCE hInstance)
//{
//    WNDCLASSEXW wcex;
//
//    wcex.cbSize = sizeof(WNDCLASSEX);
//
//    wcex.style = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc = WndProc;
//    wcex.cbClsExtra = 0;
//    wcex.cbWndExtra = 0;
//    wcex.hInstance = hInstance;
//    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEMPLATEWINDOWS));
//    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
//    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TEMPLATEWINDOWS);
//    wcex.lpszClassName = szwindowClass;
//    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
//
//    return RegisterClassExW(&wcex);
//}
//
////
////   FUNCTION: InitInstance(HINSTANCE, int)
////
////   PURPOSE: Saves instance handle and creates main as::asndow
////
////   COMMENTS:
////
////        In this function, we save the instance handle in a global variable and
////        create and display the main program as::asndow.
////
//BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
//{
//    hInst = hInstance; // Store instance handle in our global variable
//
//    HWND hWnd = CreateWindowW(szwindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
//
//    if (!hWnd)
//    {
//        return FALSE;
//    }
//
//    ShowWindow(hWnd, nCmdShow);
//    UpdateWindow(hWnd);
//
//
//    mainC.SetWindow(hWnd); // assign as::asndow handle (mandatory)
//
//
//    return TRUE;
//}
//
////
////  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  PURPOSE:  Processes messages for the main as::asndow.
////
////  WM_COMMAND  - process the application menu
////  WM_PAINT    - Paint the main as::asndow
////  WM_DESTROY  - post a quit message and return
////
////
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    switch (message)
//    {
//    case WM_COMMAND:
//    {
//        int wmId = LOWORD(wParam);
//        // Parse the menu selections:
//        switch (wmId)
//        {
//        case IDM_ABOUT:
//            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
//            break;
//        case IDM_EXIT:
//            DestroyWindow(hWnd);
//            break;
//        default:
//            return DefWindowProc(hWnd, message, wParam, lParam);
//        }
//    }
//    break;
//    case WM_SIZE:
//    {
//        if (as::asRenderer::GetDevice() != nullptr)
//        {
//            int width = LOWORD(lParam);
//            int height = HIWORD(lParam);
//
//           as::asRenderer::GetDevice()->SetResolution(width, height);
//            as::asRenderer::GetCamera().CreatePerspective((float)as::asRenderer::GetInternalResolution().x, (float)as::asRenderer::GetInternalResolution().y, 0.1f, 800);
//        }
//    }
//    break;
//    case WM_KEYDOWN:
//        switch (wParam)
//        {
//        case VK_HOME:
//            as::asBackLog::Toggle();
//            break;
//        case VK_UP:
//            if (as::asBackLog::isActive())
//                as::asBackLog::historyPrev();
//            break;
//        case VK_DOWN:
//            if (as::asBackLog::isActive())
//                as::asBackLog::historyNext();
//            break;
//        case VK_NEXT:
//            if (as::asBackLog::isActive())
//                as::asBackLog::Scroll(10);
//            break;
//        case VK_PRIOR:
//            if (as::asBackLog::isActive())
//                as::asBackLog::Scroll(-10);
//            break;
//        default:
//            break;
//        }
//        break;
//    case WM_CHAR:
//        switch (wParam)
//        {
//        case VK_BACK:
//            if (as::asBackLog::isActive())
//                as::asBackLog::deletefromInput();
//           // as::asTextInputField::DeleteFromInput();
//            break;
//        case VK_RETURN:
//            if (as::asBackLog::isActive())
//                as::asBackLog::acceptInput();
//            break;
//        default:
//        {
//            const char c = (const char)(TCHAR)wParam;
//            if (as::asBackLog::isActive())
//            {
//                as::asBackLog::input(c);
//            }
//          //  as::asTextInputField::AddInput(c);
//        }
//        break;
//        }
//        break;
//    case WM_PAINT:
//    {
//        PAINTSTRUCT ps;
//        HDC hdc = BeginPaint(hWnd, &ps);
//        // TODO: Add any draas::asng code that uses hdc here...
//        EndPaint(hWnd, &ps);
//    }
//    break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
//
//// Message handler for about box.
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    UNREFERENCED_PARAMETER(lParam);
//    switch (message)
//    {
//    case WM_INITDIALOG:
//        return (INT_PTR)TRUE;
//
//    case WM_COMMAND:
//        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//        {
//            EndDialog(hDlg, LOWORD(wParam));
//            return (INT_PTR)TRUE;
//        }
//        break;
//    }
//    return (INT_PTR)FALSE;
//}
