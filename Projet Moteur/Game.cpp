//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <DirectXMath.h>
#include "../DXUT/Core/DXUT.h"

extern void ExitGame();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

static const float ROTATION_GAIN = 0.004f;
static const float MOVEMENT_GAIN = 0.07f;
static const XMVECTORF32 START_POSITION = { 0.f, 2.f, 0.f, 0.f };
static const XMVECTORF32 ROOM_BOUNDS = { 30.f, 30.f, 30.f, 0.f };

Game::Game() :
    m_window(0),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1),
	m_pitch(0),
	m_yaw(0)
{
	m_cameraPos = START_POSITION.v;
}

void Game::CreateLoadRender3D(std::unique_ptr<DirectX::GeometricPrimitive> shape, std::wstring textureFile) {
	mv_shapes.push_back(std::move(shape));
	mv_textures.push_back(textureFile);
}

//Initialisation de direct3d et de ses resources
void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

    // TODO: Change the timer settings if want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);
}

// Boucle basique du jeu
void Game::Tick()
{
    m_timer.Tick([&]()
    {
		Update(m_timer);
    });

    Render();
}



// Mise à jour du monde
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // GAME LOGIC
	//float time = float(timer.GetTotalSeconds());
	//m_world = DirectX::SimpleMath::Matrix::CreateRotationZ(cosf(time)*2.f);
	
	//UTILISATION DE LA SOURIS
	auto mouse = m_mouse->GetState();
	if (mouse.positionMode == Mouse::MODE_RELATIVE) {
		DirectX::SimpleMath::Vector3 delta = DirectX::SimpleMath::Vector3(float(mouse.x), float(mouse.y), 0.f)*ROTATION_GAIN;
		m_pitch -= delta.y;
		m_yaw -= delta.x;

		float limit = XM_PI / 2.0f - 0.01f;
		m_pitch = std::max(-limit, m_pitch);
		m_pitch = std::min(+limit, m_pitch);

		if (m_yaw > XM_PI) {
			m_yaw -= XM_PI*2.0f;
		}
		else if (m_yaw < -XM_PI) {
			m_yaw += XM_PI * 2.0f;
		}
	}

	m_mouse->SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);

	auto kb = m_keyboard->GetState();
	if (kb.Escape)
		PostQuitMessage(0);

	if (kb.Home) {
		m_cameraPos = START_POSITION.v;
		m_pitch = m_yaw = 0;
	}

	DirectX::SimpleMath::Vector3 move = DirectX::SimpleMath::Vector3::Zero;

	/*
	if (kb.Up || kb.Z) 
		move.y += 0.1f;
	if (kb.Down || kb.S)
		move.y -= 0.1f;
	if (kb.Left || kb.Q)
		move.x += 0.1f;
	if (kb.Right || kb.D)
		move.x -=0.1f;
	if (kb.PageUp || kb.Space)
		move.z += 0.1f;
	if (kb.PageDown || kb.X)
		move.z -= 0.1f;
		*/

	if (kb.Up || kb.Z)
		move.z += 0.01f;
	if (kb.Down || kb.S)
		move.z -= 0.01f;
	if (kb.Left || kb.Q)
		move.x += 0.01f;
	if (kb.Right || kb.D)
		move.x -= 0.01f;
	if (kb.PageUp || kb.Space)
		move.y += 0.01f;
	if (kb.PageDown || kb.X)
		move.y -= 0.01f;


	DirectX::SimpleMath::Quaternion q = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.f);

	move = DirectX::SimpleMath::Vector3::Transform(move, q);
	move *= MOVEMENT_GAIN;

	m_cameraPos += move;

	DirectX::SimpleMath::Vector3 halfBound = (DirectX::SimpleMath::Vector3(ROOM_BOUNDS.v) / DirectX::SimpleMath::Vector3(2.f)) - DirectX::SimpleMath::Vector3(0.1f, 0.1f, 0.1f);

	m_cameraPos = DirectX::SimpleMath::Vector3::Min(m_cameraPos, halfBound);
	m_cameraPos = DirectX::SimpleMath::Vector3::Max(m_cameraPos, -halfBound);

    elapsedTime;
}

#include <string>

std::wstring Game::GetText() const {
	return L"";
}

