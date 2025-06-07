// main.cpp : Entry point for the OOP Paint application
#include "framework.h"
#include "IShape.h"
#include "RectangleShape.h"
#include "CircleShape.h"
#include "FileManager.h"
#include "Utils.h"
#include "TextShape.h"
#include "Resource.h"

// Global variables
HINSTANCE g_hInst;
HWND g_hStatusBar = nullptr;
COLORREF g_DrawColor = RGB(0, 0, 0);
int g_PenSize = 2;
int g_EraserSize = 10;
bool g_IsEraserMode = false;
bool g_IsAddingText = false;
const wchar_t szWindowClass[] = L" Paint na IPS";
LOGFONT g_logFont = { 0 };
HFONT g_hFont = nullptr;

// Shape management
enum class ShapeType { None, Rectangle, Circle };
ShapeType g_CurrentShapeType = ShapeType::None;
bool g_IsShapeDrawing = false;
POINT g_ShapeStart = { 0, 0 }, g_ShapeEnd = { 0, 0 };

// Drawing state
bool g_IsDrawing = false;
POINT g_LastPoint = { 0, 0 };

// Canvas
HDC g_hMemDC = nullptr;
HBITMAP g_hBitmap = nullptr;
int g_CanvasWidth = 1920, g_CanvasHeight = 1080;

// Shapes container (encapsulation, polymorphism)
std::vector<std::unique_ptr<IShape>> g_Shapes;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void UpdateStatusBar();

