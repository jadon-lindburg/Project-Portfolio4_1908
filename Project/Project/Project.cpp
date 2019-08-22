// ---------- INCLUDES ----------
//#include "stdafx.h"
#include "Project.h"

#include <iostream>

// DirectX
#include <d3d11.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")
#include "DDSTextureLoader.h"

#include "OBJDataLoader.h"

// shaders
#include "VS.csh"
#include "VS_Distort.csh"
#include "GS.csh"
#include "GS_Distort.csh"
#include "PS.csh"
#include "PS_CubeMap.csh"
#include "PS_Distort.csh"
#include "PS_InputColor.csh"
#include "PS_InputColorLights.csh"
#include "PS_SolidColor.csh"
#include "PS_SolidColorLights.csh"

// meshes (Obj2Header)
#include "Assets/heavenTorch.h"

using namespace DirectX;
// ---------- INCLUDES ----------

// ---------- MACROS ----------
#define MAX_LOADSTRING 100

#define MAX_INSTANCES 5
#define MAX_LIGHTS_DIR 3
#define MAX_LIGHTS_PNT 3
#define MAX_LIGHTS_SPT 3

#define DEGTORAD(deg) (deg * (XM_2PI / 180.0f))
// ---------- MACROS ----------

/* KEY
g_	: global
p_	: pointer
pp_	: double pointer
S_	: struct
*/

// ---------- STRUCTS ----------
struct S_VERTEX
{
	XMFLOAT4	pos;	//16B
	XMFLOAT3	norm;	//12B
	XMFLOAT3	tex;	//12B
	XMFLOAT4	color;	//16B
};
struct S_LIGHT_DIR
{
	XMFLOAT4	dir;	//16B
	XMFLOAT4	color;	//16B
};
struct S_LIGHT_PNT
{
	XMFLOAT4	pos;	//16B
	FLOAT		range;	//4B
	XMFLOAT3	atten;	//12B
	XMFLOAT4	color;	//16B
};
struct S_LIGHT_SPT
{
	XMFLOAT4	pos;	//16B
	XMFLOAT4	dir;	//16B
	FLOAT		range;	//4B
	FLOAT		cone;	//4B
	XMFLOAT3	atten;	//12B
	XMFLOAT4	color;	//16B
};
struct S_CBUFFER_VS
{
	XMMATRIX	wrld;							//64B
	XMMATRIX	view;							//64B
	XMMATRIX	proj;							//64B
	XMMATRIX	instanceOffsets[MAX_INSTANCES];	//64B * MAX_INSTANCES
	FLOAT		t;								//4B
	XMFLOAT3	pad;							//12B
};
struct S_CBUFFER_PS
{
	XMFLOAT4	ambientColor;					//16B
	XMFLOAT4	instanceColors[MAX_INSTANCES];	//16B * MAX_INSTANCES
	S_LIGHT_DIR	dLights[MAX_LIGHTS_DIR];		//32B * MAX_LIGHTS_DIR
	S_LIGHT_PNT	pLights[MAX_LIGHTS_PNT];		//48B * MAX_LIGHTS_PNT
	//S_LIGHT_SPT	sLights[MAX_LIGHTS_SPT];		//68B * MAX_LIGHTS_SPT
	FLOAT		t;								//4B
	XMFLOAT3	pad;
};
// ---------- STRUCTS ----------

// ---------- GLOBAL VARS ----------
// ----- WIN32 VARS -----
HINSTANCE					g_hInst = nullptr;							// current instance
HWND						g_hWnd = nullptr;							// the window
WCHAR						g_szTitle[MAX_LOADSTRING];					// the title bar text
WCHAR						g_szWindowClass[MAX_LOADSTRING];			// the main window class name
// ----- WIN32 VARS -----

// ----- D3D VARS -----
D3D_DRIVER_TYPE				g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL			g_featureLevel = D3D_FEATURE_LEVEL_11_0;
// --- DEVICE / SWAP CHAIN ---
ID3D11Device*				g_p_device = nullptr;						//released
IDXGISwapChain*				g_p_swapChain = nullptr;					//released
ID3D11DeviceContext*		g_p_deviceContext = nullptr;				//released
// --- DEVICE / SWAP CHAIN ---
// --- RENDER TARGET VIEWS ---
ID3D11RenderTargetView*		g_p_renderTargetView = nullptr;				//released
// --- RENDER TARGET VIEWS ---
// --- DEPTH STENCILS ---
ID3D11Texture2D*			g_p_depthStencil = nullptr;					//released
ID3D11DepthStencilView*		g_p_depthStencilView = nullptr;				//released
// --- DEPTH STENCILS ---
// --- VIEWPORTS ---
D3D11_VIEWPORT				g_viewport;
// --- VIEWPORTS ---
// --- INPUT LAYOUT ---
ID3D11InputLayout*			g_p_vertexLayout = nullptr;					//released
// --- INPUT LAYOUT ---
// --- VERT / IND BUFFERS ---
// TEST MESH
ID3D11Buffer*				g_p_vBuffer_TestHardMesh = nullptr;			//released
ID3D11Buffer*				g_p_iBuffer_TestHardMesh = nullptr;			//released
UINT						g_numVerts_TestHardMesh = 0;
UINT						g_numInds_TestHardMesh = 0;
// TEST OBJ2HEADER MESH
ID3D11Buffer*				g_p_vBuffer_TestHeaderMesh = nullptr;		//released
ID3D11Buffer*				g_p_iBuffer_TestHeaderMesh = nullptr;		//released
UINT						g_numVerts_TestHeaderMesh = 0;
UINT						g_numInds_TestHeaderMesh = 0;
// TEST PROCEDURAL MESH
ID3D11Buffer*				g_p_vBuffer_TestProcMesh = nullptr;			//released
ID3D11Buffer*				g_p_iBuffer_TestProcMesh = nullptr;			//released
UINT						g_numVerts_TestProcMesh = 0;
UINT						g_numInds_TestProcMesh = 0;
UINT						g_numDivisions_TestProcMesh = 100;
FLOAT						g_scale_TestProcMesh = 100.0f;
// TEST LOAD MESH
ID3D11Buffer*				g_p_vBuffer_TestLoadMesh = nullptr;			//released
ID3D11Buffer*				g_p_iBuffer_TestLoadMesh = nullptr;			//released
UINT						g_numVerts_TestLoadMesh = 0;
UINT						g_numInds_TestLoadMesh = 0;
// --- VERT / IND BUFFERS ---
// --- CONSTANT BUFFERS ---
ID3D11Buffer*				g_p_cBufferVS = nullptr;					//released
ID3D11Buffer*				g_p_cBufferPS = nullptr;					//released
// --- CONSTANT BUFFERS ---
// --- SHADER RESOURCE VIEWS ---
ID3D11ShaderResourceView*	g_p_texRV_TestHeaderMesh = nullptr;			//released
// --- SHADER RESOURCE VIEWS ---
// --- SAMPLER STATES ---
ID3D11SamplerState*			g_p_samplerLinear = nullptr;				//released
// --- SAMPLER STATES ---
// --- SHADERS ---
// VERTEX
ID3D11VertexShader*			g_p_VS = nullptr;							//released
ID3D11VertexShader*			g_p_VS_Distort = nullptr;					//released
// GEOMETRY
ID3D11GeometryShader*		g_p_GS = nullptr;							//released
ID3D11GeometryShader*		g_p_GS_Distort = nullptr;					//released
// PIXEL
ID3D11PixelShader*			g_p_PS = nullptr;							//released
ID3D11PixelShader*			g_p_PS_CubeMap = nullptr;					//released
ID3D11PixelShader*			g_p_PS_Distort = nullptr;					//released
ID3D11PixelShader*			g_p_PS_InputColor = nullptr;				//released
ID3D11PixelShader*			g_p_PS_InputColorLights = nullptr;			//released
ID3D11PixelShader*			g_p_PS_SolidColor = nullptr;				//released
ID3D11PixelShader*			g_p_PS_SolidColorLights = nullptr;			//released
// --- SHADERS ---
// ----- D3D vars -----