// Ben sa render quoi
void Game::Render()
{
    // il ne faut pas essayer de render rien du tout !
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    //(TODO) RENDERING CODE
	//Camera
	float y = sinf(m_pitch);
	float r = cosf(m_pitch);
	float z = r*cosf(m_yaw);
	float x = r*sinf(m_yaw);

	XMVECTOR lookAt = m_cameraPos + DirectX::SimpleMath::Vector3(x, y, z);

	XMMATRIX view = XMMatrixLookAtRH(m_cameraPos, lookAt, DirectX::SimpleMath::Vector3::Up);



	//FPS
	
	m_fontSpriteBatch->Begin();
	std::wstring output = std::wstring(m_graphicInformations+L" FPS: "+std::to_wstring(m_timer.GetFramesPerSecond()));
	DirectX::SimpleMath::Vector2 origin = m_font->MeasureString(output.c_str()) / 1.f;
	m_font->DrawString(m_fontSpriteBatch.get(), output.c_str(), m_fontPos, Colors::White, 0.f, origin);
	m_fontSpriteBatch->End();

	//CUBE
	//m_shape->Draw(m_world, m_view, m_proj, Colors::White, m_cubeTexture.Get());

	//CUBES
	for (size_t i(0u); i < mv_shapes.size(); ++i) {
		mv_shapes[i]->Draw(m_world, view, m_proj, Colors::White, mv_internTextures[i].Get());
	}

	//ANTI-ALLIASING
	m_d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	m_d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
	m_d3dContext->RSSetState(m_states->CullNone());

	m_effect->Apply(m_d3dContext.Get());

	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	//IMAGE EXPLOSION
	/*
	m_spriteBatch->Begin();

	float time = float(m_timer.GetTotalSeconds());


	//Draw -> texture(.Get) ; position ; nullptr ; couleur de fond ; rotation ; origine [ ; scale ]
	m_spriteBatch->Draw(m_texture.Get(), m_screenPos, nullptr, Colors::White, 0.f, m_origin);
	m_spriteBatch->End();
	*/

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
	//Doc officiel
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(0, 0);

    // si la device a été reset, il faut reload l'engine
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Le jeux devient la fenêtre active
}

void Game::OnDeactivated()
{
    // TODO: Le jeux devien une feêntre en background
}

void Game::OnSuspending()
{
    // TODO: Le jeu et minimisé
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Le jeu revien de la minimisation
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Taille de la fenêtre par défaut (note minimum size is 320x200).
    width = 800;
    height = 600;
}