// Main entry point
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    g_hInst = hInstance;
    // Register window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance,
        LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT)), LoadCursor(nullptr, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1), nullptr, L" Paint na IPS", LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT)) };
    RegisterClassEx(&wc);

    // Create main window
    HWND hWnd = CreateWindow(L" Paint na IPS", L" Paint na IPS", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1024, 768, nullptr, LoadMenu(hInstance, MAKEINTRESOURCE(IDC_PAINT)), hInstance, nullptr);
    if (!hWnd) return 0;

    // Create status bar
    InitCommonControls();
    g_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, nullptr,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
        hWnd, (HMENU)1, hInstance, nullptr);

    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
    UpdateStatusBar();
    UpdateWindow(hWnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static POINT textPoint = { 0, 0 };

    switch (msg) {
    case WM_CREATE: {
        // Create memory DC and bitmap for canvas
        HDC hdc = GetDC(hWnd);
        g_hMemDC = CreateCompatibleDC(hdc);
        g_hBitmap = CreateCompatibleBitmap(hdc, g_CanvasWidth, g_CanvasHeight);
        SelectObject(g_hMemDC, g_hBitmap);
        HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
        RECT rect = { 0,0,g_CanvasWidth,g_CanvasHeight };
        FillRect(g_hMemDC, &rect, hBrush);
        ReleaseDC(hWnd, hdc);

        g_logFont.lfHeight = -20;
        wcscpy_s(g_logFont.lfFaceName, L"Arial");
        UpdateStatusBar();
        break;
    }
    case WM_SIZE: {
        // Resize status bar
        if (g_hStatusBar) SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
        break;
    }
    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam), y = HIWORD(lParam);
        if (g_IsAddingText) {
            textPoint = { x, y };
            wchar_t text[256] = L"";
            // Show text dialog
            if (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_TEXT_DIALOG), hWnd, [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
                static wchar_t* pText = nullptr;
                switch (message) {
                case WM_INITDIALOG:
                    pText = (wchar_t*)lParam;
                    SetDlgItemTextW(hDlg, IDC_EDIT1, pText);
                    return (INT_PTR)TRUE;
                case WM_COMMAND:
                    if (LOWORD(wParam) == IDOK) {
                        GetDlgItemTextW(hDlg, IDC_EDIT1, pText, 255);
                        EndDialog(hDlg, IDOK);
                        return (INT_PTR)TRUE;
                    }
                    else if (LOWORD(wParam) == IDCANCEL) {
                        EndDialog(hDlg, IDCANCEL);
                        return (INT_PTR)TRUE;
                    }
                    break;
                }
                return (INT_PTR)FALSE;
                }, (LPARAM)text) == IDOK) {
                // Add text as a shape
                g_Shapes.push_back(std::make_unique<TextShape>(
                    textPoint, text, g_DrawColor, g_logFont));
                // Draw the new shape to the memory DC
                g_Shapes.back()->Draw(g_hMemDC);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            g_IsAddingText = false;
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
            UpdateStatusBar();
            break;
        }
        if (g_CurrentShapeType != ShapeType::None) {
            g_IsShapeDrawing = true;
            g_ShapeStart = g_ShapeEnd = { x, y };
            SetCapture(hWnd);
        }
        else {
            g_IsDrawing = true;
            g_LastPoint = { x, y };
            SetCapture(hWnd);
        }
        break;
    }
    case WM_MOUSEMOVE: {
        int x = LOWORD(lParam), y = HIWORD(lParam);
        if (g_IsShapeDrawing && (wParam & MK_LBUTTON)) {
            g_ShapeEnd = { x, y };
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        else if (g_IsDrawing && (wParam & MK_LBUTTON)) {
            POINT pt = { x, y };
            COLORREF penColor = g_IsEraserMode ? RGB(255, 255, 255) : g_DrawColor;
            int penWidth = g_IsEraserMode ? g_EraserSize : g_PenSize;
            HPEN hPen = CreatePen(PS_SOLID, penWidth, penColor);
            HGDIOBJ oldPen = SelectObject(g_hMemDC, hPen);
            MoveToEx(g_hMemDC, g_LastPoint.x, g_LastPoint.y, nullptr);
            LineTo(g_hMemDC, pt.x, pt.y);
            SelectObject(g_hMemDC, oldPen);
            DeleteObject(hPen);
            g_LastPoint = pt;
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
    }
    case WM_LBUTTONUP: {
        int x = LOWORD(lParam), y = HIWORD(lParam);
        if (g_IsShapeDrawing) {
            g_ShapeEnd = { x, y };
            try {
                // Use polymorphism and encapsulation
                if (g_CurrentShapeType == ShapeType::Rectangle) {
                    g_Shapes.push_back(std::make_unique<RectangleShape>(g_ShapeStart, g_ShapeEnd, g_DrawColor, g_PenSize));
                }
                else if (g_CurrentShapeType == ShapeType::Circle) {
                    g_Shapes.push_back(std::make_unique<CircleShape>(g_ShapeStart, g_ShapeEnd, g_DrawColor, g_PenSize));
                }
                // Draw shape to canvas
                g_Shapes.back()->Draw(g_hMemDC);
            }
            catch (const std::exception& ex) {
                MessageBoxA(hWnd, ex.what(), "Error", MB_OK | MB_ICONERROR);
            }
            g_IsShapeDrawing = false;
            g_CurrentShapeType = ShapeType::None;
            ReleaseCapture();
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        else if (g_IsDrawing) {
            g_IsDrawing = false;
            ReleaseCapture();
        }
        break;
    }
    case WM_KEYDOWN: {
        switch (wParam) {
        case 'E':
            g_IsEraserMode = !g_IsEraserMode;
            InvalidateRect(hWnd, nullptr, FALSE);
            UpdateStatusBar();
            break;
        case 'T':
            g_IsAddingText = true;
            SetCursor(LoadCursor(nullptr, IDC_IBEAM));
            UpdateStatusBar();
            break;
        case VK_ESCAPE:
            g_IsShapeDrawing = false;
            g_IsDrawing = false;
            ReleaseCapture();
            InvalidateRect(hWnd, nullptr, FALSE);
            break;
        case VK_F5:
            g_CanvasWidth = 800;
            g_CanvasHeight = 600;
            if (g_hMemDC && g_hBitmap) {
                HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
                RECT rect = { 0,0,g_CanvasWidth,g_CanvasHeight };
                FillRect(g_hMemDC, &rect, hBrush);
                g_Shapes.clear();
                InvalidateRect(hWnd, nullptr, TRUE);
            }
            break;
        case 'R':
            g_CurrentShapeType = ShapeType::Rectangle;
            g_IsEraserMode = false;
            UpdateStatusBar();
            break;
        case 'C':
            g_CurrentShapeType = ShapeType::Circle;
            g_IsEraserMode = false;
            UpdateStatusBar();
            break;
        case 'L':
            g_CurrentShapeType = ShapeType::None;
            g_IsEraserMode = false;
            UpdateStatusBar();
            break;
        }
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_NEW: {
            int newWidth = g_CanvasWidth;
            int newHeight = g_CanvasHeight;
            // Prompt for width
            if (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SIZE_DIALOG), hWnd,
                [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
                    static int* pSize = nullptr;
                    switch (message) {
                    case WM_INITDIALOG:
                        pSize = (int*)lParam;
                        SetDlgItemInt(hDlg, IDC_SIZE_EDIT, *pSize, FALSE);
                        return (INT_PTR)TRUE;
                    case WM_COMMAND:
                        if (LOWORD(wParam) == IDOK) {
                            BOOL success = FALSE;
                            int val = GetDlgItemInt(hDlg, IDC_SIZE_EDIT, &success, FALSE);
                            if (success && val > 0 && val <= 4096) {
                                *pSize = val;
                                EndDialog(hDlg, IDOK);
                            }
                            else {
                                MessageBox(hDlg, L"Please enter a value between 1 and 4096.", L"Invalid Size", MB_OK | MB_ICONWARNING);
                            }
                            return (INT_PTR)TRUE;
                        }
                        else if (LOWORD(wParam) == IDCANCEL) {
                            EndDialog(hDlg, IDCANCEL);
                            return (INT_PTR)TRUE;
                        }
                        break;
                    }
                    return (INT_PTR)FALSE;
                }, (LPARAM)&newWidth) == IDOK) {
                // Prompt for height
                if (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SIZE_DIALOG), hWnd,
                    [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
                        static int* pSize = nullptr;
                        switch (message) {
                        case WM_INITDIALOG:
                            pSize = (int*)lParam;
                            SetDlgItemInt(hDlg, IDC_SIZE_EDIT, *pSize, FALSE);
                            return (INT_PTR)TRUE;
                        case WM_COMMAND:
                            if (LOWORD(wParam) == IDOK) {
                                BOOL success = FALSE;
                                int val = GetDlgItemInt(hDlg, IDC_SIZE_EDIT, &success, FALSE);
                                if (success && val > 0 && val <= 4096) {
                                    *pSize = val;
                                    EndDialog(hDlg, IDOK);
                                }
                                else {
                                    MessageBox(hDlg, L"Please enter a value between 1 and 4096.", L"Invalid Size", MB_OK | MB_ICONWARNING);
                                }
                                return (INT_PTR)TRUE;
                            }
                            else if (LOWORD(wParam) == IDCANCEL) {
                                EndDialog(hDlg, IDCANCEL);
                                return (INT_PTR)TRUE;
                            }
                            break;
                        }
                        return (INT_PTR)FALSE;
                    }, (LPARAM)&newHeight) == IDOK) {
                    if (newWidth > 0 && newWidth <= 4096 && newHeight > 0 && newHeight <= 4096) {
                        g_CanvasWidth = newWidth;
                        g_CanvasHeight = newHeight;
                        if (g_hMemDC && g_hBitmap) {
                            DeleteObject(g_hBitmap);
                            g_hBitmap = CreateCompatibleBitmap(GetDC(hWnd), g_CanvasWidth, g_CanvasHeight);
                            SelectObject(g_hMemDC, g_hBitmap);
                            HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
                            RECT rect = { 0,0,g_CanvasWidth,g_CanvasHeight };
                            FillRect(g_hMemDC, &rect, hBrush);
                            g_Shapes.clear();
                            InvalidateRect(hWnd, NULL, TRUE);
                        }
                    }
                    else {
                        MessageBox(hWnd, L"Width and height must be between 1 and 4096.", L"Invalid Size", MB_OK | MB_ICONWARNING);
                    }
                }
            }
            break;
        }

        case IDM_SAVE: {
            OPENFILENAME ofn = { 0 };
            WCHAR szFile[MAX_PATH] = L"";
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"BMP Files\0*.bmp\0All Files\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_OVERWRITEPROMPT;
            if (GetSaveFileName(&ofn)) {
                try {
                    FileManager::Save(szFile, g_hBitmap, g_hMemDC, g_CanvasWidth, g_CanvasHeight);
                }
                catch (const std::exception& ex) {
                    MessageBoxA(hWnd, ex.what(), "Save Error", MB_OK | MB_ICONERROR);
                }
            }
            break;
        }
        case IDM_LOAD: {
            OPENFILENAME ofn = { 0 };
            WCHAR szFile[MAX_PATH] = L"";
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"BMP Files\0*.bmp\0All Files\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                try {
                    FileManager::Load(szFile, g_hBitmap, g_hMemDC, g_CanvasWidth, g_CanvasHeight);
                    SelectObject(g_hMemDC, g_hBitmap);
                    g_Shapes.clear(); // Clear shapes, as bitmap is loaded
                    InvalidateRect(hWnd, nullptr, FALSE);
                }
                catch (const std::exception& ex) {
                    MessageBoxA(hWnd, ex.what(), "Load Error", MB_OK | MB_ICONERROR);
                }
            }
            break;
        }
        case IDM_ABOUT:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_ADD_TEXT:
            g_IsAddingText = true;
            SetCursor(LoadCursor(NULL, IDC_IBEAM)); // Show text cursor
            UpdateStatusBar();
            break;
        case IDM_ERASER:
            g_IsEraserMode = !g_IsEraserMode; // Toggle eraser mode
            InvalidateRect(hWnd, NULL, FALSE); //update UI
            UpdateStatusBar();
            break;
        case IDM_SET_PEN_SIZE: {
            int newSize = g_PenSize;
            // Show size dialog (implement as needed)
            if (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SIZE_DIALOG), hWnd, [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
                static int* pSize = nullptr;
                switch (message) {
                case WM_INITDIALOG:
                    pSize = (int*)lParam;
                    SetDlgItemInt(hDlg, IDC_SIZE_EDIT, *pSize, FALSE);
                    return (INT_PTR)TRUE;
                case WM_COMMAND:
                    if (LOWORD(wParam) == IDOK) {
                        BOOL success = FALSE;
                        int val = GetDlgItemInt(hDlg, IDC_SIZE_EDIT, &success, FALSE);
                        if (success && val > 0 && val < 4096) {
                            *pSize = val;
                            EndDialog(hDlg, IDOK);
                        }
                        else {
                            MessageBox(hDlg, L"Please enter a value between 1 and 4096.", L"Invalid Size", MB_OK | MB_ICONWARNING);
                        }
                        return (INT_PTR)TRUE;
                    }
                    else if (LOWORD(wParam) == IDCANCEL) {
                        EndDialog(hDlg, IDCANCEL);
                        return (INT_PTR)TRUE;
                    }
                    break;
                }
                return (INT_PTR)FALSE;
                }, (LPARAM)&newSize) == IDOK) {
                g_PenSize = Clamp(newSize, 1, 100);
                UpdateStatusBar();
            }
            break;
        }
        case IDM_SET_ERASER_SIZE: {
            int newSize = g_EraserSize;
            // Show size dialog (implement as above)
            if (DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SIZE_DIALOG), hWnd, [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
                static int* pSize = nullptr;
                switch (message) {
                case WM_INITDIALOG:
                    pSize = (int*)lParam;
                    SetDlgItemInt(hDlg, IDC_SIZE_EDIT, *pSize, FALSE);
                    return (INT_PTR)TRUE;
                case WM_COMMAND:
                    if (LOWORD(wParam) == IDOK) {
                        BOOL success = FALSE;
                        int val = GetDlgItemInt(hDlg, IDC_SIZE_EDIT, &success, FALSE);
                        if (success && val > 0 && val < 4096) {
                            *pSize = val;
                            EndDialog(hDlg, IDOK);
                        }
                        else {
                            MessageBox(hDlg, L"Please enter a value between 1 and 4096.", L"Invalid Size", MB_OK | MB_ICONWARNING);
                        }
                        return (INT_PTR)TRUE;
                    }
                    else if (LOWORD(wParam) == IDCANCEL) {
                        EndDialog(hDlg, IDCANCEL);
                        return (INT_PTR)TRUE;
                    }
                    break;
                }
                return (INT_PTR)FALSE;
                }, (LPARAM)&newSize) == IDOK) {
                g_EraserSize = Clamp(newSize, 1, 100);
                UpdateStatusBar();
            }
            break;
        }
        case IDM_SELECT_COLOR: {
            g_IsEraserMode = false;
            CHOOSECOLOR cc = { 0 };
            static COLORREF customColors[16] = { 0 };
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hWnd;
            cc.rgbResult = g_DrawColor;
            cc.lpCustColors = customColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc)) {
                g_DrawColor = cc.rgbResult;
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            break;
        }
        case IDM_SELECT_FONT: {
            CHOOSEFONT cf = { 0 };
            cf.lStructSize = sizeof(cf);
            cf.hwndOwner = hWnd;
            cf.lpLogFont = &g_logFont;
            cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
            if (ChooseFont(&cf)) {
                if (g_hFont) DeleteObject(g_hFont);
                g_hFont = CreateFontIndirect(&g_logFont);
            }
            break;
        }
        case IDM_SHAPE_RECT:
            g_CurrentShapeType = ShapeType::Rectangle;
            g_IsEraserMode = false;
            UpdateStatusBar();
            break;
        case IDM_SHAPE_CIRC:
            g_CurrentShapeType = ShapeType::Circle;
            g_IsEraserMode = false;
            UpdateStatusBar();
            break;
        case IDM_SHAPE_LINE:
            g_CurrentShapeType = ShapeType::None;
            g_IsEraserMode = false;
            UpdateStatusBar();
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // Use the correct global variables
        if (g_hMemDC && g_hBitmap) {
            // Use canvas size for width/height
            BitBlt(hdc, 0, 0, g_CanvasWidth, g_CanvasHeight, g_hMemDC, 0, 0, SRCCOPY);

            // Define preview box constants if not already defined
            const int g_PreviewBoxMargin = 8;
            const int g_PreviewBoxSize = 32;

            RECT previewBox = {
                g_PreviewBoxMargin,
                g_PreviewBoxMargin,
                g_PreviewBoxMargin + g_PreviewBoxSize,
                g_PreviewBoxMargin + g_PreviewBoxSize
            };

            // Draw shape preview if currently drawing a shape
            if (g_IsShapeDrawing) {
                HPEN hPen = CreatePen(PS_DOT, g_PenSize, g_DrawColor);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
                if (g_CurrentShapeType == ShapeType::Rectangle) {
                    Rectangle(hdc, g_ShapeStart.x, g_ShapeStart.y, g_ShapeEnd.x, g_ShapeEnd.y);
                }
                else if (g_CurrentShapeType == ShapeType::Circle) {
                    Ellipse(hdc, g_ShapeStart.x, g_ShapeStart.y, g_ShapeEnd.x, g_ShapeEnd.y);
                }
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBrush);
                DeleteObject(hPen);
            }

            if (g_IsEraserMode) {
                // Fill with white (eraser color)
                HBRUSH eraserBrush = CreateSolidBrush(RGB(255, 255, 255));
                FillRect(hdc, &previewBox, eraserBrush);
                DeleteObject(eraserBrush);

                // Draw a red border to indicate eraser mode
                HPEN borderPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
                HGDIOBJ oldPen = SelectObject(hdc, borderPen);
                HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
                Rectangle(hdc, previewBox.left, previewBox.top, previewBox.right, previewBox.bottom);
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBrush);
                DeleteObject(borderPen);

                // Draw "Eraser" text below the box
                TextOut(hdc, g_PreviewBoxMargin, g_PreviewBoxMargin + g_PreviewBoxSize + 4, L"Eraser", 6);
            }
            else {
                // Normal color preview
                HBRUSH previewBrush = CreateSolidBrush(g_DrawColor);
                FillRect(hdc, &previewBox, previewBrush);
                DeleteObject(previewBrush);

                // Draw a black border
                HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
                HGDIOBJ oldPen = SelectObject(hdc, borderPen);
                HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
                Rectangle(hdc, previewBox.left, previewBox.top, previewBox.right, previewBox.bottom);
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBrush);
                DeleteObject(borderPen);

                // Draw "Color" text below the box
                TextOut(hdc, g_PreviewBoxMargin, g_PreviewBoxMargin + g_PreviewBoxSize + 4, L"Color", 5);
                UpdateStatusBar();
            }
        }
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        if (g_hMemDC) DeleteDC(g_hMemDC);
        if (g_hBitmap) DeleteObject(g_hBitmap);
        if (g_hFont) DeleteObject(g_hFont);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
// Update status bar with current tool info
void UpdateStatusBar() {
    if (g_hStatusBar) {
        wchar_t tool[32] = L"";
        if (g_IsEraserMode) wcscpy_s(tool, L"Eraser");
        else if (g_IsAddingText) wcscpy_s(tool, L"Text");
        else if (g_CurrentShapeType == ShapeType::Rectangle) wcscpy_s(tool, L"Rectangle");
        else if (g_CurrentShapeType == ShapeType::Circle) wcscpy_s(tool, L"Circle");
        else wcscpy_s(tool, L"Pen");
        wchar_t buf[128];
        swprintf_s(buf, L"Tool: %s   Pen Size: %d   Eraser Size: %d", tool, g_PenSize, g_EraserSize);
        SendMessageW(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)buf);
    }
}
