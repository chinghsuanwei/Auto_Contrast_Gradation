// auto_constract_windows.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "auto_constract_windows.h"
#include "wingdi.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#define MAX_LOADSTRING 100
#define THRESHOLD 0.03
#define BMPHEADSIZE 54

#define THRESHOLD_LOW 0.33
#define THRESHOLD_HIGH 0.99
#define _CRT_SECURE_NO_WARNINGS

typedef unsigned char       IMAGE_TYPE;
typedef unsigned char       T_U8;
typedef short       T_U16;
typedef unsigned int       T_U32;

unsigned char img_data[1024*1024*20];

//Original function
int Auto_Contrast_Gradation(IMAGE_TYPE* bmp_img, double dlowcut, double dhighcut)
{
	DWORD width, height, dst_index, index;
	WORD  biBitCount;
	T_U8* bmp_data, R, G, B, * Result_img, BytePerPixel = 3;
	T_U32 line_byte, TmpR, TmpG, TmpB, Sum, PixCount;
	T_U16 i, j, MinBlue, MaxBlue, MinRed, MaxRed, MinGreen, MaxGreen, Max, Min, Y;
	int newX, newY;
	double Tmp;
	double HistRed[256] = { 0 };
	double HistGreen[256] = { 0 };
	double HistBlue[256] = { 0 };
	int bluemap[256] = { 0 };
	int redmap[256] = { 0 };
	int greenmap[256] = { 0 };
	int Map[256] = { 0 };


	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;

	FILE* Auto_Contrast_Gradation_fp;
	fopen_s(&Auto_Contrast_Gradation_fp, "Auto_Contrast_Gradation.bmp", "wb");

	if (NULL == Auto_Contrast_Gradation_fp)
	{
		printf("Can't open Auto_Contrast_Gradation.bmp\n");
		return -1;
	}

	memset(&bf, 0, sizeof(bf));
	memset(&bi, 0, sizeof(bi));

	memcpy(&bf, bmp_img, 14);
	memcpy(&bi, &bmp_img[14], 40);

	height = bi.biHeight;
	width = bi.biWidth;
	biBitCount = bi.biBitCount;//每一个像素由24 bits表示，即RGB分量每一个分量用8 bits表示
	line_byte = (width * 3 + 3) & ~0x3;

	fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, Auto_Contrast_Gradation_fp);
	fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, Auto_Contrast_Gradation_fp);


	bmp_data = bmp_img + BMPHEADSIZE;

	for (i = 0; i < height; i++)
	{
		dst_index = i * line_byte;
		for (j = 0; j < width; j++)
		{
			R = bmp_data[dst_index + 2];
			G = bmp_data[dst_index + 1];
			B = bmp_data[dst_index];

			HistRed[R]++;
			HistGreen[G]++;
			HistBlue[B]++;

			dst_index += BytePerPixel;

		}
	}


	PixCount = height * width;


	//For Blue Channel
	Sum = 0;
	for (Y = 0; Y < 256; Y++)
	{
		Sum += HistBlue[Y];
		if (Sum >= PixCount * dlowcut * THRESHOLD)
		{
			MinBlue = Y;
			break;
		}
	}

	Sum = 0;
	for (Y = 255; Y >= 0; Y--)
	{
		Sum += HistBlue[Y];
		if (Sum >= PixCount * dhighcut * THRESHOLD)
		{
			MaxBlue = Y;
			break;
		}
	}


	//For Green Channel
	Sum = 0;
	for (Y = 0; Y < 256; Y++)
	{
		Sum += HistGreen[Y];
		if (Sum >= PixCount * dlowcut * THRESHOLD)
		{
			MinGreen = Y;
			break;
		}
	}

	Sum = 0;
	for (Y = 255; Y >= 0; Y--)
	{
		Sum += HistGreen[Y];
		if (Sum >= PixCount * dhighcut * THRESHOLD)
		{
			MaxGreen = Y;
			break;
		}
	}


	//For Red Channel
	Sum = 0;
	for (Y = 0; Y < 256; Y++)
	{
		Sum += HistRed[Y];
		if (Sum >= PixCount * dlowcut * THRESHOLD)
		{
			MinRed = Y;
			break;
		}
	}

	Sum = 0;
	for (Y = 255; Y >= 0; Y--)
	{
		Sum += HistRed[Y];
		if (Sum >= PixCount * dhighcut * THRESHOLD)
		{
			MaxRed = Y;
			break;
		}
	}


	for (Y = 0; Y < 256; Y++)
	{
		if (Y <= MinBlue)
			bluemap[Y] = 0;
		else if (Y >= MaxBlue)
			bluemap[Y] = 255;
		else
		{
			Tmp = (double)(Y - MinBlue) / (MaxBlue - MinBlue);
			bluemap[Y] = (int)(Tmp * 255);
		}
	}


	for (Y = 0; Y < 256; Y++)
	{
		if (Y <= MinGreen)
			greenmap[Y] = 0;
		else if (Y >= MaxGreen)
			greenmap[Y] = 255;
		else
		{
			Tmp = (double)(Y - MinGreen) / (MaxGreen - MinGreen);
			greenmap[Y] = (int)(Tmp * 255);
		}
	}



	for (Y = 0; Y < 256; Y++)
	{
		if (Y <= MinRed)
			redmap[Y] = 0;
		else if (Y >= MaxRed)
			redmap[Y] = 255;
		else
		{
			Tmp = (double)(Y - MinRed) / (MaxRed - MinRed);
			redmap[Y] = (int)(Tmp * 255);
		}
	}


	//Auto Gradation
	for (i = 0; i < height; i++)
	{
		dst_index = i * line_byte;
		for (j = 0; j < width; j++)
		{
			bmp_data[dst_index] = bluemap[bmp_data[dst_index]];
			bmp_data[dst_index + 1] = greenmap[bmp_data[dst_index + 1]];
			bmp_data[dst_index + 2] = redmap[bmp_data[dst_index + 2]];

			dst_index += BytePerPixel;

		}
	}


	//Auto Contrast
	if (MinBlue < MinGreen)
		Min = MinBlue;
	else
		Min = MinGreen;
	if (Min > MinRed)
		Min = MinRed;

	if (MaxBlue > MaxGreen)
		Max = MaxBlue;
	else
		Max = MaxGreen;
	if (Max < MaxRed)
		Max = MaxRed;


	for (Y = 0; Y < 256; Y++)
	{
		if (Y <= Min)
			Map[Y] = 0;
		else if (Y > Max)
			Map[Y] = 255;
		else
		{
			Tmp = (double)(Y - Min) / (Max - Min);
			Map[Y] = (int)(Tmp * 255);
		}
	}


	for (i = 0; i < height; i++)
	{
		dst_index = i * line_byte;
		for (j = 0; j < width; j++)
		{
			bmp_data[dst_index] = Map[bmp_data[dst_index]];
			bmp_data[dst_index + 1] = Map[bmp_data[dst_index + 1]];
			bmp_data[dst_index + 2] = Map[bmp_data[dst_index + 2]];

			dst_index += BytePerPixel;

		}
	}

	fwrite(bmp_data, line_byte * height * sizeof(T_U8), 1, Auto_Contrast_Gradation_fp);
	fclose(Auto_Contrast_Gradation_fp);
	return 0;
}