// ----- MATRICES -----
XMFLOAT4X4					g_wrld;
XMFLOAT4X4					g_view;
XMFLOAT4X4					g_proj;
XMFLOAT4X4					g_wrld_TestHardMesh;
XMFLOAT4X4					g_wrld_TestHeaderMesh;
XMFLOAT4X4					g_wrld_TestProcMesh;
XMFLOAT4X4					g_wrld_TestLoadMesh;
// ----- MATRICES -----

// ----- CAMERA VALUES -----
FLOAT						g_camMoveSpeed = 4.0f;		// units per second
FLOAT						g_camRotSpeed = 25.0f;		// degrees per second
FLOAT						g_camZoomSpeed = 0.01f;		// zoom level per second
FLOAT						g_camZoom = 1.0f;			// current zoom level
const FLOAT					g_camZoomMin = 0.5f;
const FLOAT					g_camZoomMax = 2.0f;
FLOAT						g_camNearSpeed = 1.0f;		// near plane change per second
FLOAT						g_camNearPlane = 0.01f;		// current near plane
const FLOAT					g_camNearMin = 0.01f;
const FLOAT					g_camNearMax = 9.0f;
FLOAT						g_camFarSpeed = 10.0f;		// far plane change per second
FLOAT						g_camFarPlane = 100.0f;		// current far plane
const FLOAT					g_camFarMin = 10.0f;
const FLOAT					g_camFarMax = 100.0f;
// ----- CAMERA VALUES -----

