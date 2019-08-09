// Project.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Project.h"

#include <iostream>

#include <d3d11.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")

#include "VS.csh"
#include "PS.csh"

using namespace DirectX;

#define MAX_LOADSTRING 100

#define MAX_DLIGHTS 3
#define MAX_PLIGHTS 3
#define MAX_SLIGHTS 3
#define MAX_INSTANCES 5

#define DEGTORAD(deg) (deg * (XM_2PI / 180.0f))

/* KEY
g_ : global
p_ : pointer
S_ : struct
*/

struct S_VERTEX
{
	XMFLOAT4	pos;	//16B
	XMFLOAT4	color;	//16B
	XMFLOAT3	norm;	//12B
	XMFLOAT3	tex;	//12B
};

struct S_DLIGHT
{
	XMFLOAT4	dir;	//16B
	XMFLOAT4	color;	//16B
};

struct S_PLIGHT
{
	XMFLOAT4	pos;	//16B
	XMFLOAT4	color;	//16B
	FLOAT		range;	//4B
	XMFLOAT3	atten;	//12B
};

struct S_SLIGHT
{
	XMFLOAT4	pos;	//16B
	XMFLOAT4	color;	//16B
	XMFLOAT3	dir;	//12B
	FLOAT		range;	//4B
	FLOAT		cone;	//4B
	XMFLOAT3	atten;	//12B
};

struct S_CBUFFER_VS
{
	XMMATRIX	wrld;							//64B
	XMMATRIX	view;							//64B
	XMMATRIX	proj;							//64B
	XMMATRIX	instanceOffsets[MAX_INSTANCES];	//66B * MAX_INSTANCES
	FLOAT		t;								//4B
	XMFLOAT3	pad;							//12B
};

struct S_CBUFFER_PS
{
	XMFLOAT4	ambientColor;					//16B
	XMFLOAT4	instanceColors[MAX_INSTANCES];	//16B * MAX_INSTANCES
	S_DLIGHT	dLights[MAX_DLIGHTS];			//32B * MAX_DLIGHTS
	S_PLIGHT	pLights[MAX_PLIGHTS];			//48B * MAX_PLIGHTS
	S_SLIGHT	sLights[MAX_SLIGHTS];			//64B * MAX_SLIGHTS
	FLOAT		t;								//4B
	XMFLOAT3	pad;							//12B
};

HINSTANCE					g_hInst = nullptr;							// current instance
HWND						g_hWnd = nullptr;							// the window
WCHAR						g_szTitle[MAX_LOADSTRING];					// the title bar text
WCHAR						g_szWindowClass[MAX_LOADSTRING];			// the main window class name

// D3D vars
D3D_DRIVER_TYPE				g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL			g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*				g_p_device = nullptr;						//released
IDXGISwapChain*				g_p_swapChain = nullptr;					//released
ID3D11DeviceContext*		g_p_deviceContext = nullptr;				//released
ID3D11RenderTargetView*		g_p_renderTargetView = nullptr;				//released
D3D11_VIEWPORT				g_viewport;
// input layout
ID3D11InputLayout*			g_p_vertexLayout = nullptr;					//released
// vertex/index buffers
// TEST MESH
ID3D11Buffer*				g_p_vBufferTestMesh = nullptr;				//released
ID3D11Buffer*				g_p_iBufferTestMesh = nullptr;				//released
UINT						g_numVertsTestMesh = 0;
UINT						g_numIndsTestMesh = 0;
// constant buffers
ID3D11Buffer*				g_p_cBufferVS = nullptr;					//released
ID3D11Buffer*				g_p_cBufferPS = nullptr;					//released
// vertex shaders
ID3D11VertexShader*			g_p_VS = nullptr;							//released
// pixel shaders
ID3D11PixelShader*			g_p_PS = nullptr;							//released

// matrices
XMFLOAT4X4					g_wrld;
XMFLOAT4X4					g_view;
XMFLOAT4X4					g_proj;
XMFLOAT4X4					g_wrldTestMesh;

