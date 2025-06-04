// PAINT.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "paint.h"

#define MAX_LOADSTRING 100

// Forward declaration of SizeDialogProc
INT_PTR CALLBACK SizeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
COLORREF g_DrawColor = RGB(0, 0, 0);            // Default: black
enum ShapeType { SHAPE_NONE, SHAPE_RECT, SHAPE_CIRC };
ShapeType g_ShapeType = SHAPE_NONE;
bool isShapeDrawing = false;
POINT shapeStart = { 0, 0 };
POINT shapeEnd = { 0, 0 };
const int g_PreviewBoxSize = 32;    // Size of the preview box (width and height)
const int g_PreviewBoxMargin = 8;   // Margin from the window edge
bool isEraserMode = false;
int g_PenSize = 2;      // Default pen size
int g_EraserSize = 10;  // Default eraser size
HWND hStatusBar = NULL;
std::vector<HBITMAP> g_UndoStack;
const int MAX_UNDO = 10; // Limit the number of undo steps



// Drawing state
bool isDrawing = false;
POINT lastPoint = { 0, 0 };
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HBITMAP CopyBitmap(HDC hdcSrc, HBITMAP hbmSrc, int width, int height) {
    HDC hdcMemSrc = CreateCompatibleDC(hdcSrc);
    HDC hdcMemDst = CreateCompatibleDC(hdcSrc);
    HBITMAP hbmCopy = CreateCompatibleBitmap(hdcSrc, width, height);
    HGDIOBJ oldSrc = SelectObject(hdcMemSrc, hbmSrc);
    HGDIOBJ oldDst = SelectObject(hdcMemDst, hbmCopy);
    BitBlt(hdcMemDst, 0, 0, width, height, hdcMemSrc, 0, 0, SRCCOPY);
    SelectObject(hdcMemSrc, oldSrc);
    SelectObject(hdcMemDst, oldDst);
    DeleteDC(hdcMemSrc);
    DeleteDC(hdcMemDst);
    return hbmCopy;
}

void PushUndo(HDC hdc, HBITMAP hBitmap, int width, int height) {
    if (!hBitmap) return;
    HBITMAP hCopy = CopyBitmap(hdc, hBitmap, width, height);
    g_UndoStack.push_back(hCopy);
    if (g_UndoStack.size() > MAX_UNDO) {
        DeleteObject(g_UndoStack.front());
        g_UndoStack.erase(g_UndoStack.begin());
    }
}

void PopUndo(HDC& hdc, HBITMAP& hBitmap, int width, int height) {
    if (g_UndoStack.empty()) return;
    HBITMAP hPrev = g_UndoStack.back();
    g_UndoStack.pop_back();

    // Select the new bitmap into the memory DC
    HGDIOBJ oldBitmap = SelectObject(hdc, hPrev);

    // Delete the current bitmap (avoid memory leak)
    if (hBitmap && hBitmap != hPrev) {
        DeleteObject(hBitmap);
    }
    hBitmap = hPrev;
}

void NewCanvas(HDC& hMemDC, HBITMAP& hBitmap, int width, int height) {
    // Clear undo stack
    for (HBITMAP hbm : g_UndoStack) {
        DeleteObject(hbm);
    }
    g_UndoStack.clear();

    // Delete old bitmap
    if (hBitmap) {
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }

    // Create new bitmap and select into memory DC
    hBitmap = CreateCompatibleBitmap(hMemDC, width, height);
    SelectObject(hMemDC, hBitmap);

    // Fill with white
    HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RECT rect = { 0, 0, width, height };
    FillRect(hMemDC, &rect, hBrush);

    // Push initial state for undo
    PushUndo(hMemDC, hBitmap, width, height);
}