// ----- TOGGLES -----
bool						g_freelook = true;
bool						g_defaultVS = true;
bool						g_defaultGS = true;
bool						g_defaultPS = true;
// ----- TOGGLES -----
// ---------- GLOBAL VARS ----------

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				ProcessHeaderVerts(_OBJ_VERT_*, UINT, S_VERTEX**);
void				CreateProceduralGrid(S_VERTEX, UINT, FLOAT, S_VERTEX**, UINT&, UINT**, UINT&);
void				ProcessOBJData(const char*, S_VERTEX**, UINT&, UINT**, UINT&);
void				Render();
void				Cleanup();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

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

	g_hWnd = CreateWindowW(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	// ++ XOR with thickframe prevents click & drag resize, XOR with maximizebox prevents control button resize

	if (!g_hWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	HRESULT hr;

	// get window dimensions
	RECT windowRect;
	GetClientRect(g_hWnd, &windowRect);
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;

	// --------------------------------------------------
	// ATTACH D3D TO WINDOW

	// ---------- D3D DEVICE AND SWAP CHAIN ----------
	// ----- SWAP CHAIN DESCRIPTOR -----
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1; // number of buffers in swap chain
	swapChainDesc.OutputWindow = g_hWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // pixel format
	swapChainDesc.BufferDesc.Width = windowWidth;
	swapChainDesc.BufferDesc.Height = windowHeight;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // buffer usage flag; specifies what swap chain's buffer will be used for
	swapChainDesc.SampleDesc.Count = 1; // samples per pixel while drawing
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	// ----- SWAP CHAIN DESCRIPTOR -----
	// create device and swap chain
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		createDeviceFlags, &g_featureLevel, 1, D3D11_SDK_VERSION,
		&swapChainDesc, &g_p_swapChain, &g_p_device, 0, &g_p_deviceContext);
	// ---------- D3D DEVICE AND SWAP CHAIN ----------

	// ---------- RENDER TARGET VIEWS ----------
	ID3D11Resource* backBuffer = nullptr;
	// get buffer from swap chain
	hr = g_p_swapChain->GetBuffer(0, __uuidof(backBuffer), (void**)&backBuffer);
	// use buffer to create render target view
	hr = g_p_device->CreateRenderTargetView(backBuffer, nullptr,
		&g_p_renderTargetView);
	backBuffer->Release();
	// ---------- RENDER TARGET VIEWS ----------

	// ---------- DEPTH STENCILS ----------
	// create depth stencil texture
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = windowWidth;
	depthStencilDesc.Height = windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	hr = g_p_device->CreateTexture2D(&depthStencilDesc, nullptr, &g_p_depthStencil);

	// create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = g_p_device->CreateDepthStencilView(g_p_depthStencil, &depthStencilViewDesc, &g_p_depthStencilView);
	// ---------- DEPTH STENCILS ----------

	// ---------- VIEWPORTS ----------
	// setup main viewport
	g_viewport.Width = (FLOAT)windowWidth;
	g_viewport.Height = (FLOAT)windowHeight;
	g_viewport.TopLeftX = 0;
	g_viewport.TopLeftX = 0;
	g_viewport.MinDepth = 0.0f; // exponential depth; near/far planes are handled in projection matrix
	g_viewport.MaxDepth = 1.0f;
	// ---------- VIEWPORTS ----------

	// ---------- SHADERS ----------
	// ----- VERTEX SHADERS -----
	hr = g_p_device->CreateVertexShader(VS_Distort, sizeof(VS_Distort), nullptr, &g_p_VS_Distort);
	hr = g_p_device->CreateVertexShader(VS, sizeof(VS), nullptr, &g_p_VS);
	// ----- VERTEX SHADERS -----
	// ----- GEOMETRY SHADERS -----
	hr = g_p_device->CreateGeometryShader(GS, sizeof(GS), nullptr, &g_p_GS);
	hr = g_p_device->CreateGeometryShader(GS_Distort, sizeof(GS_Distort), nullptr, &g_p_GS_Distort);
	// ----- GEOMETRY SHADERS -----
	// ----- PIXEL SHADERS -----
	hr = g_p_device->CreatePixelShader(PS, sizeof(PS), nullptr, &g_p_PS);
	hr = g_p_device->CreatePixelShader(PS_CubeMap, sizeof(PS_CubeMap), nullptr, &g_p_PS_CubeMap);
	hr = g_p_device->CreatePixelShader(PS_Distort, sizeof(PS_Distort), nullptr, &g_p_PS_Distort);
	hr = g_p_device->CreatePixelShader(PS_InputColor, sizeof(PS_InputColor), nullptr, &g_p_PS_InputColor);
	hr = g_p_device->CreatePixelShader(PS_InputColorLights, sizeof(PS_InputColorLights), nullptr, &g_p_PS_InputColorLights);
	hr = g_p_device->CreatePixelShader(PS_SolidColor, sizeof(PS_SolidColor), nullptr, &g_p_PS_SolidColor);
	hr = g_p_device->CreatePixelShader(PS_SolidColorLights, sizeof(PS_SolidColorLights), nullptr, &g_p_PS_SolidColorLights);
	// ----- PIXEL SHADERS -----
	// ---------- SHADERS ----------

	// ---------- INPUT LAYOUT ----------
	// input layout data
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numInputElements = ARRAYSIZE(inputElementDesc);
	// create input layout
	hr = g_p_device->CreateInputLayout(inputElementDesc, numInputElements, VS, sizeof(VS), &g_p_vertexLayout);
	// set input layout
	g_p_deviceContext->IASetInputLayout(g_p_vertexLayout);
	// ---------- INPUT LAYOUT ----------

	// ---------- MESHES ----------
	// ----- TEST HARDCODED MESH -----
	// --- VERTEX BUFFER DATA ---
	S_VERTEX verts_TestHardMesh[] =
	{
		// pos, norm, tex, color
		{ { 0, -0.5f, -0.5f, 1 }, { 0, -0.5f, -0.5f }, { 0, 0, 0 }, { 0, 0, 1, 1} }, // bottom front
		{ { 0.5f, -0.5f, 0.5f, 1 }, { 0.33f, -0.33f, 0.33f }, { 0, 0, 0 }, { 0, 1, 0, 1} }, // bottom back right
		{ { -0.5f, -0.5f, 0.5f, 1 }, { -0.33f, -0.33f, 0.33f }, { 0, 0, 0 }, { 1, 0, 0, 1} }, // bottom back left
		{ { 0, 0.5f, 0, 1 }, { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 1, 1} } // top
	};
	g_numVerts_TestHardMesh = ARRAYSIZE(verts_TestHardMesh);
	// --- VERTEX BUFFER DATA ---
	// --- INDEX BUFFER DATA ---
	int inds_TestHardMesh[] =
	{
		0, 1, 2, // bottom
		3, 0, 2, // front left
		3, 1, 0, // front right
		3, 2, 1	 // back
	};
	g_numInds_TestHardMesh = ARRAYSIZE(inds_TestHardMesh);
	// --- INDEX BUFFER DATA ---
	// --- CREATE VERTEX BUFFER ---
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_VERTEX) * g_numVerts_TestHardMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = verts_TestHardMesh;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_vBuffer_TestHardMesh);
	// --- CREATE VERTEX BUFFER ---
	// --- CREATE INDEX BUFFER ---
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(int) * g_numInds_TestHardMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	subData = {};
	subData.pSysMem = inds_TestHardMesh;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_iBuffer_TestHardMesh);
	// --- CREATE INDEX BUFFER ---
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_TestHardMesh, XMMatrixIdentity());
	// ----- TEST HARDCODED MESH -----

	// ----- TEST OBJ2HEADER MESH -----
	// --- CONVERT VERTEX DATA ---
	// get number of verts
	g_numVerts_TestHeaderMesh = ARRAYSIZE(heavenTorch_data);
	// store verts
	S_VERTEX* p_verts_TestHeaderMesh = nullptr;
	ProcessHeaderVerts((_OBJ_VERT_*)&heavenTorch_data, g_numVerts_TestHeaderMesh, &p_verts_TestHeaderMesh);
	// --- CONVERT VERTEX DATA ---
	// get number of inds
	g_numInds_TestHeaderMesh = ARRAYSIZE(heavenTorch_indicies);
	// --- CREATE VERTEX BUFFER ---
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_VERTEX) * g_numVerts_TestHeaderMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	subData = {};
	subData.pSysMem = p_verts_TestHeaderMesh;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_vBuffer_TestHeaderMesh);
	// --- CREATE VERTEX BUFFER ---
	// --- CREATE INDEX BUFFER ---
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(int) * g_numInds_TestHeaderMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	subData = {};
	subData.pSysMem = heavenTorch_indicies;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_iBuffer_TestHeaderMesh);
	// --- CREATE INDEX BUFFER ---
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_TestHeaderMesh, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_TestHeaderMesh;
	// ----- TEST OBJ2HEADER MESH -----

	// ----- TEST PROCEDURAL MESH -----
	// --- GENERATE VERTEX / INDEX DATA ---
	S_VERTEX gridOrigin = { { 0, 0, 0, 1 }, { 0, 1, 0 }, { 0, 0, 0 }, {} };
	S_VERTEX* p_verts_TestProcMesh = nullptr;
	UINT* p_inds_TestProcMesh = nullptr;
	CreateProceduralGrid(gridOrigin, g_numDivisions_TestProcMesh, g_scale_TestProcMesh,
		&p_verts_TestProcMesh, g_numVerts_TestProcMesh, &p_inds_TestProcMesh, g_numInds_TestProcMesh);
	// --- GENERATE VERTEX / INDEX DATA ---
	// --- CREATE VERTEX BUFFER ---
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_VERTEX) * g_numVerts_TestProcMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	subData = {};
	subData.pSysMem = p_verts_TestProcMesh;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_vBuffer_TestProcMesh);
	// --- CREATE VERTEX BUFFER ---
	// --- CREATE INDEX BUFFER ---
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(int) * g_numInds_TestProcMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	subData = {};
	subData.pSysMem = p_inds_TestProcMesh;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_iBuffer_TestProcMesh);
	// --- CREATE INDEX BUFFER ---
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_TestProcMesh, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_TestProcMesh;
	delete[] p_inds_TestProcMesh;
	// ----- TEST PROCEDURAL MESH -----

	// ----- TEST LOAD MESH -----
	// --- LOAD VERTEX / INDEX DATA ---
	S_VERTEX* p_verts_TestLoadMesh = nullptr;
	UINT* p_inds_TestLoadMesh = nullptr;
	ProcessOBJData("Assets/heavenTorch.obj", &p_verts_TestLoadMesh, g_numVerts_TestLoadMesh, &p_inds_TestLoadMesh, g_numInds_TestLoadMesh);
	// --- CREATE VERTEX BUFFER ---
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_VERTEX) * g_numVerts_TestLoadMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	subData = {};
	subData.pSysMem = p_verts_TestLoadMesh;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_vBuffer_TestLoadMesh);
	// --- CREATE VERTEX BUFFER ---
	// --- CREATE INDEX BUFFER ---
	bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(int) * g_numInds_TestLoadMesh;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	subData = {};
	subData.pSysMem = p_inds_TestLoadMesh;
	hr = g_p_device->CreateBuffer(&bufferDesc, &subData, &g_p_iBuffer_TestLoadMesh);
	// --- CREATE INDEX BUFFER ---
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_TestLoadMesh, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_TestLoadMesh;
	delete[] p_inds_TestLoadMesh;
	// ----- TEST LOAD MESH -----
	// ---------- MESHES ----------

	// set type of topology to draw
	g_p_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ---------- CONSTANT BUFFERS ----------
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
	// ---------- CONSTANT BUFFERS ----------

	// ---------- SHADER RESOURCE VIEWS ----------
	// test Obj2Header mesh
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/heaventorch_diffuse.dds", nullptr, &g_p_texRV_TestHeaderMesh);
	// ---------- SHADER RESOURCE VIEWS ----------

	// ---------- SAMPLER STATES ----------
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_p_device->CreateSamplerState(&samplerDesc, &g_p_samplerLinear);
	// ---------- SAMPLER STATES ----------

	// ---------- MATRICES ----------
	// world
	XMStoreFloat4x4(&g_wrld, XMMatrixIdentity());
	// view
	XMVECTOR eye = XMVectorSet(0, 6, -10, 0);
	XMVECTOR at = XMVectorSet(0, 2, 0, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
	XMStoreFloat4x4(&g_view, XMMatrixInverse(&XMMatrixDeterminant(view), view));
	// projection
	XMStoreFloat4x4(&g_proj, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));

	// mesh world matrices
	XMStoreFloat4x4(&g_wrld_TestHardMesh, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_TestHeaderMesh, XMMatrixIdentity());
	// ---------- MATRICES ----------

	// ATTACH D3D TO WINDOW
	// --------------------------------------------------

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

