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

using namespace DirectX;
// ---------- INCLUDES ----------

// ---------- MACROS ----------
#define MAX_LOADSTRING 100

#define MAX_INSTANCES 5
#define MAX_LIGHTS_DIR 3
#define MAX_LIGHTS_PNT 3
#define MAX_LIGHTS_SPT 3

#define DEGTORAD(deg) (deg * (XM_PI / 180.0f))
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
ID3D11RenderTargetView*		g_p_renderTargetView_RTT = nullptr;			//released
// --- RENDER TARGET VIEWS ---
// --- DEPTH STENCILS ---
ID3D11Texture2D*			g_p_depthStencil = nullptr;					//released
ID3D11DepthStencilView*		g_p_depthStencilView = nullptr;				//released
ID3D11Texture2D*			g_p_depthStencil_RTT = nullptr;				//released
ID3D11DepthStencilView*		g_p_depthStencilView_RTT = nullptr;			//released
// --- DEPTH STENCILS ---
// --- VIEWPORTS ---
D3D11_VIEWPORT				g_viewport0;
D3D11_VIEWPORT				g_viewport1;
// --- VIEWPORTS ---
// --- INPUT LAYOUT ---
ID3D11InputLayout*			g_p_vertexLayout = nullptr;					//released
// --- INPUT LAYOUT ---
// --- VERT / IND BUFFERS ---
// SKYBOX
ID3D11Buffer*				g_p_vBuffer_Skybox = nullptr;				//released
ID3D11Buffer*				g_p_iBuffer_Skybox = nullptr;				//released
UINT						g_numVerts_Skybox = 0;
UINT						g_numInds_Skybox = 0;
// CUBE
ID3D11Buffer*				g_p_vBuffer_Cube = nullptr;					//released
ID3D11Buffer*				g_p_iBuffer_Cube = nullptr;					//released
UINT						g_numVerts_Cube = 0;
UINT						g_numInds_Cube = 0;
// GROUND PLANE
ID3D11Buffer*				g_p_vBuffer_GroundPlane = nullptr;			//released
ID3D11Buffer*				g_p_iBuffer_GroundPlane = nullptr;			//released
UINT						g_numVerts_GroundPlane = 0;
UINT						g_numInds_GroundPlane = 0;
UINT						g_numDivisions_GroundPlane = 100;
FLOAT						g_scale_GroundPlane = 10.0f;
// BRAZIER01
ID3D11Buffer*				g_p_vBuffer_Brazier01 = nullptr;			//released
ID3D11Buffer*				g_p_iBuffer_Brazier01 = nullptr;			//released
UINT						g_numVerts_Brazier01 = 0;
UINT						g_numInds_Brazier01 = 0;
// SPACESHIP
ID3D11Buffer*				g_p_vBuffer_Spaceship = nullptr;			//released
ID3D11Buffer*				g_p_iBuffer_Spaceship = nullptr;			//released
UINT						g_numVerts_Spaceship = 0;
UINT						g_numInds_Spaceship = 0;
// PLANET
ID3D11Buffer*				g_p_vBuffer_Planet = nullptr;				//released
ID3D11Buffer*				g_p_iBuffer_Planet = nullptr;				//released
UINT						g_numVerts_Planet = 0;
UINT						g_numInds_Planet = 0;
// --- VERT / IND BUFFERS ---
// --- CONSTANT BUFFERS ---
ID3D11Buffer*				g_p_cBufferVS = nullptr;					//released
ID3D11Buffer*				g_p_cBufferPS = nullptr;					//released
// --- CONSTANT BUFFERS ---
// --- TEXTURES / SHADER RESOURCE VIEWS ---
ID3D11ShaderResourceView*	g_p_SRV_Skybox = nullptr;					//released
ID3D11ShaderResourceView*	g_p_SRV_Brazier01 = nullptr;				//released
ID3D11ShaderResourceView*	g_p_SRV_Spaceship = nullptr;				//released
ID3D11ShaderResourceView*	g_p_SRV_Sun = nullptr;						//released
ID3D11ShaderResourceView*	g_p_SRV_Earth = nullptr;					//released
ID3D11ShaderResourceView*	g_p_SRV_Moon = nullptr;						//released
ID3D11ShaderResourceView*	g_p_SRV_Mars = nullptr;						//released
ID3D11ShaderResourceView*	g_p_SRV_Jupiter = nullptr;					//released
ID3D11Texture2D*			g_p_tex_RTT = nullptr;						//released
ID3D11ShaderResourceView*	g_p_SRV_RTT = nullptr;						//released
// --- TEXTURES / SHADER RESOURCE VIEWS ---
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
XMFLOAT4X4					g_proj_RTT;
XMFLOAT4X4					g_wrld_Skybox;
XMFLOAT4X4					g_wrld_Cube;
XMFLOAT4X4					g_wrld_GroundPlane;
XMFLOAT4X4					g_wrld_Brazier01;
XMFLOAT4X4					g_wrld_Spaceship;
XMFLOAT4X4					g_wrld_Sun;
XMFLOAT4X4					g_wrld_Earth;
XMFLOAT4X4					g_wrld_Moon;
XMFLOAT4X4					g_wrld_Mars;
XMFLOAT4X4					g_wrld_Jupiter;
// ----- MATRICES -----