/**————————————————
版权声明：本文为CSDN博主「大熊背」的原创文章，遵循CC 4.0 BY - SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https ://blog.csdn.net/lz0499/article/details/77619068
*/


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AUTOCONSTRACTWINDOWS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUTOCONSTRACTWINDOWS));

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

    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCONSTRACTWINDOWS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_AUTOCONSTRACTWINDOWS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

 //  int sz = 0;
 //  FILE* fp = NULL;
 //  fopen_s(&fp, "test2.bmp", "r");
 //  fseek(fp, 0L, SEEK_END);
 //  sz = ftell(fp);
 //  rewind(fp);
 //  
 //  char ch;
	//int i = 0;
	//while (i<sz){
	//	ch = fgetc(fp);
	//	//printf("%c", ch);
	//	memcpy(&img_data[i], &ch, sizeof(char));
	//	i++;
	//}
	////printf("i = %d", i);
	//if(fp)
	//	fclose(fp);
   

   //for (double x = 0.01; x <= 1.0; x += 0.1)
   //{
	  // for (double y = 0.1; y <= 0.2; y += 0.1)
	  // {
		 //  std::ifstream file("a.bmp", std::ios::binary | std::ios::ate);
		 //  std::streamsize size = file.tellg();
		 //  file.seekg(0, std::ios::beg);
		 //  std::vector<char> buffer(size);
		 //  if (file.read(buffer.data(), size))
		 //  {
			//   /* worked! */
		 //  }

		 //  string s = "result_";
		 //  s += std::to_string(x);
		 //  s += std::to_string(y);
		 //  s += ".bmp";
		 //  Auto_Contrast_Gradation((unsigned char*)&buffer[0], x, y, s);
	  // }
   //}
   
   std::ifstream file("test.bmp", std::ios::binary | std::ios::ate);
   std::streamsize size = file.tellg();
   file.seekg(0, std::ios::beg);
   std::vector<char> buffer(size);
   if (file.read(buffer.data(), size))
   {
	   /* worked! */
   }

   string s = "result_";
   s += ".bmp";
   Auto_Contrast_Gradation((unsigned char*)&buffer[0], 0.1, 0.1);


	
	
   
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
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