void ProcessHeaderVerts(_OBJ_VERT_* _p_data, UINT _numVerts, S_VERTEX** _pp_verts)
{
	S_VERTEX* p_verts = new S_VERTEX[_numVerts];
	for (UINT i = 0; i < _numVerts; i++)
	{
		// retrieve position
		p_verts[i].pos.x = _p_data[i].pos[0];
		p_verts[i].pos.y = _p_data[i].pos[1];
		p_verts[i].pos.z = _p_data[i].pos[2];
		p_verts[i].pos.w = 1;
		// retrieve normals
		p_verts[i].norm.x = _p_data[i].nrm[0];
		p_verts[i].norm.y = _p_data[i].nrm[1];
		p_verts[i].norm.z = _p_data[i].nrm[2];
		// retrieve texcoords
		p_verts[i].tex.x = _p_data[i].uvw[0];
		p_verts[i].tex.y = _p_data[i].uvw[1];
		p_verts[i].tex.z = _p_data[i].uvw[2];
		// set color
		p_verts[i].color = XMFLOAT4(1, 1, 1, 1);
	}
	*_pp_verts = p_verts;
}

void CreateProceduralGrid(S_VERTEX _origin, UINT _numDivisions, FLOAT _scale,
	S_VERTEX** _pp_verts, UINT& _numVerts, UINT** _pp_inds, UINT& _numInds)
{
	// calculate number of verts / inds
	_numVerts = _numDivisions * _numDivisions;
	_numInds = 6 * (_numDivisions - 1) * (_numDivisions - 1);
	// set vertex data
	S_VERTEX* p_verts = new S_VERTEX[_numVerts];
	for (UINT z = 0; z < _numDivisions; z++)
		for (UINT x = 0; x < _numDivisions; x++)
		{
			UINT index = x + (z * _numDivisions);
			assert(index < _numVerts);
			// calculate offset amount
			FLOAT offsetX = (_scale * -0.5f) + (_scale / (_numDivisions - 1)) * x;
			FLOAT offsetZ = (_scale * -0.5f) + (_scale / (_numDivisions - 1)) * z;
			// offset position
			p_verts[index].pos = _origin.pos;
			p_verts[index].pos.x += offsetX;
			p_verts[index].pos.z += offsetZ;
			// copy normal
			p_verts[index].norm = { 0, 1, 0 };
			// offset tex coord
			p_verts[index].tex = _origin.tex;
			p_verts[index].tex.x += offsetX;
			p_verts[index].tex.y += offsetZ;
			// randomize color
			p_verts[index].color = {};
			p_verts[index].color.x = (rand() % 1000) / 1000.0f;
			p_verts[index].color.y = (rand() % 1000) / 1000.0f;
			p_verts[index].color.z = (rand() % 1000) / 1000.0f;
			p_verts[index].color.w = 1;
		}
	*_pp_verts = p_verts;
	// set indices
	UINT* p_inds = new UINT[_numInds];
	for (UINT z = 0; z < _numDivisions - 1; z++)
		for (UINT x = 0; x < _numDivisions - 1; x++)
		{
			UINT vertIndex = x + (z * _numDivisions);
			assert(vertIndex < _numVerts);
			UINT index = 6 * (x + (z * (_numDivisions - 1)));
			assert(index < _numInds);
			p_inds[index + 0] = vertIndex;
			p_inds[index + 1] = vertIndex + _numDivisions + 1;
			p_inds[index + 2] = vertIndex + 1;
			p_inds[index + 3] = vertIndex;
			p_inds[index + 4] = vertIndex + _numDivisions;
			p_inds[index + 5] = vertIndex + _numDivisions + 1;

			//_RPTN(0, "%d, %d, %d,\n%d, %d, %d\n\n", p_inds[index + 0], p_inds[index + 1], p_inds[index + 2], p_inds[index + 3], p_inds[index + 4], p_inds[index + 5]);
		}
	*_pp_inds = p_inds;
}