// ----- CAMERA VALUES -----
FLOAT						g_camMoveSpeed = 4.0f;		// units per second
FLOAT						g_camRotSpeed = 40.0f;		// degrees per second
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
void				CreateProceduralGrid(S_VERTEX, UINT, FLOAT, S_VERTEX**, UINT&, UINT**, UINT&);
void				ProcessOBJData(const char*, S_VERTEX**, UINT&, UINT**, UINT&);
HRESULT				InitDepthStencilView(UINT, UINT, ID3D11Texture2D**, ID3D11DepthStencilView**);
HRESULT				InitVertexBuffer(UINT, S_VERTEX**, ID3D11Buffer**);
HRESULT				InitIndexBuffer(UINT, UINT**, ID3D11Buffer**);
HRESULT				InitConstantBuffer(UINT, ID3D11Buffer**);
HRESULT				InitSamplerState(ID3D11SamplerState**);
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
	// XOR with thickframe prevents click & drag resize, XOR with maximizebox prevents control button resize

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

	// ---------- SHADER RESOURCE VIEWS ----------
	// --- RTT ---
	// texture
	D3D11_TEXTURE2D_DESC texDesc_RTT = {};
	texDesc_RTT.Width = windowWidth;
	texDesc_RTT.Height = windowHeight;
	texDesc_RTT.MipLevels = 1;
	texDesc_RTT.ArraySize = 1;
	texDesc_RTT.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc_RTT.SampleDesc.Count = 1;
	texDesc_RTT.Usage = D3D11_USAGE_DEFAULT;
	texDesc_RTT.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc_RTT.CPUAccessFlags = 0;
	texDesc_RTT.MiscFlags = 0;
	hr = g_p_device->CreateTexture2D(&texDesc_RTT, NULL, &g_p_tex_RTT);
	// shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc_RTT;
	SRVDesc_RTT.Format = texDesc_RTT.Format;
	SRVDesc_RTT.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc_RTT.Texture2D.MostDetailedMip = 0;
	SRVDesc_RTT.Texture2D.MipLevels = 1;
	hr = g_p_device->CreateShaderResourceView(g_p_tex_RTT, &SRVDesc_RTT, &g_p_SRV_RTT);
	// --- RTT ---

	// skybox
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/skybox.dds", nullptr, &g_p_SRV_Skybox);

	// mesh textures
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/Brazier01map.dds", nullptr, &g_p_SRV_Brazier01);
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/spaceshipmap.dds", nullptr, &g_p_SRV_Spaceship);
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/sunmap.dds", nullptr, &g_p_SRV_Sun);
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/earthmap.dds", nullptr, &g_p_SRV_Earth);
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/moonmap.dds", nullptr, &g_p_SRV_Moon);
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/marsmap.dds", nullptr, &g_p_SRV_Mars);
	hr = CreateDDSTextureFromFile(g_p_device, L"Assets/jupitermap.dds", nullptr, &g_p_SRV_Jupiter);
	// ---------- SHADER RESOURCE VIEWS ----------

	// ---------- SAMPLER STATES ----------
	hr = InitSamplerState(&g_p_samplerLinear);
	// ---------- SAMPLER STATES ----------

	// ---------- RENDER TARGET VIEWS ----------
	// --- MAIN ---
	// get back buffer from swap chain
	ID3D11Resource* p_backBuffer = nullptr;
	hr = g_p_swapChain->GetBuffer(0, __uuidof(p_backBuffer), (void**)&p_backBuffer);
	// use back buffer to create render target view
	hr = g_p_device->CreateRenderTargetView(p_backBuffer, nullptr, &g_p_renderTargetView);
	// release back buffer
	p_backBuffer->Release();
	// --- MAIN ---
	// --- RTT ---
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc_RTT;
	RTVDesc_RTT.Format = texDesc_RTT.Format;
	RTVDesc_RTT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc_RTT.Texture2D.MipSlice = 0;
	hr = g_p_device->CreateRenderTargetView(g_p_tex_RTT, &RTVDesc_RTT, &g_p_renderTargetView_RTT);
	// --- RTT ---
	// ---------- RENDER TARGET VIEWS ----------

	// ---------- DEPTH STENCILS ----------
	hr = InitDepthStencilView(windowWidth, windowHeight, &g_p_depthStencil, &g_p_depthStencilView);
	hr = InitDepthStencilView(windowWidth, windowHeight, &g_p_depthStencil_RTT, &g_p_depthStencilView_RTT);
	// ---------- DEPTH STENCILS ----------

	// ---------- VIEWPORTS ----------
	// setup main viewport
	g_viewport0.Width = (FLOAT)windowWidth;
	g_viewport0.Height = (FLOAT)windowHeight;
	g_viewport0.TopLeftX = 0;
	g_viewport0.TopLeftX = 0;
	g_viewport0.MinDepth = 0.0f; // exponential depth; near/far planes are handled in projection matrix
	g_viewport0.MaxDepth = 1.0f;

	// setup minimap viewport
	g_viewport1.Width = (FLOAT)windowWidth / 4.0f;
	g_viewport1.Height = (FLOAT)windowHeight / 4.0f;
	g_viewport1.TopLeftX = 0;
	g_viewport1.TopLeftX = 0;
	g_viewport1.MinDepth = 0.0f; // exponential depth; near/far planes are handled in projection matrix
	g_viewport1.MaxDepth = 1.0f;
	// ---------- VIEWPORTS ----------

	// ---------- SHADERS ----------
	// vertex
	hr = g_p_device->CreateVertexShader(VS_Distort, sizeof(VS_Distort), nullptr, &g_p_VS_Distort);
	hr = g_p_device->CreateVertexShader(VS, sizeof(VS), nullptr, &g_p_VS);
	// geometry
	hr = g_p_device->CreateGeometryShader(GS, sizeof(GS), nullptr, &g_p_GS);
	hr = g_p_device->CreateGeometryShader(GS_Distort, sizeof(GS_Distort), nullptr, &g_p_GS_Distort);
	// pixel
	hr = g_p_device->CreatePixelShader(PS, sizeof(PS), nullptr, &g_p_PS);
	hr = g_p_device->CreatePixelShader(PS_CubeMap, sizeof(PS_CubeMap), nullptr, &g_p_PS_CubeMap);
	hr = g_p_device->CreatePixelShader(PS_Distort, sizeof(PS_Distort), nullptr, &g_p_PS_Distort);
	hr = g_p_device->CreatePixelShader(PS_InputColor, sizeof(PS_InputColor), nullptr, &g_p_PS_InputColor);
	hr = g_p_device->CreatePixelShader(PS_InputColorLights, sizeof(PS_InputColorLights), nullptr, &g_p_PS_InputColorLights);
	hr = g_p_device->CreatePixelShader(PS_SolidColor, sizeof(PS_SolidColor), nullptr, &g_p_PS_SolidColor);
	hr = g_p_device->CreatePixelShader(PS_SolidColorLights, sizeof(PS_SolidColorLights), nullptr, &g_p_PS_SolidColorLights);
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
	// ----- SKYBOX -----
	// load vertex / index data
	S_VERTEX* p_verts_Skybox = nullptr;
	UINT* p_inds_Skybox = nullptr;
	ProcessOBJData("Assets/skybox.obj", &p_verts_Skybox, g_numVerts_Skybox, &p_inds_Skybox, g_numInds_Skybox);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Skybox, &p_verts_Skybox, &g_p_vBuffer_Skybox);
	hr = InitIndexBuffer(g_numInds_Skybox, &p_inds_Skybox, &g_p_iBuffer_Skybox);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Skybox, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Skybox;
	delete[] p_inds_Skybox;
	// ----- SKYBOX -----

	// ----- CUBE -----
	// load vertex / index data
	S_VERTEX* p_verts_Cube = nullptr;
	UINT* p_inds_Cube = nullptr;
	ProcessOBJData("Assets/cube.obj", &p_verts_Cube, g_numVerts_Cube, &p_inds_Cube, g_numInds_Cube);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Cube, (S_VERTEX**)&p_verts_Cube, &g_p_vBuffer_Cube);
	hr = InitIndexBuffer(g_numInds_Cube, (UINT**)&p_inds_Cube, &g_p_iBuffer_Cube);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Cube, XMMatrixIdentity());
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	// generate vertex / index data
	S_VERTEX gridOrigin = { { 0, 0, 0, 1 }, { 0, 1, 0 }, { 0, 0, 0 }, {} };
	S_VERTEX* p_verts_GroundPlane = nullptr;
	UINT* p_inds_GroundPlane = nullptr;
	CreateProceduralGrid(gridOrigin, g_numDivisions_GroundPlane, g_scale_GroundPlane,
		&p_verts_GroundPlane, g_numVerts_GroundPlane, &p_inds_GroundPlane, g_numInds_GroundPlane);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_GroundPlane, &p_verts_GroundPlane, &g_p_vBuffer_GroundPlane);
	hr = InitIndexBuffer(g_numInds_GroundPlane, &p_inds_GroundPlane, &g_p_iBuffer_GroundPlane);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_GroundPlane, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_GroundPlane;
	delete[] p_inds_GroundPlane;
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	// load vertex / index data
	S_VERTEX* p_verts_Brazier01 = nullptr;
	UINT* p_inds_Brazier01 = nullptr;
	ProcessOBJData("Assets/Brazier01.obj", &p_verts_Brazier01, g_numVerts_Brazier01,
		&p_inds_Brazier01, g_numInds_Brazier01);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Brazier01, &p_verts_Brazier01, &g_p_vBuffer_Brazier01);
	hr = InitIndexBuffer(g_numInds_Brazier01, &p_inds_Brazier01, &g_p_iBuffer_Brazier01);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Brazier01, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Brazier01;
	delete[] p_inds_Brazier01;
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	// load vertex / index data
	S_VERTEX* p_verts_Spaceship = nullptr;
	UINT* p_inds_Spaceship = nullptr;
	ProcessOBJData("Assets/spaceship.obj", &p_verts_Spaceship, g_numVerts_Spaceship,
		&p_inds_Spaceship, g_numInds_Spaceship);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Spaceship, &p_verts_Spaceship, &g_p_vBuffer_Spaceship);
	hr = InitIndexBuffer(g_numInds_Spaceship, &p_inds_Spaceship, &g_p_iBuffer_Spaceship);
	// set initial world matrix
	XMStoreFloat4x4(&g_wrld_Spaceship, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Spaceship;
	delete[] p_inds_Spaceship;
	// ----- SPACESHIP -----

	// ----- PLANET -----
	// load vertex / index data
	S_VERTEX* p_verts_Planet = nullptr;
	UINT* p_inds_Planet = nullptr;
	ProcessOBJData("Assets/planet.obj", &p_verts_Planet, g_numVerts_Planet,
		&p_inds_Planet, g_numInds_Planet);
	// create vertex / index buffers
	hr = InitVertexBuffer(g_numVerts_Planet, &p_verts_Planet, &g_p_vBuffer_Planet);
	hr = InitIndexBuffer(g_numInds_Planet, &p_inds_Planet, &g_p_iBuffer_Planet);
	// set initial world matrices
	XMStoreFloat4x4(&g_wrld_Sun, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Earth, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Moon, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Mars, XMMatrixIdentity());
	XMStoreFloat4x4(&g_wrld_Jupiter, XMMatrixIdentity());
	// clear temp memory
	delete[] p_verts_Planet;
	delete[] p_inds_Planet;
	// ----- PLANET -----
	// ---------- MESHES ----------

	// set type of topology to draw
	g_p_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ---------- CONSTANT BUFFERS ----------
	hr = InitConstantBuffer(sizeof(S_CBUFFER_VS), &g_p_cBufferVS);
	hr = InitConstantBuffer(sizeof(S_CBUFFER_PS), &g_p_cBufferPS);
	// ---------- CONSTANT BUFFERS ----------

	// ---------- MATRICES ----------
	// world
	XMStoreFloat4x4(&g_wrld, XMMatrixIdentity());

	// view
	XMVECTOR eye = XMVectorSet(0, 10, -15, 0);
	XMVECTOR at = XMVectorSet(0, 2, 0, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
	XMStoreFloat4x4(&g_view, XMMatrixInverse(&XMMatrixDeterminant(view), view));

	// projection
	// main
	XMStoreFloat4x4(&g_proj, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
	// RTT
	XMStoreFloat4x4(&g_proj_RTT, XMMatrixPerspectiveFovLH(XM_PIDIV4, windowWidth / (FLOAT)windowHeight, 0.01f, 100.0f));
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

HRESULT InitDepthStencilView(UINT _width, UINT _height, ID3D11Texture2D** _pp_depthStencil,
	ID3D11DepthStencilView** _pp_depthStencilView)
{
	HRESULT hr;
	// create depth stencil texture
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = _width;
	depthStencilDesc.Height = _height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	hr = g_p_device->CreateTexture2D(&depthStencilDesc, nullptr, _pp_depthStencil);
	if (FAILED(hr))
		return hr;

	// create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	return g_p_device->CreateDepthStencilView(*_pp_depthStencil, &depthStencilViewDesc, _pp_depthStencilView);
}

HRESULT InitVertexBuffer(UINT _numVerts, S_VERTEX** _pp_verts, ID3D11Buffer** _pp_vBuffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(S_VERTEX) * _numVerts;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = *_pp_verts;
	return g_p_device->CreateBuffer(&bufferDesc, &subData, _pp_vBuffer);
}

HRESULT InitIndexBuffer(UINT _numInds, UINT** _pp_inds, ID3D11Buffer** _pp_iBuffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(UINT) * _numInds;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = *_pp_inds;
	return g_p_device->CreateBuffer(&bufferDesc, &subData, _pp_iBuffer);
}

HRESULT InitConstantBuffer(UINT _bufferSize, ID3D11Buffer** _pp_cBuffer)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = _bufferSize;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	return g_p_device->CreateBuffer(&bufferDesc, nullptr, _pp_cBuffer);
}

HRESULT InitSamplerState(ID3D11SamplerState** _pp_samplerState)
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	return g_p_device->CreateSamplerState(&samplerDesc, _pp_samplerState);
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
	XMMATRIX proj_RTT = XMLoadFloat4x4(&g_proj_RTT);
	XMMATRIX wrld_Skybox = XMLoadFloat4x4(&g_wrld_Skybox);
	XMMATRIX wrld_Cube = XMLoadFloat4x4(&g_wrld_Cube);
	XMMATRIX wrld_GroundPlane = XMLoadFloat4x4(&g_wrld_GroundPlane);
	XMMATRIX wrld_Brazier01 = XMLoadFloat4x4(&g_wrld_Brazier01);
	XMMATRIX wrld_Spaceship = XMLoadFloat4x4(&g_wrld_Spaceship);
	XMMATRIX wrld_Sun = XMLoadFloat4x4(&g_wrld_Sun);
	XMMATRIX wrld_Earth = XMLoadFloat4x4(&g_wrld_Earth);
	XMMATRIX wrld_Moon = XMLoadFloat4x4(&g_wrld_Moon);
	XMMATRIX wrld_Mars = XMLoadFloat4x4(&g_wrld_Mars);
	XMMATRIX wrld_Jupiter = XMLoadFloat4x4(&g_wrld_Jupiter);
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
		{ { 1.5f, 0.5, 0, 1 }, 10, { 0, 0, 0.5f}, { 0, 1, 0, 1 } },
		{},
		{}
	};
	// spot
	S_LIGHT_SPT sLights[MAX_LIGHTS_SPT] = {};
	// ----- LIGHTS -----

	// ----- UPDATE WORLD POSITIONS -----
	// --- CUBE ---
	// orbit about origin
	rotate = XMMatrixRotationY(0.5f * t);
	wrld_Cube = XMMatrixTranslation(2.5f, 6, 0) * rotate;
	// --- CUBE ---
	// --- GROUND PLANE ---
	wrld_GroundPlane = XMMatrixTranslation(0, -1, 0);
	// --- GROUND PLANE ---
	// --- SPACESHIP ---
	// orbit about origin
	rotate = XMMatrixRotationY(-1.3f * t);
	wrld_Spaceship = XMMatrixTranslation(5, 2, 0) * rotate;
	// --- SPACESHIP ---
	// --- SUN ---
	// spin
	scale = XMMatrixScaling(1.2f, 1.2f, 1.2f);
	rotate = XMMatrixRotationY(0.1f * t);
	wrld_Sun = scale * rotate * XMMatrixTranslation(0, 3, 0);
	// --- SUN ---
	// --- EARTH ---
	// orbit about origin
	scale = XMMatrixScaling(0.4f, 0.4f, 0.4f);
	rotate = XMMatrixRotationY(0.74f * t);
	wrld_Earth = scale * XMMatrixTranslation(3, 3, -2) * rotate;
	// spin
	rotate = XMMatrixRotationY(2.1f * t);
	wrld_Earth = rotate * wrld_Earth;
	// --- EARTH ---
	// --- MOON ---
	// orbit about earth
	scale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
	rotate = XMMatrixRotationY(0.4f * t);
	wrld_Moon = scale * XMMatrixTranslation(2, 0, 0) * rotate * wrld_Earth;
	// --- MOON ---
	// --- MARS ---
	// orbit about origin
	scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	rotate = XMMatrixRotationY(0.53f * t);
	wrld_Mars = scale * XMMatrixTranslation(4, 3, 3) * rotate;
	// spin
	rotate = XMMatrixRotationY(1.6f * t);
	wrld_Mars = rotate * wrld_Mars;
	// --- MARS ---
	// --- JUPITER ---
	// orbit about origin
	scale = XMMatrixScaling(0.7f, 0.7f, 0.7f);
	rotate = XMMatrixRotationY(0.31f * t);
	wrld_Jupiter = scale * XMMatrixTranslation(6, 3, -3) * rotate;
	// spin
	rotate = XMMatrixRotationY(1.2f * t);
	wrld_Jupiter = rotate * wrld_Jupiter;
	// --- JUPITER ---
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
	//view = (XMMatrixTranslation(x, 0, z) * view) * XMMatrixTranslation(0, y, 0);
	view = XMMatrixTranslation(x, y, z) * view;
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

	// ----- PER-INSTANCE DATA -----
	XMMATRIX instanceOffsets[MAX_INSTANCES] = {};
	XMFLOAT4 instanceColors[MAX_INSTANCES] = {};
	// ----- PER-INSTANCE DATA -----

	// UPDATES / DRAW SETUP
	// --------------------------------------------------
	// DRAWING

	// ---------- RENDER-TO-TEXTURE PASS -----------
	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	cBufferVS.view = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	cBufferVS.proj = proj_RTT;
	cBufferVS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 1, 1, 1, 1 };
	cBufferPS.dLights[0] = dLights[0];
	cBufferPS.pLights[0] = pLights[0];
	cBufferPS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	g_p_deviceContext->RSSetViewports(1, &g_viewport0);
	// set render target view
	g_p_deviceContext->OMSetRenderTargets(1, &g_p_renderTargetView_RTT, g_p_depthStencilView_RTT);
	// set shader constant buffers
	g_p_deviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	g_p_deviceContext->PSSetConstantBuffers(1, 1, &g_p_cBufferPS);
	// clear render target view
	g_p_deviceContext->ClearRenderTargetView(g_p_renderTargetView_RTT, clearColor);
	// clear depth stencil view to 1.0 (max depth)
	g_p_deviceContext->ClearDepthStencilView(g_p_depthStencilView_RTT, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Skybox, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = XMMatrixTranslationFromVector(view.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS_CubeMap, 0, 0);
	g_p_deviceContext->PSSetShaderResources(1, 1, &g_p_SRV_Skybox);
	g_p_deviceContext->PSSetSamplers(0, 1, &g_p_samplerLinear);
	g_p_deviceContext->DrawIndexed(g_numInds_Skybox, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	g_p_deviceContext->ClearDepthStencilView(g_p_depthStencilView_RTT, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_GroundPlane, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_GroundPlane, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	g_p_deviceContext->DrawIndexed(g_numInds_GroundPlane, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Brazier01, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Brazier01, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Brazier01);
	g_p_deviceContext->DrawIndexed(g_numInds_Brazier01, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Spaceship, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Spaceship, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Spaceship);
	g_p_deviceContext->DrawIndexed(g_numInds_Spaceship, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Sun);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Earth);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Moon);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Mars);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Jupiter);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
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
		cBufferVS.wrld = scale * XMMatrixTranslation(distScale * dLights[i].dir.x,
			distScale * dLights[i].dir.y, distScale * dLights[i].dir.z);
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.wrld = scale * XMMatrixTranslation(pLights[i].pos.x, pLights[i].pos.y, pLights[i].pos.z);
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- RENDER-TO-TEXTURE PASS -----------

	// ---------- MAIN RENDER PASS ----------
	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	if (g_freelook)
		cBufferVS.view = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	else
	{
		XMVECTOR eye = XMVectorSet(0, 10, -10, 1);
		XMVECTOR at = wrld_Cube.r[3];
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);
		cBufferVS.view = XMMatrixLookAtLH(eye, at, up);
	}
	cBufferVS.proj = proj;
	cBufferVS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 0.5f, 0.5f, 0.5f, 1 };
	cBufferPS.dLights[0] = dLights[0];
	cBufferPS.pLights[0] = pLights[0];
	cBufferPS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	g_p_deviceContext->RSSetViewports(1, &g_viewport0);
	// set render target view
	g_p_deviceContext->OMSetRenderTargets(1, &g_p_renderTargetView, g_p_depthStencilView);
	// set shader constant buffers
	g_p_deviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	g_p_deviceContext->PSSetConstantBuffers(1, 1, &g_p_cBufferPS);
	// clear render target view
	g_p_deviceContext->ClearRenderTargetView(g_p_renderTargetView, clearColor);
	// clear depth stencil view to 1.0 (max depth)
	g_p_deviceContext->ClearDepthStencilView(g_p_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Skybox, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = XMMatrixTranslationFromVector(view.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS_CubeMap, 0, 0);
	g_p_deviceContext->PSSetShaderResources(1, 1, &g_p_SRV_Skybox);
	g_p_deviceContext->PSSetSamplers(0, 1, &g_p_samplerLinear);
	g_p_deviceContext->DrawIndexed(g_numInds_Skybox, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	g_p_deviceContext->ClearDepthStencilView(g_p_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_RTT);
	g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_GroundPlane, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_GroundPlane, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	g_p_deviceContext->DrawIndexed(g_numInds_GroundPlane, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Brazier01, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Brazier01, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	if (g_defaultVS) g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);	// default shader
	else g_p_deviceContext->VSSetShader(g_p_VS_Distort, 0, 0);		// fancy shader
	if (g_defaultGS) g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);	// default shader
	else g_p_deviceContext->GSSetShader(g_p_GS_Distort, 0, 0);		// fancy shader
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	if (g_defaultPS) g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);	// default shader
	else g_p_deviceContext->PSSetShader(g_p_PS_Distort, 0, 0);		// fancy shader
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Brazier01);
	g_p_deviceContext->DrawIndexed(g_numInds_Brazier01, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Spaceship, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Spaceship, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Spaceship);
	g_p_deviceContext->DrawIndexed(g_numInds_Spaceship, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Sun);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Earth);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Moon);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Mars);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Jupiter);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[1] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[2] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[3] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[4] = XMMatrixIdentity();
	// size scaling
	sizeScale = 1.5f;
	// distance scaling
	distScale = 1.0f;
	// --- DIRECTIONAL ---
	sizeScale = 2.0f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	distScale = 10.0f;
	for (UINT i = 0; i < LIGHTS_DIR; i++)
	{
		cBufferVS.wrld = scale * XMMatrixTranslation(distScale * dLights[i].dir.x,
			distScale * dLights[i].dir.y, distScale * dLights[i].dir.z);
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.wrld = scale * XMMatrixTranslation(pLights[i].pos.x, pLights[i].pos.y, pLights[i].pos.z);
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- MAIN RENDER PASS -----------

	// ---------- MINIMAP RENDER PASS ----------
	// ----- SET SHARED CONSTANT BUFFER VALUES -----
	// vertex
	XMVECTOR eye = XMVectorSet(0, 15, 0, 1);
	XMVECTOR at = XMVectorSet(0, 0, 0, 1);
	XMVECTOR up = XMVectorSet(0, 0, 1, 0);
	XMMATRIX view1 = XMMatrixLookAtLH(eye, at, up);
	view1 = XMMatrixInverse(&XMMatrixDeterminant(view1), view1);
	cBufferVS.view = XMMatrixInverse(&XMMatrixDeterminant(view1), view1);
	cBufferVS.proj = proj;
	cBufferVS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);

	// pixel
	cBufferPS.ambientColor = { 0.5f, 0.5f, 0.5f, 1 };
	cBufferPS.dLights[0] = dLights[0];
	cBufferPS.pLights[0] = pLights[0];
	cBufferPS.t = t;
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	// ----- SET SHARED CONSTANT BUFFER VALUES -----

	// ----- RENDER PREP -----
	// set viewport
	g_p_deviceContext->RSSetViewports(1, &g_viewport1);
	// set render target view
	g_p_deviceContext->OMSetRenderTargets(1, &g_p_renderTargetView, g_p_depthStencilView);
	// set shader constant buffers
	g_p_deviceContext->VSSetConstantBuffers(0, 1, &g_p_cBufferVS);
	g_p_deviceContext->PSSetConstantBuffers(1, 1, &g_p_cBufferPS);
	// clear depth stencil view to 1.0 (max depth)
	g_p_deviceContext->ClearDepthStencilView(g_p_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- DRAW SKYBOX ---
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Skybox, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Skybox, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = XMMatrixTranslationFromVector(view1.r[3]);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS_CubeMap, 0, 0);
	g_p_deviceContext->PSSetShaderResources(1, 1, &g_p_SRV_Skybox);
	g_p_deviceContext->PSSetSamplers(0, 1, &g_p_samplerLinear);
	g_p_deviceContext->DrawIndexed(g_numInds_Skybox, 0, 0);
	// --- DRAW SKYBOX ---

	// re-clear depth stencil view
	g_p_deviceContext->ClearDepthStencilView(g_p_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	// ----- RENDER PREP -----

	// ----- CUBE -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Cube;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_RTT);
	g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	// ----- CUBE -----

	// ----- GROUND PLANE -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_GroundPlane, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_GroundPlane, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_GroundPlane;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS_InputColorLights, 0, 0);
	g_p_deviceContext->DrawIndexed(g_numInds_GroundPlane, 0, 0);
	// ----- GROUND PLANE -----

	// ----- BRAZIER01 -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Brazier01, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Brazier01, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = XMMatrixIdentity();
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Brazier01);
	g_p_deviceContext->DrawIndexed(g_numInds_Brazier01, 0, 0);
	// ----- BRAZIER01 -----

	// ----- SPACESHIP -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Spaceship, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Spaceship, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Spaceship;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Spaceship);
	g_p_deviceContext->DrawIndexed(g_numInds_Spaceship, 0, 0);
	// ----- SPACESHIP -----

	// ----- SUN -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Sun;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Sun);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- SUN -----

	// ----- EARTH -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Earth;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Earth);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- EARTH -----

	// ----- MOON -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Moon;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Moon);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MOON -----

	// ----- MARS -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Mars;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Mars);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- MARS -----

	// ----- JUPITER -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Planet, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Planet, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.wrld = wrld_Jupiter;
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
	g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
	g_p_deviceContext->GSSetShader(g_p_GS, 0, 0);
	cBufferPS.instanceColors[0] = { 0.1f, 0.1f, 0.1f, 1 };
	g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
	g_p_deviceContext->PSSetShader(g_p_PS, 0, 0);
	g_p_deviceContext->PSSetShaderResources(0, 1, &g_p_SRV_Jupiter);
	g_p_deviceContext->DrawIndexed(g_numInds_Planet, 0, 0);
	// ----- JUPITER -----

	// ----- VISUAL LIGHTS -----
	g_p_deviceContext->IASetVertexBuffers(0, 1, &g_p_vBuffer_Cube, strides, offsets);
	g_p_deviceContext->IASetIndexBuffer(g_p_iBuffer_Cube, DXGI_FORMAT_R32_UINT, 0);
	cBufferVS.instanceOffsets[0] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[1] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[2] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[3] = XMMatrixIdentity();
	cBufferVS.instanceOffsets[4] = XMMatrixIdentity();
	// size scaling
	sizeScale = 1.5f;
	// distance scaling
	distScale = 1.0f;
	// --- DIRECTIONAL ---
	sizeScale = 2.0f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	distScale = 10.0f;
	for (UINT i = 0; i < LIGHTS_DIR; i++)
	{
		cBufferVS.wrld = scale * XMMatrixTranslation(distScale * dLights[i].dir.x,
			distScale * dLights[i].dir.y, distScale * dLights[i].dir.z);
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = dLights[i].color;
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- DIRECTIONAL ---
	// --- POINT ---
	sizeScale = 0.25f;
	scale = XMMatrixScaling(sizeScale, sizeScale, sizeScale);
	for (UINT i = 0; i < LIGHTS_PNT; i++)
	{
		cBufferVS.wrld = scale * XMMatrixTranslation(pLights[i].pos.x, pLights[i].pos.y, pLights[i].pos.z);
		g_p_deviceContext->UpdateSubresource(g_p_cBufferVS, 0, nullptr, &cBufferVS, 0, 0);
		g_p_deviceContext->VSSetShader(g_p_VS, 0, 0);
		cBufferPS.instanceColors[0] = pLights[i].color;
		g_p_deviceContext->UpdateSubresource(g_p_cBufferPS, 0, nullptr, &cBufferPS, 0, 0);
		g_p_deviceContext->PSSetShader(g_p_PS_SolidColor, 0, 0);
		g_p_deviceContext->DrawIndexed(g_numInds_Cube, 0, 0);
	}
	// --- POINT ---
	// ----- VISUAL LIGHTS -----
	// ---------- MINIMAP RENDER PASS -----------

	// present back buffer; change args to limit/sync framerate
	g_p_swapChain->Present(1, 0);

	// DRAWING
	// --------------------------------------------------
	// STORE VARS

	// ----- STORE MATRICES -----
	XMStoreFloat4x4(&g_wrld, wrld);
	XMStoreFloat4x4(&g_view, view);
	XMStoreFloat4x4(&g_proj, proj);
	XMStoreFloat4x4(&g_proj_RTT, proj_RTT);
	XMStoreFloat4x4(&g_wrld_Skybox, wrld_Skybox);
	XMStoreFloat4x4(&g_wrld_Cube, wrld_Cube);
	XMStoreFloat4x4(&g_wrld_GroundPlane, wrld_GroundPlane);
	XMStoreFloat4x4(&g_wrld_Brazier01, wrld_Brazier01);
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
	if (g_p_SRV_RTT) g_p_SRV_RTT->Release();
	if (g_p_tex_RTT) g_p_tex_RTT->Release();
	if (g_p_SRV_Jupiter) g_p_SRV_Jupiter->Release();
	if (g_p_SRV_Mars) g_p_SRV_Mars->Release();
	if (g_p_SRV_Moon) g_p_SRV_Moon->Release();
	if (g_p_SRV_Earth) g_p_SRV_Earth->Release();
	if (g_p_SRV_Sun) g_p_SRV_Sun->Release();
	if (g_p_SRV_Spaceship) g_p_SRV_Spaceship->Release();
	if (g_p_SRV_Brazier01) g_p_SRV_Brazier01->Release();
	if (g_p_SRV_Skybox) g_p_SRV_Skybox->Release();
	// --- CONSTANT BUFFERS ---
	if (g_p_cBufferPS) g_p_cBufferPS->Release();
	if (g_p_cBufferVS) g_p_cBufferVS->Release();
	// --- VERT / IND BUFFERS ---
	// PLANET
	if (g_p_iBuffer_Planet) g_p_iBuffer_Planet->Release();
	if (g_p_vBuffer_Planet) g_p_vBuffer_Planet->Release();
	// SPACESHIP
	if (g_p_iBuffer_Spaceship) g_p_iBuffer_Spaceship->Release();
	if (g_p_vBuffer_Spaceship) g_p_vBuffer_Spaceship->Release();
	// BRAZIER01
	if (g_p_iBuffer_Brazier01) g_p_iBuffer_Brazier01->Release();
	if (g_p_vBuffer_Brazier01) g_p_vBuffer_Brazier01->Release();
	// GROUND PLANE
	if (g_p_iBuffer_GroundPlane) g_p_iBuffer_GroundPlane->Release();
	if (g_p_vBuffer_GroundPlane) g_p_vBuffer_GroundPlane->Release();
	// CUBE
	if (g_p_iBuffer_Cube) g_p_iBuffer_Cube->Release();
	if (g_p_vBuffer_Cube) g_p_vBuffer_Cube->Release();
	// SKYBOX
	if (g_p_iBuffer_Skybox) g_p_iBuffer_Skybox->Release();
	if (g_p_vBuffer_Skybox) g_p_vBuffer_Skybox->Release();
	// --- VERTEX LAYOUT ---
	if (g_p_vertexLayout) g_p_vertexLayout->Release();
	// --- DEPTH STENCILS ---
	if (g_p_depthStencilView_RTT) g_p_depthStencilView_RTT->Release();
	if (g_p_depthStencil_RTT) g_p_depthStencil_RTT->Release();
	if (g_p_depthStencilView) g_p_depthStencilView->Release();
	if (g_p_depthStencil) g_p_depthStencil->Release();
	// --- RENDER TARGET VIEWS ---
	if (g_p_renderTargetView_RTT) g_p_renderTargetView_RTT->Release();
	if (g_p_renderTargetView) g_p_renderTargetView->Release();
	// --- DEVICE / SWAP CHAIN ---
	if (g_p_deviceContext) g_p_deviceContext->Release();
	if (g_p_swapChain) g_p_swapChain->Release();
	if (g_p_device) g_p_device->Release();
}
