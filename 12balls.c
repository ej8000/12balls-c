#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ID_Button1 160
#define ID_Button2 161
#define ID_Button3 162
#define ID_Edit    150

typedef struct {
    HBITMAP BMP;
    RECT RC;
    double W;
    int CNT;
    HWND CTL;
} BOX;

BOX Ball[24];
int cx, cy;
int oldBall;
int bSnag, bDrag;
int odd;
double hvy;
const char* msg;

HWND hForm, hLBL;
HBITMAP scimg = NULL;
HBITMAP hBall[24] = {0};
POINT pt;

HWND Button[3];
HWND Edit1;

void InitBalls(HWND hwnd) {
    if (scimg) DeleteObject(scimg);
    scimg = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(15), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    for (int q = 0; q < 24; q++) {
        int x, y;
        if (q < 12) x = 108 + q * 26;
        else if (q < 18) x = 38 + (q - 12) * 26;
        else x = 295 + (q - 18) * 26;
        y = (q > 11) ? 126 : 280;

        Ball[q].CTL = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_BITMAP,
                                   x, y, 25, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
        if (q < 12) {
            hBall[q] = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(2), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
            HDC hdcScrn = GetDC(NULL);
            HDC hdcMem = CreateCompatibleDC(hdcScrn);
            SelectObject(hdcMem, hBall[q]);
            HFONT hFont = CreateFont(12, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, "Verdana");
            HFONT hOldFont = SelectObject(hdcMem, hFont);
            SetTextColor(hdcMem, RGB(0, 0, 255)); // Blue text
            SetBkMode(hdcMem, TRANSPARENT);
            char num[10];
            wsprintf(num, "%d", q + 1);
            TextOut(hdcMem, 8 - 2 * lstrlen(num), 5, num, lstrlen(num));
            SelectObject(hdcMem, hOldFont);
            DeleteObject(hFont);
            DeleteDC(hdcMem);
            ReleaseDC(NULL, hdcScrn);
            Ball[q].W = 1.0;
        } else {
            hBall[q] = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(3), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
            Ball[q].W = 0.0;
        }
        SendMessage(Ball[q].CTL, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBall[q]);
        Ball[q].BMP = hBall[q];
        SetRect(&Ball[q].RC, x, y, x + 25, y + 25);
        SetWindowPos(Ball[q].CTL, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void shuffle() {
    srand(GetTickCount());
    odd = rand() % 12;
    if ((rand() % 2) != 0) {
        hvy = 0.5;
        msg = "heavy";
    } else {
        hvy = -0.5;
        msg = "light";
    }
    Ball[odd].W += hvy;
}

void prepare(HWND hwnd) {
    SetWindowText(Edit1, "");
    for (int q = 0; q < 24; q++) {
        if (Ball[q].CTL) DestroyWindow(Ball[q].CTL);
        if (hBall[q]) DeleteObject(hBall[q]);
    }
    InitBalls(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
    shuffle();
}

void weigh(HWND hwnd) {
    double Lw = 0.0;
    double Rw = 0.0;
    for (int i = 12; i <= 17; i++) {
        Lw += Ball[i].W;
        Rw += Ball[i + 6].W;
    }
    
    if (scimg) DeleteObject(scimg);
    
    if (Lw > Rw) {
        scimg = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(16), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        InvalidateRect(hwnd, NULL, TRUE);
        for (int i = 12; i < 24; i++) {
            int x = (i < 18) ? (38 + (i - 12) * 26) : (295 + (i - 18) * 26);
            int y = (i > 17) ? 112 : 140;
            SetRect(&Ball[i].RC, x, y, x + 25, y + 25);
            SetWindowPos(Ball[i].CTL, HWND_TOP, Ball[i].RC.left, Ball[i].RC.top, 0, 0, SWP_NOSIZE);
        }
    } else if (Lw < Rw) {
        scimg = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(17), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        InvalidateRect(hwnd, NULL, TRUE);
        for (int i = 12; i < 24; i++) {
            int x = (i < 18) ? (38 + (i - 12) * 26) : (295 + (i - 18) * 26);
            int y = (i > 17) ? 142 : 108;
            SetRect(&Ball[i].RC, x, y, x + 25, y + 25);
            SetWindowPos(Ball[i].CTL, HWND_TOP, Ball[i].RC.left, Ball[i].RC.top, 0, 0, SWP_NOSIZE);
        }
    } else {
        scimg = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(15), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        InvalidateRect(hwnd, NULL, TRUE);
        for (int i = 12; i < 24; i++) {
            int x = (i < 18) ? (38 + (i - 12) * 26) : (295 + (i - 18) * 26);
            int y = 126;
            SetRect(&Ball[i].RC, x, y, x + 25, y + 25);
            SetWindowPos(Ball[i].CTL, HWND_TOP, Ball[i].RC.left, Ball[i].RC.top, 0, 0, SWP_NOSIZE);
        }
    }
}

void solve() {
    char buf[100];
    wsprintf(buf, "the ball %d is %s", odd + 1, msg);
    SetWindowText(Edit1, buf);
}

void Grab(HWND hwnd, int x, int y) {
    pt.x = x;
    pt.y = y;
    for (int q = 0; q < 24; q++) {
        if (PtInRect(&Ball[q].RC, pt) && Ball[q].W != 0) {
            cx = pt.x - Ball[q].RC.left;
            cy = pt.y - Ball[q].RC.top;
            oldBall = q;
            bDrag = 1;
            SetCapture(hwnd);
            RECT rc;
            GetWindowRect(hwnd, &rc);
            ClipCursor(&rc);
            break;
        }
    }
}

void Drag(HWND hwnd, int x, int y) {
    if (bDrag) {
        SetWindowPos(Ball[oldBall].CTL, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE); 
        MoveWindow(Ball[oldBall].CTL, x - cx, y - cy, 25, 25, TRUE); 
    }
}

void Drop(HWND hwnd, int x, int y) {
    if (!bDrag) return;
    int ol = Ball[oldBall].RC.left;
    int ot = Ball[oldBall].RC.top;
    bDrag = 0;
    ReleaseCapture();
    ClipCursor(NULL);

    pt.x = x;
    pt.y = y;
    bSnag = 0;

    for (int q = 0; q < 24; q++) {
        if (PtInRect(&Ball[q].RC, pt)) {
            int w = 25, h = 25;
            int l = Ball[q].RC.left, t = Ball[q].RC.top;

            MoveWindow(Ball[oldBall].CTL, l, t, w, h, TRUE);
            MoveWindow(Ball[q].CTL, ol, ot, w, h, TRUE);

            HWND tempCtl = Ball[q].CTL;
            Ball[q].CTL = Ball[oldBall].CTL;
            Ball[oldBall].CTL = tempCtl;

            double tempW = Ball[q].W;
            Ball[q].W = Ball[oldBall].W;
            Ball[oldBall].W = tempW;

            bSnag = 1;
            break;
        }
    }

    if (!bSnag) {
        MoveWindow(Ball[oldBall].CTL, ol, ot, 25, 25, TRUE);
    }
    InvalidateRect(hwnd, NULL, TRUE); // Force repaint
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hForm = hwnd;
            Edit1 = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY | ES_CENTER,
                                   420, 250, 150, 22, hwnd, (HMENU)ID_Edit, GetModuleHandle(NULL), NULL);
            Button[0] = CreateWindow("BUTTON", "weigh", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     490, 20, 80, 25, hwnd, (HMENU)ID_Button1, GetModuleHandle(NULL), NULL);
            Button[1] = CreateWindow("BUTTON", "reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     490, 50, 80, 25, hwnd, (HMENU)ID_Button2, GetModuleHandle(NULL), NULL);
            Button[2] = CreateWindow("BUTTON", "solve", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     490, 80, 80, 25, hwnd, (HMENU)ID_Button3, GetModuleHandle(NULL), NULL);
                                     
            SendMessage(Edit1, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
            SendMessage(Button[0], WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
            SendMessage(Button[1], WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
            SendMessage(Button[2], WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));

            InitBalls(hwnd);
            shuffle();
            break;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_Button1) weigh(hwnd);
            if (LOWORD(wParam) == ID_Button2) prepare(hwnd);
            if (LOWORD(wParam) == ID_Button3) solve();
            break;
            
        case WM_LBUTTONDOWN:
            Grab(hwnd, LOWORD(lParam), HIWORD(lParam));
            break;
            
        case WM_MOUSEMOVE:
            Drag(hwnd, LOWORD(lParam), HIWORD(lParam));
            break;
            
        case WM_LBUTTONUP:
            Drop(hwnd, LOWORD(lParam), HIWORD(lParam));
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (scimg) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HGDIOBJ old = SelectObject(hdcMem, scimg);
                BitBlt(hdc, 0, 20, 481, 217, hdcMem, 0, 0, SRCCOPY);
                SelectObject(hdcMem, old);
                DeleteDC(hdcMem);
            }
            EndPaint(hwnd, &ps);
            break;
        }
            
        case WM_DESTROY:
            for (int q = 0; q < 24; q++) {
                if (hBall[q]) DeleteObject(hBall[q]);
            }
            if (scimg) DeleteObject(scimg);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "12BallsCGame";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("12BallsCGame", "12Balls (C version)", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 600, 350, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