void ProcessOBJData(const char* _filepath, S_VERTEX** _pp_verts, UINT& _numVerts, UINT** _pp_inds, UINT& _numInds)
{
	S_OBJ_DATA data = LoadOBJData(_filepath);
	_numVerts = data.numVerts;
	_numInds = data.numInds;
	// copy vertex data
	S_VERTEX* verts = new S_VERTEX[_numVerts];
	for (UINT i = 0; i < _numVerts; i++)
	{
		// copy position
		verts[i].pos.x = data.vertices[i].pos[0];
		verts[i].pos.y = data.vertices[i].pos[1];
		verts[i].pos.z = data.vertices[i].pos[2];
		verts[i].pos.w = 1;
		// copy normal
		verts[i].norm.x = data.vertices[i].norm[0];
		verts[i].norm.y = data.vertices[i].norm[1];
		verts[i].norm.z = data.vertices[i].norm[2];
		// copy texcoord
		verts[i].tex.x = data.vertices[i].tex[0];
		verts[i].tex.y = data.vertices[i].tex[1];
		verts[i].tex.z = data.vertices[i].tex[2];
		// set color
		verts[i].color = { 1, 1, 1, 1 };
	}
	*_pp_verts = verts;
	// copy index data
	UINT* inds = new UINT[_numInds];
	for (UINT i = 0; i < _numInds; i++)
	{
		inds[i] = data.indices[i];
	}
	*_pp_inds = inds;
}