// Reouces dépendantes de la device
void Game::CreateDevice()
{

    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels (see code below related to 11.1 fallback handling).
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };


    // Création de l'API DX11 (->D3D11)
    HRESULT hr = D3D11CreateDevice(
        nullptr,                                // nullptr = adapteur graphique par défaut (carte graphique et non IGP)
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        m_d3dDevice.ReleaseAndGetAddressOf(),   // returns the Direct3D device created
        &m_featureLevel,                        // returns feature level of device created
        m_d3dContext.ReleaseAndGetAddressOf()   // returns the device immediate context
        );

    if (hr == E_INVALIDARG)
    {
        // DirectX 11.0 ne reconnaîtra pas D3D_FEATURE_LEVEL_11_1 donc on rééssai sans lui
        hr = D3D11CreateDevice(nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            &featureLevels[1],
            _countof(featureLevels) - 1,
            D3D11_SDK_VERSION,
            m_d3dDevice.ReleaseAndGetAddressOf(),
            &m_featureLevel,
            m_d3dContext.ReleaseAndGetAddressOf()
            );
    }

    DX::ThrowIfFailed(hr);

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(m_d3dDevice.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Ajout de messages d'ID
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    //Utilisation de DirectX 11.1 si il est installé
    if (SUCCEEDED(m_d3dDevice.As(&m_d3dDevice1)))
        (void)m_d3dContext.As(&m_d3dContext1);

	//LIGNES NON ESSENTIELS
	

	this->CreateLoadRender3D(DirectX::GeometricPrimitive::CreateCube(m_d3dContext.Get()), L"cobblestone.bmp");
	this->CreateLoadRender3D(DirectX::GeometricPrimitive::CreateCube(m_d3dContext.Get()), L"controller.png");

	// TODO: Initialize device dependent objects here (independent of window size).

	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());
	m_effect = std::make_unique<BasicEffect>(m_d3dDevice.Get());
	m_effect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;

	m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	DX::ThrowIfFailed(
		m_d3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
			VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			m_inputLayout.ReleaseAndGetAddressOf()));

	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_d3dContext.Get());


	//FPS
	m_font = std::make_unique<SpriteFont>(m_d3dDevice.Get(), L"FT_COURIER.spritefont");
	m_fontSpriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());

	//CUBE
	m_shape = DirectX::GeometricPrimitive::CreateCube(m_d3dContext.Get());
	m_world = DirectX::SimpleMath::Matrix::Identity;

	for (size_t i(0u); i < mv_shapes.size(); ++i) {
		mv_shapes[i] = DirectX::GeometricPrimitive::CreateCube(m_d3dContext.Get());
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tex;
		mv_internTextures.push_back(tex);
		DX::ThrowIfFailed(CreateWICTextureFromFile(m_d3dDevice.Get(), mv_textures[i].c_str(), nullptr, mv_internTextures[i].ReleaseAndGetAddressOf()));
	}

	//ANTI-ALLIASING
	CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
		D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE, FALSE);
	DX::ThrowIfFailed(m_d3dDevice->CreateRasterizerState(&rastDesc, m_raster.ReleaseAndGetAddressOf()));

	//IMAGES
	/*
	m_states = std::make_unique<CommonStates>(m_d3dDevice.Get());
	m_spriteBatch = std::make_unique<SpriteBatch>(m_d3dContext.Get());
	ComPtr<ID3D11Resource> resource;
	DX::ThrowIfFailed(CreateDDSTextureFromFile(m_d3dDevice.Get(), L"explo.DDS", resource.GetAddressOf(), m_texture.ReleaseAndGetAddressOf())); //TEST TEXTURE
	ComPtr<ID3D11Texture2D> stone;
	DX::ThrowIfFailed(resource.As(&stone));

	CD3D11_TEXTURE2D_DESC stoneDesc;
	stone->GetDesc(&stoneDesc);

	m_origin.x = float(stoneDesc.Width / 2);
	m_origin.y = float(stoneDesc.Height / 2);
	*/
}
// Allocation de la mémoire après un resize
void Game::CreateResources()
{

    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT backBufferCount = 2;

    // Si la swap chain existe déjà, on la resize sinon on la crée
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // Si la device est perdu, on la recrée avec une nouvelle swap chain
            OnDeviceLost();

            // OnDeviceLost() fait tout le boulot et nous ramène avant le if
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // Retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identification de l'adapteur graphique physique (le GPU quoi)
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

		//FPS / Recupération des informations graphiques pour les afficher
		DXGI_ADAPTER_DESC dxgiAdapterDesc;
		dxgiAdapter->GetDesc(&dxgiAdapterDesc);
		m_graphicInformations = L"Graphics :" + std::wstring(dxgiAdapterDesc.Description);

        // Obtention de la 'factory' qui l'a créé (GPU SOFT)
        ComPtr<IDXGIFactory1> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        ComPtr<IDXGIFactory2> dxgiFactory2;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
        {
            // DirectX 11.1 or later

            // Create a descriptor for the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
            swapChainDesc.Width = backBufferWidth;
            swapChainDesc.Height = backBufferHeight;
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.SampleDesc.Count = 4;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = backBufferCount;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
            fsSwapChainDesc.Windowed = TRUE;

            // Create a SwapChain from a Win32 window.
            DX::ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(
                m_d3dDevice.Get(),
                m_window,
                &swapChainDesc,
                &fsSwapChainDesc,
                nullptr,
                m_swapChain1.ReleaseAndGetAddressOf()
                ));

            DX::ThrowIfFailed(m_swapChain1.As(&m_swapChain));
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
            swapChainDesc.BufferCount = backBufferCount;
            swapChainDesc.BufferDesc.Width = backBufferWidth;
            swapChainDesc.BufferDesc.Height = backBufferHeight;
            swapChainDesc.BufferDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.OutputWindow = m_window;
            swapChainDesc.SampleDesc.Count = 4;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Windowed = TRUE;

            DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapChainDesc, m_swapChain.ReleaseAndGetAddressOf()));
        }

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtention du backbuffer, dernière étape du rendertarget3d
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Creation d'une vue sur le rendertarget (fini)
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocation d'un buffer 2D sur le depth/stencil buffer pour bind
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat,
		backBufferWidth, backBufferHeight,
		1, 1, D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0, 4, 0);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

	//LIGNES NON ESSENTIELS



    // TODO: Initialize windows-size dependent objects here.
	//FPS
	m_fontPos.x = backBufferWidth/1.1f;
	m_fontPos.y = backBufferHeight/15.f;

	//OTHER
	//CUBE
	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(DirectX::SimpleMath::Vector3(2.f, 2.f, 2.f), DirectX::SimpleMath::Vector3::Zero, DirectX::SimpleMath::Vector3::UnitY);
	m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(70.f), float(backBufferWidth) / float(backBufferHeight), 0.01f, 100.f);

	//IMAGE EXPLO
	/*
	m_screenPos.x = backBufferWidth / 2.f;
	m_screenPos.y = backBufferHeight / 2.f;
	*/
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
	/*
	m_texture.Reset();
	m_spriteBatch.reset();
	m_states.reset();
	*/

	//CUBE
	m_shape.reset();
	m_cubeTexture.Reset();

	for (size_t i(0u); i < mv_shapes.size(); ++i) {
		mv_shapes[i].reset();
		mv_internTextures[i].Reset();
	}



	//ANTI-ALLIAS
	m_raster.Reset();

	m_effect.reset();
	m_batch.reset();
	m_inputLayout.Reset();
	m_states.reset();

	//FPS
	m_font.reset();
	m_fontSpriteBatch.reset();


    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain1.Reset();
    m_swapChain.Reset();
    m_d3dContext1.Reset();
    m_d3dContext.Reset();
    m_d3dDevice1.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}