// camera values
FLOAT						g_camMoveSpeed = 3.0f;		// units per second
FLOAT						g_camRotSpeed = 30.0f;		// degrees per second
FLOAT						g_camZoomSpeed = 0.01f;		// zoom level per second
FLOAT						g_camZoom = 1.0f;
const FLOAT					g_camZoomMin = 0.5f;
const FLOAT					g_camZoomMax = 2.0f;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				Render();
void				Cleanup();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PROJECT, g_szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECT));

	MSG msg;

	// Main message loop:
	while (true) //(GetMessage(&msg, nullptr, 0, 0))
	{
		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;

		Render();
	}

	Cleanup();

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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PROJECT);
	wcex.lpszClassName = g_szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
	g_hInst = hInstance; // Store instance handle in our global variable

	g_hWnd = CreateWindowW(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, // XOR with thickframe prevents click & drag resize, XOR with maximizebox prevents control button resize
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!g_hWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	RECT windowRect;
	GetClientRect(g_hWnd, &windowRect);
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;

	// attach d3d11 to window
	HRESULT hr;

	// swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1;														// number of buffers in swap chain
	swapChainDesc.OutputWindow = g_hWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// pixel format
	swapChainDesc.BufferDesc.Width = windowWidth;
	swapChainDesc.BufferDesc.Height = windowHeight;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						// buffer usage flag; specifies what swap chain's buffer will be used for
	swapChainDesc.SampleDesc.Count = 1;													// samples per pixel while drawing
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		createDeviceFlags, &g_featureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &g_p_swapChain, &g_p_device, 0, &g_p_deviceContext);

	// create render target view
	ID3D11Resource* backBuffer = nullptr;
	hr = g_p_swapChain->GetBuffer(0, __uuidof(backBuffer), (void**)&backBuffer);		// get buffer from swap chain
	hr = g_p_device->CreateRenderTargetView(backBuffer, nullptr,						// use buffer to create render target view
		&g_p_renderTargetView);
	backBuffer->Release();

	// setup viewport
	g_viewport.Width = (FLOAT)windowWidth;
	g_viewport.Height = (FLOAT)windowHeight;
	g_viewport.TopLeftX = 0;
	g_viewport.TopLeftX = 0;
	g_viewport.MinDepth = 0.0f;															// exponential depth; near/far planes are handled in projection matrix
	g_viewport.MaxDepth = 1.0f;
	
	// SHADERS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// vertex shaders
	hr = g_p_device->CreateVertexShader(VS, sizeof(VS), nullptr, &g_p_VS);
	// pixel shaders
	hr = g_p_device->CreatePixelShader(PS, sizeof(PS), nullptr, &g_p_PS);
	// SHADERS ----------------------------------------------------------------------------------------------------

	// create input layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numInputElements = ARRAYSIZE(inputElementDesc);
	hr = g_p_device->CreateInputLayout(inputElementDesc, numInputElements, VS, sizeof(VS), &g_p_vertexLayout);

	// set input layout
	g_p_deviceContext->IASetInputLayout(g_p_vertexLayout);

	// MESHES ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// test mesh
	S_VERTEX testMeshVerts[] =
	{
		{ XMFLOAT4(0, -0.5f, -0.5f, 1), XMFLOAT4(0, 0, 1, 1), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0) }, // bottom front
		{ XMFLOAT4(0.5f, -0.5f, 0.5f, 1), XMFLOAT4(0, 1, 0, 1), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0) }, // bottom back right
		{ XMFLOAT4(-0.5f, -0.5f, 0.5f, 1), XMFLOAT4(1, 0, 0, 1), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0) }, // bottom back left
		{ XMFLOAT4(0, 0.5f, 0, 1), XMFLOAT4(1, 1, 1, 1), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0) } // top
	};
	g_numVertsTestMesh = ARRAYSIZE(testMeshVerts);
	
	int testMeshInds[] =
	{
		0, 1, 2, // bottom
		3, 0, 2, // front left
		3, 1, 0, // front right
		3, 2, 1	 // back
	};
	g_numIndsTestMesh = ARRAYSIZE(testMeshInds);

	// setup test mesh vertex buffer
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_VERTEX) * g_numVertsTestMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = testMeshVerts;

	// create test mesh vertex buffer
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_vBufferTestMesh);

	// setup test mesh index buffer
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(int) * g_numIndsTestMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	subData = {};
	subData.pSysMem = testMeshInds;

	// create test mesh index buffer
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_iBufferTestMesh);

	// set test mesh initial world matrix
	XMStoreFloat4x4(&g_wrldTestMesh, XMMatrixIdentity());
	// MESHES ----------------------------------------------------------------------------------------------------

	//_RPTN(0, "FLOAT			: %d\n", sizeof(FLOAT));
	//_RPTN(0, "XMFLOAT4		: %d\n", sizeof(XMFLOAT4));
	//_RPTN(0, "XMVECTOR		: %d\n", sizeof(XMVECTOR));
	//_RPTN(0, "XMFLOAT4X4		: %d\n", sizeof(XMFLOAT4X4));
	//_RPTN(0, "XMMATRIX		: %d\n", sizeof(XMMATRIX));
	//_RPTN(0, "S_DLIGHT		: %d\n", sizeof(S_DLIGHT));
	//_RPTN(0, "S_PLIGHT		: %d\n", sizeof(S_PLIGHT));
	//_RPTN(0, "S_SLIGHT		: %d\n", sizeof(S_SLIGHT));
	//_RPTN(0, "S_CBUFFER_VS	: %d\n", sizeof(S_CBUFFER_VS));
	//_RPTN(0, "S_CBUFFER_PS	: %d\n", sizeof(S_CBUFFER_PS));
	
	// set type of topology to draw
	g_p_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);										

	// create VS constant buffer
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_CBUFFER_VS);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	hr = g_p_device->CreateBuffer(&bufferDesc, nullptr, &g_p_cBufferVS);

	// create PS constant buffer
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_CBUFFER_PS);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	hr = g_p_device->CreateBuffer(&bufferDesc, nullptr, &g_p_cBufferPS);

	// MATRICES ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	XMStoreFloat4x4(&g_wrld, XMMatrixIdentity());

	XMVECTOR eye = XMVectorSet(0, -2, -10, 0);
	XMVECTOR at = XMVectorSet(0, 0, 0, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
	XMStoreFloat4x4(&g_view, XMMatrixInverse(&XMMatrixDeterminant(view), view));

	XMStoreFloat4x4(&g_proj, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	// MATRICES ----------------------------------------------------------------------------------------------------


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
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	//case WM_PAINT:
	//    {
	//        PAINTSTRUCT ps;
	//        HDC hdc = BeginPaint(hWnd, &ps);
	//        // TODO: Add any drawing code that uses hdc here...
	//        EndPaint(hWnd, &ps);
	//    }
	//    break;
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

void Render()
{
	// update time
	static ULONGLONG timeStart = 0;
	static ULONGLONG timePrev = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	static float t = (timeCur - timeStart) / 1000.0f;
	float dt = (timeCur - timePrev) / 1000.0f;
	timePrev = timeCur;

	// get window dimensions
	RECT windowRect;
	GetClientRect(g_hWnd, &windowRect);
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;

	// set byte strides and offsets
	UINT strides[] = { sizeof(S_VERTEX) };
	UINT offsets[] = { 0 };

	// create shader constant buffers
	S_CBUFFER_VS cBufferVS = {};
	S_CBUFFER_PS cBufferPS = {};

	// retrieve matrices
	XMMATRIX wrld = XMLoadFloat4x4(&g_wrld);
	XMMATRIX view = XMLoadFloat4x4(&g_view);
	XMMATRIX proj = XMLoadFloat4x4(&g_proj);
	XMMATRIX wrldTestMesh = XMLoadFloat4x4(&g_wrldTestMesh);

	// update camera
	// position
	if (GetAsyncKeyState('W'))
		view = XMMatrixTranslation(0, 0, g_camMoveSpeed * dt) * view;
	if (GetAsyncKeyState('S'))
		view = XMMatrixTranslation(0, 0, -1 * g_camMoveSpeed * dt) * view;
	if (GetAsyncKeyState('D'))
		view = XMMatrixTranslation(g_camMoveSpeed * dt, 0, 0) * view;
	if (GetAsyncKeyState('A'))
		view = XMMatrixTranslation(-1 * g_camMoveSpeed * dt, 0, 0) * view;
	if (GetAsyncKeyState(VK_SPACE))
		view = XMMatrixTranslation(0, g_camMoveSpeed * dt, 0) * view;
	if (GetAsyncKeyState(VK_LSHIFT))
		view = XMMatrixTranslation(0, -1 * g_camMoveSpeed * dt, 0) * view;

	// rotation
	if (GetAsyncKeyState(VK_DOWN))
		view = XMMatrixRotationX(DEGTORAD(g_camRotSpeed) * dt) * view;
	if (GetAsyncKeyState(VK_UP))
		view = XMMatrixRotationX(-1 * DEGTORAD(g_camRotSpeed) * dt) * view;
	if (GetAsyncKeyState(VK_LEFT))
	{
		XMVECTOR trans = view.r[3];
		view = view * XMMatrixTranslationFromVector(-1 * trans);
		view = view * XMMatrixRotationY(-1 * DEGTORAD(g_camRotSpeed) * dt);
		view = view * XMMatrixTranslationFromVector(trans);
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		XMVECTOR trans = view.r[3];
		view = view * XMMatrixTranslationFromVector(-1 * trans);
		view = view * XMMatrixRotationY(DEGTORAD(g_camRotSpeed) * dt);
		view = view * XMMatrixTranslationFromVector(trans);
	}

	// zoom
	if (GetAsyncKeyState(VK_OEM_MINUS))
	{
		g_camZoom -= g_camMoveSpeed * dt;
		if (g_camZoom < g_camZoomMin)
			g_camZoom = g_camZoomMin;
	}
	if (GetAsyncKeyState(VK_OEM_PLUS))
	{
		g_camZoom += g_camMoveSpeed * dt;
		if (g_camZoom > g_camZoomMax)
			g_camZoom = g_camZoomMax;
	}
	if (GetAsyncKeyState(VK_BACK))
		g_camZoom = 1.0f;

	proj = XMMatrixPerspectiveFovLH(XM_PIDIV4 / g_camZoom, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f);
	
	//--------------------------------------------------
	// DRAWING
	//--------------------------------------------------

	float clearColor[4] = { 0, 0, 0.25f, 1 };
	g_p_deviceContext->ClearRenderTargetView(g_p_renderTargetView, clearColor);		// clear render target view

	// RASTERIZER
	g_p_deviceContext->RSSetViewports(1, &g_viewport);								// set viewport
	// OUTPUT MERGER
	// set render target view
	ID3D11RenderTargetView* p_tempRTV[] = { g_p_renderTargetView };					// temporarily convert render target view to array while setting RTV
	g_p_deviceContext->OMSetRenderTargets(1, p_tempRTV, nullptr);					// TODO: replace 3rd arg(nullptr) with zbuffer / depth stencil view


	XMMATRIX rotate = XMMatrixRotationY(0.5f * dt);

	// setup test mesh for draw
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBufferTestMesh, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBufferTestMesh, DXGI_FORMAT_R32_UINT, 0);

	wrldTestMesh = wrldTestMesh * rotate;
	cBufferVS.wrld = XMMatrixTranspose(wrldTestMesh);
	cBufferVS.view = XMMatrixTranspose(XMMatrixInverse(&XMMatrixDeterminant(view), view));
	cBufferVS.proj = XMMatrixTranspose(proj);
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);

	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetConstantBuffers(0, 1, &g_p_cBufferPS);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	// draw test mesh
	g_p_deviceContext->DrawIndexed(g_numIndsTestMesh, 0, 0);


	g_p_swapChain->Present(1, 0);													// present back buffer; change args to limit/sync framerate

	// store matrices
	XMStoreFloat4x4(&g_wrld, wrld);
	XMStoreFloat4x4(&g_view, view);
	XMStoreFloat4x4(&g_proj, proj);
	XMStoreFloat4x4(&g_wrldTestMesh, wrldTestMesh);
}

void Cleanup()
{
	if (g_p_PS) g_p_PS->Release();
	if (g_p_VS) g_p_VS->Release();
	if (g_p_cBufferPS) g_p_cBufferPS->Release();
	if (g_p_cBufferVS) g_p_cBufferVS->Release();
	if (g_p_iBufferTestMesh) g_p_iBufferTestMesh->Release();
	if (g_p_vBufferTestMesh) g_p_vBufferTestMesh->Release();
	if (g_p_vertexLayout) g_p_vertexLayout->Release();
	if (g_p_renderTargetView) g_p_renderTargetView->Release();
	if (g_p_deviceContext) g_p_deviceContext->Release();
	if (g_p_swapChain) g_p_swapChain->Release();
	if (g_p_device) g_p_device->Release();
}