void Render()
{
	// --------------------------------------------------
	// UPDATES / DRAW SETUP

	// ----- UPDATE TIME -----
	static ULONGLONG timeStart = 0;
	static ULONGLONG timePrev = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	float t = (timeCur - timeStart) / 1000.0f;
	float dt = (timeCur - timePrev) / 1000.0f;
	timePrev = timeCur;
	// ----- UPDATE TIME -----

	// ----- GET WINDOW DIMENSIONS -----
	RECT windowRect;
	GetClientRect(g_hWnd, &windowRect);
	UINT windowWidth = windowRect.right - windowRect.left;
	UINT windowHeight = windowRect.bottom - windowRect.top;
	// ----- GET WINDOW DIMENSIONS -----

	// ----- SET BYTE STRIDES / OFFSETS -----
	UINT strides[] = { sizeof(S_VERTEX) };
	UINT offsets[] = { 0 };
	// ----- SET BYTE STRIDES / OFFSETS -----

	// ----- CREATE CONSTANT BUFFER STRUCT INSTANCES -----
	S_CBUFFER_VS cBufferVS = {};
	S_CBUFFER_PS cBufferPS = {};
	// ----- CREATE CONSTANT BUFFER STRUCT INSTANCES -----

	// ----- GENERAL PURPOSE VARS -----
	// color to clear render targets to
	FLOAT clearColor[4] = { 0, 0, 0.25f, 1 };
	// matrices
	XMMATRIX translate = XMMatrixIdentity();
	XMMATRIX rotate = XMMatrixIdentity();
	XMMATRIX scale = XMMatrixIdentity();
	// ----- GENERAL PURPOSE VARS -----

	// ----- RETRIEVE MATRICES -----
	XMMATRIX wrld = XMLoadFloat4x4(&g_wrld);
	XMMATRIX view = XMLoadFloat4x4(&g_view);
	XMMATRIX proj = XMLoadFloat4x4(&g_proj);
	XMMATRIX wrld_TestHardMesh = XMLoadFloat4x4(&g_wrld_TestHardMesh);
	XMMATRIX wrld_TestHeaderMesh = XMLoadFloat4x4(&g_wrld_TestHeaderMesh);
	XMMATRIX wrld_TestProcMesh = XMLoadFloat4x4(&g_wrld_TestProcMesh);
	XMMATRIX wrld_TestLoadMesh = XMLoadFloat4x4(&g_wrld_TestLoadMesh);
	// ----- RETRIEVE MATRICES -----

	// ----- LIGHTS -----
	// directional
#define LIGHTS_DIR 1
	S_LIGHT_DIR dLights[MAX_LIGHTS_DIR] =
	{
		// dir, color
		{ { 0, 1, 0, 0 }, { 0, 0, 1, 1 } },
		{},
		{}
	};
	// point
#define LIGHTS_PNT 1
	S_LIGHT_PNT pLights[MAX_LIGHTS_PNT] =
	{
		// pos, range, atten, color
		{ { 1, 0.5, 0, 1 }, 10, { 0, 0, 0.5f}, { 0, 1, 0, 1 } },
		{},
		{}
	};
	// spot
	S_LIGHT_SPT sLights[MAX_LIGHTS_SPT] = {};
	// ----- LIGHTS -----

	// ----- UPDATE WORLD POSITIONS -----
	// --- TEST HARDCODED MESH ---
	// orbit mesh around origin
	rotate = XMMatrixRotationY(0.5f * t);
	wrld_TestHardMesh = XMMatrixTranslation(5, 2, 0) * rotate;
	// --- TEST HARDCODED MESH ---
	// --- TEST OBJ2HEADER MESH ---
	rotate = XMMatrixRotationY(-0.3f * t);
	wrld_TestHeaderMesh = rotate * XMMatrixTranslation(2, 0, 0);
	// --- TEST OBJ2HEADER MESH ---
	// --- TEST PROCEDURAL MESH ---
	wrld_TestProcMesh = XMMatrixTranslation(0, -1, 0);
	// --- TEST PROCEDURAL MESH ---
	// --- LIGHTS ---
	// DLIGHT 0
	XMMATRIX lightMatrix = XMMatrixTranslation(dLights[0].dir.x, dLights[0].dir.y, dLights[0].dir.z);
	rotate = XMMatrixRotationZ(0.4f * t);
	XMStoreFloat4(&dLights[0].dir, (lightMatrix * rotate).r[3]);
	// PLIGHT 0
	lightMatrix = XMMatrixTranslation(pLights[0].pos.x, pLights[0].pos.y, pLights[0].pos.z);
	rotate = XMMatrixRotationY(0.7f * t);
	XMStoreFloat4(&pLights[0].pos, (lightMatrix * rotate).r[3]);
	// --- LIGHTS ---
	// ----- UPDATE WORLD POSITIONS -----

	// ----- HANDLE TOGGLES -----
	// camera
	static bool keyHeld_freelook = false;
	bool keyPressed_freelook = GetAsyncKeyState('C');
	if (!keyHeld_freelook && keyPressed_freelook) // toggle freelook
	{
		keyHeld_freelook = true;
		g_freelook = !g_freelook;
	}
	if (keyHeld_freelook && !keyPressed_freelook) // reset freelook held flag
	{
		keyHeld_freelook = false;
	}
	// vertex shader
	static bool keyHeld_defaultVS = false;
	bool keyPressed_defaultVS = GetAsyncKeyState('1');
	if (!keyHeld_defaultVS && keyPressed_defaultVS) // toggle defaultVS
	{
		keyHeld_defaultVS = true;
		g_defaultVS = !g_defaultVS;
	}
	if (keyHeld_defaultVS && !keyPressed_defaultVS) // reset defaultVS held flag
	{
		keyHeld_defaultVS = false;
	}
	// geometry shader
	static bool keyHeld_defaultGS = false;
	bool keyPressed_defaultGS = GetAsyncKeyState('2');
	if (!keyHeld_defaultGS && keyPressed_defaultGS) // toggle defaultGS
	{
		keyHeld_defaultGS = true;
		g_defaultGS = !g_defaultGS;
	}
	if (keyHeld_defaultGS && !keyPressed_defaultGS) // reset defaultGS held flag
	{
		keyHeld_defaultGS = false;
	}
	// pixel shader
	static bool keyHeld_defaultPS = false;
	bool keyPressed_defaultPS = GetAsyncKeyState('3');
	if (!keyHeld_defaultPS && keyPressed_defaultPS) // toggle defaultPS
	{
		keyHeld_defaultPS = true;
		g_defaultPS = !g_defaultPS;
	}
	if (keyHeld_defaultPS && !keyPressed_defaultPS) // reset defaultPS held flag
	{
		keyHeld_defaultPS = false;
	}
	// ----- HANDLE TOGGLES -----

	// ----- UPDATE CAMERA -----
	// -- POSITION --
	FLOAT x, y, z;
	x = y = z = 0.0f;
	if (GetAsyncKeyState('A'))			x -= g_camMoveSpeed * dt; // move left
	if (GetAsyncKeyState('D'))			x += g_camMoveSpeed * dt; // move right
	if (GetAsyncKeyState(VK_LSHIFT))	y -= g_camMoveSpeed * dt; // move down
	if (GetAsyncKeyState(VK_SPACE))		y += g_camMoveSpeed * dt; // move up
	if (GetAsyncKeyState('S'))			z -= g_camMoveSpeed * dt; // move backward
	if (GetAsyncKeyState('W'))			z += g_camMoveSpeed * dt; // move forward
	// apply offset
	view = (XMMatrixTranslation(x, 0, z) * view) * XMMatrixTranslation(0, y, 0);
	// -- POSITION --
	// -- ROTATION --
	FLOAT xr, yr;
	xr = yr = 0.0f;
	if (GetAsyncKeyState(VK_UP))	xr -= DEGTORAD(g_camRotSpeed) * dt; // rotate upward
	if (GetAsyncKeyState(VK_DOWN))	xr += DEGTORAD(g_camRotSpeed) * dt; // rotate downward
	if (GetAsyncKeyState(VK_LEFT))	yr -= DEGTORAD(g_camRotSpeed) * dt; // rotate left
	if (GetAsyncKeyState(VK_RIGHT))	yr += DEGTORAD(g_camRotSpeed) * dt; // rotate right
	// apply rotation
	XMVECTOR camPos = view.r[3];
	view = view * XMMatrixTranslationFromVector(-1 * camPos);
	view = XMMatrixRotationX(xr) * (view * XMMatrixRotationY(yr));
	view = view * XMMatrixTranslationFromVector(camPos);
	// -- ROTATION --
	// -- ZOOM --
	if (GetAsyncKeyState(VK_OEM_MINUS)) // zoom out
	{
		g_camZoom -= g_camMoveSpeed * dt;
		if (g_camZoom < g_camZoomMin)
			g_camZoom = g_camZoomMin;
	}
	if (GetAsyncKeyState(VK_OEM_PLUS)) // zoom in
	{
		g_camZoom += g_camMoveSpeed * dt;
		if (g_camZoom > g_camZoomMax)
			g_camZoom = g_camZoomMax;
	}
	// -- ZOOM --
	// -- NEAR / FAR PLANES --
	if (GetAsyncKeyState(VK_OEM_4)) // far plane closer
	{
		g_camFarPlane -= g_camFarSpeed * dt;
		if (g_camFarPlane < g_camFarMin)
			g_camFarPlane = g_camFarMin;
	}
	if (GetAsyncKeyState(VK_OEM_6)) // far plane farther
	{
		g_camFarPlane += g_camFarSpeed * dt;
		if (g_camFarPlane > g_camFarMax)
			g_camFarPlane = g_camFarMax;
	}
	if (GetAsyncKeyState(VK_OEM_1)) // near plane closer
	{
		g_camNearPlane -= g_camNearSpeed * dt;
		if (g_camNearPlane < g_camNearMin)
			g_camNearPlane = g_camNearMin;
	}
	if (GetAsyncKeyState(VK_OEM_7)) // near plane farther
	{
		g_camNearPlane += g_camNearSpeed * dt;
		if (g_camNearPlane > g_camNearMax)
			g_camNearPlane = g_camNearMax;
	}
	// -- NEAR / FAR PLANES --
	// reset camera
	if (GetAsyncKeyState(VK_BACK)) // reset zoom, near / far planes
	{
		g_camZoom = 1.0f;
		g_camNearPlane = 0.01f;
		g_camFarPlane = 100.0f;
	}
	// update projection matrix with current zoom level and near/far planes
	proj = XMMatrixPerspectiveFovLH(XM_PIDIV4 / g_camZoom, windowWidth / (FLOAT)windowHeight, g_camNearPlane, g_camFarPlane);
	// ----- UPDATE CAMERA -----

	// UPDATES / DRAW SETUP
	// --------------------------------------------------
	// DRAWING

	// ---------- FIRST RENDER PASS ----------
	// ----- RENDER PREP -----
	// clear render target view
	g_p_deviceContext->ClearRenderTargetView(g_p_renderTargetView, clearColor);
	// clear depth stencil view to 1.0 (max depth)
	g_p_deviceContext->ClearDepthStencilView(g_p_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// set viewport
	g_p_deviceContext->RSSetViewports(1, &g_viewport);
	// set render target view
	g_p_deviceContext->OMSetRenderTargets(1, &g_p_renderTargetView, g_p_depthStencilView);
	// set shader constant buffers
	g_p_deviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	g_p_deviceContext->PSSetConstantBuffers(1, 1, &g_p_cBufferPS);
	// ----- RENDER PREP -----

	// ----- PER-INSTANCE DATA -----
	XMMATRIX instanceOffsets[MAX_INSTANCES] = {};
	XMFLOAT4 instanceColors[MAX_INSTANCES] = {};
	// ----- PER-INSTANCE DATA -----

	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	if (g_freelook)
		cBufferVS.view = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	else
	{
		XMVECTOR eye = XMVectorSet(0, 10, -10, 1);
		XMVECTOR at = wrld_TestHardMesh.r[3];
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);
		cBufferVS.view = XMMatrixLookAtLH(eye, at, up);
	}
	cBufferVS.proj = proj;
	cBufferVS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 0.5f, 0, 0, 1 };
	cBufferPS.dLights[0] = dLights[0];
	cBufferPS.pLights[0] = pLights[0];
	cBufferPS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- TEST HARDCODED MESH -----
	// set vert/ind buffers
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_TestHardMesh, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_TestHardMesh, DXGI_FORMAT_R32_UINT, 0);
	// set VS constant buffer values
	cBufferVS.wrld = wrld_TestHardMesh;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	// set VS resources
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	// set PS constant buffer values
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// set PS resources
	g_p_deviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	// draw
	g_p_deviceContext->DrawIndexed(g_numInds_TestHardMesh, 0, 0);
	// ----- TEST HARDCODED MESH -----
	// ----- TEST OBJ2HEADER MESH -----
	// set vert/ind buffers
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_TestHeaderMesh, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_TestHeaderMesh, DXGI_FORMAT_R32_UINT, 0);
	// mesh instance offsets
	instanceOffsets[0] = XMMatrixTranslation(0, 0, 0);
	instanceOffsets[1] = XMMatrixTranslation(2, 0, 0);
	instanceOffsets[2] = XMMatrixTranslation(4, 0, 0);
	// set VS constant buffer values
	cBufferVS.wrld = wrld_TestHeaderMesh;
	cBufferVS.instanceOffsets[0] = instanceOffsets[0];
	cBufferVS.instanceOffsets[1] = instanceOffsets[1];
	cBufferVS.instanceOffsets[2] = instanceOffsets[2];
	// set VS resources
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	// set PS constant buffer values
	// set PS resources
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_texRV_TestHeaderMesh);
	g_p_deviceContext->PSSetSamplers(0, 1, &g_p_samplerLinear);
	if (g_defaultVS) g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);	// use default shader
	else g_p_deviceContext->VSSetShader(g_p_VS_Distort, 0, 0);		// use fancy shader
	if (g_defaultGS) g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);	// use default shader
	else g_p_deviceContext->GSSetShader(g_p_GS_Distort, 0, 0);		// use fancy shader
	if (g_defaultPS) g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);	// use default shader
	else g_p_deviceContext->PSSetShader(g_p_PS_Distort, 0, 0);		// use fancy shader
	// draw
	g_p_deviceContext->DrawIndexedInstanced(g_numInds_TestHeaderMesh, 3, 0, 0, 0);
	// ----- TEST OBJ2HEADER MESH -----
	// ----- TEST PROCEDURAL MESH -----
	// set vert/ind buffers
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_TestProcMesh, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_TestProcMesh, DXGI_FORMAT_R32_UINT, 0);
	// set VS constant buffer values
	cBufferVS.wrld = wrld_TestProcMesh;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	// set VS resources
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	// set GS resources
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	// set PS constant buffer values
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// set PS resources
	g_p_deviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	// draw
	g_p_deviceContext->DrawIndexed(g_numInds_TestProcMesh, 0, 0);
	// ----- TEST PROCEDURAL MESH -----
	// ----- TEST LOAD MESH -----
	// set vert/ind buffers
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_TestLoadMesh, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_TestLoadMesh, DXGI_FORMAT_R32_UINT, 0);
	// set VS constant buffer values
	cBufferVS.wrld = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	// set VS resources
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	// set GS resources
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	// set PS constant buffer values
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// set PS resources
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_texRV_TestHeaderMesh);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	// draw
	g_p_deviceContext->DrawIndexed(g_numInds_TestLoadMesh, 0, 0);
	// ----- TEST LOAD MESH -----
	// ----- VISUAL LIGHTS -----
	// set vert/ind buffers
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_TestHardMesh, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_TestHardMesh, DXGI_FORMAT_R32_UINT, 0);
	// clear offsets
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[1] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[2] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[3] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[4] = XMMatrixIdentity();
	// size scaling
	FLOAT sizeScale = 1.5f;
	// distance scaling
	FLOAT distScale = 1.0f;
	// --- DIRECTIONAL ---
	sizeScale = 2.0f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	distScale = 10.0f;
	for (UINT i = 0; i < LIGHTS_DIR; i++)
	{
		// set VS constant buffer values
		cBufferVS.wrld = scale * XMMatrixTranslation(distScale * dLights[i].dir.x,
			distScale * dLights[i].dir.y, distScale * dLights[i].dir.z);
		// set VS resources
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		// set PS constant buffer values
		cBufferPS.instanceColors[0] = dLights[i].color;
		// set PS resources
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		// draw
		g_p_deviceContext->DrawIndexed(g_numInds_TestHardMesh, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		// set VS constant buffer values
		cBufferVS.wrld = scale * XMMatrixTranslation(pLights[i].pos.x, pLights[i].pos.y, pLights[i].pos.z);
		// set VS resources
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		// set PS constant buffer values
		cBufferPS.instanceColors[0] = pLights[i].color;
		// set PS resources
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		// draw
		g_p_deviceContext->DrawIndexed(g_numInds_TestHardMesh, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- FIRST RENDER PASS -----------

	// present back buffer; change args to limit/sync framerate
	g_p_swapChain->Present(1, 0);

	// DRAWING
	// --------------------------------------------------
	// STORE VARS

	// ----- STORE MATRICES -----
	XMStoreFloat4x4(&g_wrld, wrld);
	XMStoreFloat4x4(&g_view, view);
	XMStoreFloat4x4(&g_proj, proj);
	XMStoreFloat4x4(&g_wrld_TestHardMesh, wrld_TestHardMesh);
	XMStoreFloat4x4(&g_wrld_TestHeaderMesh, wrld_TestHeaderMesh);
	XMStoreFloat4x4(&g_wrld_TestLoadMesh, wrld_TestLoadMesh);
	// ----- STORE MATRICES -----

	// STORE VARS
	// --------------------------------------------------
}

void Cleanup()
{
	// --- SHADERS ---
	if (g_p_PS_SolidColorLights) g_p_PS_SolidColorLights->Release();
	if (g_p_PS_SolidColor) g_p_PS_SolidColor->Release();
	if (g_p_PS_InputColorLights) g_p_PS_InputColorLights->Release();
	if (g_p_PS_InputColor) g_p_PS_InputColor->Release();
	if (g_p_PS_Distort) g_p_PS_Distort->Release();
	if (g_p_PS_CubeMap) g_p_PS_CubeMap->Release();
	if (g_p_PS) g_p_PS->Release();
	if (g_p_GS_Distort) g_p_GS_Distort->Release();
	if (g_p_GS) g_p_GS->Release();
	if (g_p_VS_Distort) g_p_VS_Distort->Release();
	if (g_p_VS) g_p_VS->Release();
	// --- SAMPLER STATES ---
	if (g_p_samplerLinear) g_p_samplerLinear->Release();
	// --- SHADER RESOURCE VIEWS ---
	if (g_p_texRV_TestHeaderMesh) g_p_texRV_TestHeaderMesh->Release();
	// --- CONSTANT BUFFERS ---
	if (g_p_cBufferPS) g_p_cBufferPS->Release();
	if (g_p_cBufferVS) g_p_cBufferVS->Release();
	// --- VERT / IND BUFFERS ---
	// test load mesh
	if (g_p_iBuffer_TestLoadMesh) g_p_iBuffer_TestLoadMesh->Release();
	if (g_p_vBuffer_TestLoadMesh) g_p_vBuffer_TestLoadMesh->Release();
	// test procedural mesh
	if (g_p_iBuffer_TestProcMesh) g_p_iBuffer_TestProcMesh->Release();
	if (g_p_vBuffer_TestProcMesh) g_p_vBuffer_TestProcMesh->Release();
	// test obj2header mesh
	if (g_p_iBuffer_TestHeaderMesh) g_p_iBuffer_TestHeaderMesh->Release();
	if (g_p_vBuffer_TestHeaderMesh) g_p_vBuffer_TestHeaderMesh->Release();
	// test hardcoded mesh
	if (g_p_iBuffer_TestHardMesh) g_p_iBuffer_TestHardMesh->Release();
	if (g_p_vBuffer_TestHardMesh) g_p_vBuffer_TestHardMesh->Release();
	// --- VERTEX LAYOUT ---
	if (g_p_vertexLayout) g_p_vertexLayout->Release();
	// --- DEPTH STENCILS ---
	if (g_p_depthStencil) g_p_depthStencil->Release();
	if (g_p_depthStencilView) g_p_depthStencilView->Release();
	// --- RENDER TARGET VIEWS ---
	if (g_p_renderTargetView) g_p_renderTargetView->Release();
	// --- DEVICE / SWAP CHAIN ---
	if (g_p_deviceContext) g_p_deviceContext->Release();
	if (g_p_swapChain) g_p_swapChain->Release();
	if (g_p_device) g_p_device->Release();
}