// For GetSaveFileName/GetOpenFileName
bool SaveBitmapToFile(HBITMAP hBitmap, HDC hdc, int width, int height, LPCWSTR filename) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPFILEHEADER bmfHeader = { 0 };
    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    int lineBytes = ((width * 3 + 3) & ~3);
    int imageSize = lineBytes * height;
    std::vector<BYTE> bits(imageSize);

    BITMAPINFO biInfo = { 0 };
    biInfo.bmiHeader = bi;

    // Get the bitmap bits
    GetDIBits(hdc, hBitmap, 0, height, bits.data(), &biInfo, DIB_RGB_COLORS);

    bmfHeader.bfType = 0x4D42; // 'BM'
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = bmfHeader.bfOffBits + imageSize;

    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD dwWritten = 0;
    WriteFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwWritten, NULL);
    WriteFile(hFile, &bi, sizeof(bi), &dwWritten, NULL);
    WriteFile(hFile, bits.data(), imageSize, &dwWritten, NULL);
    CloseHandle(hFile);
    return true;
}

bool LoadBitmapFromFile(HDC hdc, HBITMAP& hBitmap, int& width, int& height, LPCWSTR filename) {
    HANDLE hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    BITMAPFILEHEADER bmfHeader;
    DWORD dwRead = 0;
    ReadFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwRead, NULL);
    if (bmfHeader.bfType != 0x4D42) { CloseHandle(hFile); return false; }

    BITMAPINFOHEADER bi;
    ReadFile(hFile, &bi, sizeof(bi), &dwRead, NULL);

    int imageSize = bi.biSizeImage;
    if (imageSize == 0) {
        int lineBytes = ((bi.biWidth * 3 + 3) & ~3);
        imageSize = lineBytes * abs(bi.biHeight);
    }
    std::vector<BYTE> bits(imageSize);
    SetFilePointer(hFile, bmfHeader.bfOffBits, NULL, FILE_BEGIN);
    ReadFile(hFile, bits.data(), imageSize, &dwRead, NULL);
    CloseHandle(hFile);

    HBITMAP hNewBitmap = CreateCompatibleBitmap(hdc, bi.biWidth, abs(bi.biHeight));
    HDC hMemDC = CreateCompatibleDC(hdc);
    HGDIOBJ oldBmp = SelectObject(hMemDC, hNewBitmap);

    BITMAPINFO biInfo = { 0 };
    biInfo.bmiHeader = bi;
    SetDIBits(hMemDC, hNewBitmap, 0, abs(bi.biHeight), bits.data(), &biInfo, DIB_RGB_COLORS);

    SelectObject(hMemDC, oldBmp);
    DeleteDC(hMemDC);

    if (hBitmap) DeleteObject(hBitmap);
    hBitmap = hNewBitmap;
    width = bi.biWidth;
    height = abs(bi.biHeight);
    return true;
}

void UpdateStatusBar()
{
    if (hStatusBar) {
        wchar_t buf[128];
        swprintf_s(buf, L"Pen Size: %d   Eraser Size: %d", g_PenSize, g_EraserSize);
        SendMessageW(hStatusBar, SB_SETTEXT, 0, (LPARAM)buf);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PAINT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PAINT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PAINT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    // Create status bar
    InitCommonControls();
    hStatusBar = CreateWindowExW(
        0, STATUSCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hWnd, (HMENU)1, hInst, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hMemDC = NULL;
    static HBITMAP hBitmap = NULL;
    static int width = 0, height = 0;

    switch (message)
    {
    case WM_SIZE:
    {
        width = LOWORD(lParam);
        height = HIWORD(lParam);

        if (hMemDC) {
            DeleteDC(hMemDC);
            hMemDC = NULL;
        }
        if (hBitmap) {
            DeleteObject(hBitmap);
            hBitmap = NULL;
        }

        HDC hdc = GetDC(hWnd);
        hMemDC = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(hMemDC, hBitmap);
        // Fill with white
        HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
        RECT rect = { 0, 0, width, height };
        FillRect(hMemDC, &rect, hBrush);
        ReleaseDC(hWnd, hdc);
        for (HBITMAP hbm : g_UndoStack) {
            DeleteObject(hbm);
        }
        g_UndoStack.clear();

        // Push initial state for undo
        PushUndo(hMemDC, hBitmap, width, height);
    }
    break;
    case WM_LBUTTONDOWN:
        if (g_ShapeType != SHAPE_NONE) {
            isShapeDrawing = true;
            shapeStart.x = shapeEnd.x = LOWORD(lParam);
            shapeStart.y = shapeEnd.y = HIWORD(lParam);
            SetCapture(hWnd);
        }
        else {
            if (!isShapeDrawing && !isDrawing && hMemDC && hBitmap && width > 0 && height > 0) {
                PushUndo(hMemDC, hBitmap, width, height);
            }
            isDrawing = true;
            lastPoint.x = LOWORD(lParam);
            lastPoint.y = HIWORD(lParam);
            SetCapture(hWnd);
        }
        break;
    case WM_MOUSEMOVE:
        if (isShapeDrawing && (wParam & MK_LBUTTON)) {
            shapeEnd.x = LOWORD(lParam);
            shapeEnd.y = HIWORD(lParam);
            InvalidateRect(hWnd, NULL, FALSE); // To show preview
        }
        else if (isDrawing && (wParam & MK_LBUTTON)) {
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);

            if (hMemDC) {
                COLORREF penColor = isEraserMode ? RGB(255, 255, 255) : g_DrawColor; // White for eraser
                int penWidth = isEraserMode ? g_EraserSize : g_PenSize;
                HPEN hPen = CreatePen(PS_SOLID, penWidth, penColor); // Wider pen for eraser
                HGDIOBJ oldPen = SelectObject(hMemDC, hPen);
                MoveToEx(hMemDC, lastPoint.x, lastPoint.y, NULL);
                LineTo(hMemDC, pt.x, pt.y);
                SelectObject(hMemDC, oldPen);
                DeleteObject(hPen);
            }


            lastPoint = pt;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    case WM_LBUTTONUP:
        if (isShapeDrawing) {
            if (isShapeDrawing && hMemDC && hBitmap && width > 0 && height > 0) {
                PushUndo(hMemDC, hBitmap, width, height);
            }
            shapeEnd.x = LOWORD(lParam);
            shapeEnd.y = HIWORD(lParam);
            // Draw the shape permanently to hMemDC
            HPEN hPen = CreatePen(PS_SOLID, g_PenSize, g_DrawColor);
            HGDIOBJ oldPen = SelectObject(hMemDC, hPen);
            HGDIOBJ oldBrush = SelectObject(hMemDC, GetStockObject(HOLLOW_BRUSH));
            if (g_ShapeType == SHAPE_RECT) {
                Rectangle(hMemDC, shapeStart.x, shapeStart.y, shapeEnd.x, shapeEnd.y);
            }
            else if (g_ShapeType == SHAPE_CIRC) {
                Ellipse(hMemDC, shapeStart.x, shapeStart.y, shapeEnd.x, shapeEnd.y);
            }
            SelectObject(hMemDC, oldPen);
            SelectObject(hMemDC, oldBrush);
            DeleteObject(hPen);
            isShapeDrawing = false;
            g_ShapeType = SHAPE_NONE;
            ReleaseCapture();
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (isDrawing) {
            isDrawing = false;
            ReleaseCapture();
        }
        break;
    case WM_KEYDOWN:
        if (wParam == 'E') {
            isEraserMode = !isEraserMode;
            InvalidateRect(hWnd, NULL, FALSE); //update UI
        }
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId){
        case IDM_NEW:
            if (hMemDC && width > 0 && height > 0) {
                NewCanvas(hMemDC, hBitmap, width, height);
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;

        case IDM_SAVE:
        {
            OPENFILENAME ofn = { 0 };
            WCHAR szFile[MAX_PATH] = L"";
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"BMP Files\0*.bmp\0All Files\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_OVERWRITEPROMPT;
            if (GetSaveFileName(&ofn)) {
                SaveBitmapToFile(hBitmap, hMemDC, width, height, szFile);
            }
        }
        break;
        case IDM_LOAD:
        {
            OPENFILENAME ofn = { 0 };
            WCHAR szFile[MAX_PATH] = L"";
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"BMP Files\0*.bmp\0All Files\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;
            if (GetOpenFileName(&ofn)) {
                LoadBitmapFromFile(hMemDC, hBitmap, width, height, szFile);
                SelectObject(hMemDC, hBitmap);
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_ERASER:
            isEraserMode = !isEraserMode; // Toggle eraser mode
            InvalidateRect(hWnd, NULL, FALSE); //update UI
            break;
        case IDM_SELECT_COLOR:
        {
            isEraserMode = false;
            CHOOSECOLOR cc = { 0 };
            static COLORREF customColors[16] = { 0 };
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hWnd;
            cc.rgbResult = g_DrawColor;
            cc.lpCustColors = customColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc)) {
                g_DrawColor = cc.rgbResult;
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;
        case IDM_SET_PEN_SIZE:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SIZE_DIALOG), hWnd, SizeDialogProc, (LPARAM)&g_PenSize);
            UpdateStatusBar();
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        case IDM_SET_ERASER_SIZE:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SIZE_DIALOG), hWnd, SizeDialogProc, (LPARAM)&g_EraserSize);
            UpdateStatusBar();
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        case IDM_SHAPE_RECT:
            g_ShapeType = SHAPE_RECT;
            isEraserMode = false;
            break;
        case IDM_SHAPE_CIRC:
            g_ShapeType = SHAPE_CIRC;
            isEraserMode = false;
            break;
		case IDM_SHAPE_LINE:
			g_ShapeType = SHAPE_NONE; // Reset shape type
			isEraserMode = false;
			break;
        case IDM_UNDO:
            PopUndo(hMemDC, hBitmap, width, height);
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (hMemDC && hBitmap) {
            BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
            // Draw color preview box in the top-left corner
            RECT previewBox = {
                g_PreviewBoxMargin,
                g_PreviewBoxMargin,
                g_PreviewBoxMargin + g_PreviewBoxSize,
                g_PreviewBoxMargin + g_PreviewBoxSize
            };
            // Draw shape preview if currently drawing a shape
            if (isShapeDrawing) {
                HPEN hPen = CreatePen(PS_DOT, g_PenSize, g_DrawColor);
                HGDIOBJ oldPen = SelectObject(hdc, hPen);
                HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
                if (g_ShapeType == SHAPE_RECT) {
                    Rectangle(hdc, shapeStart.x, shapeStart.y, shapeEnd.x, shapeEnd.y);
                }
                else if (g_ShapeType == SHAPE_CIRC) {
                    Ellipse(hdc, shapeStart.x, shapeStart.y, shapeEnd.x, shapeEnd.y);
                }
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBrush);
                DeleteObject(hPen);
            }
            if (isEraserMode) {
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
            }

        }
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        if (hMemDC) DeleteDC(hMemDC);
        if (hBitmap) DeleteObject(hBitmap);
        for (HBITMAP hbm : g_UndoStack) {
            DeleteObject(hbm);
        }
        g_UndoStack.clear();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK SizeDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int* pSize = nullptr;
    switch (message)
    {
    case WM_INITDIALOG:
        pSize = (int*)lParam;
        SetDlgItemInt(hDlg, IDC_SIZE_EDIT, *pSize, FALSE);
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            BOOL success = FALSE;
            int val = GetDlgItemInt(hDlg, IDC_SIZE_EDIT, &success, FALSE);
            if (success && val > 0 && val < 100)
            {
                *pSize = val;
                EndDialog(hDlg, IDOK);
            }
            else
            {
                MessageBox(hDlg, L"Please enter a value between 1 and 99.", L"Invalid Size", MB_OK | MB_ICONWARNING);
            }
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
