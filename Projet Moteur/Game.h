//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include <SpriteBatch.h>
#include <SimpleMath.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include <GeometricPrimitive.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <vector>
#include "../DXUT/Optional/SDKmisc.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

	void CreateLoadRender3D(std::unique_ptr<DirectX::GeometricPrimitive> shape, std::wstring textureFile);
	std::wstring GetText() const;
	

    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice1;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext1;

    Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;
    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain1;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

	//LIGNES NON ESSENTIELS	
	/*
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	DirectX::SimpleMath::Vector2 m_screenPos;
	DirectX::SimpleMath::Vector2 m_origin;
	std::unique_ptr<DirectX::CommonStates> m_states;
	*/

	//GLOBAL 3D RENDER
	DirectX::SimpleMath::Matrix m_world;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster;

	std::unique_ptr<DirectX::BasicEffect> m_effect;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	std::unique_ptr<DirectX::CommonStates> m_states;

	//Souris
	std::unique_ptr<DirectX::Mouse> m_mouse;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;

	float m_pitch;
	float m_yaw;

	//Camera
	DirectX::SimpleMath::Vector3 m_cameraPos;

	//1 CUBE
	std::unique_ptr<DirectX::GeometricPrimitive> m_shape;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeTexture;

	//CUBES
	std::vector<std::wstring> mv_textures;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> mv_internTextures;
	std::vector<std::unique_ptr<DirectX::GeometricPrimitive>> mv_shapes;

	//FPS
	std::unique_ptr<DirectX::SpriteFont> m_font;
	DirectX::SimpleMath::Vector2 m_fontPos;
	std::unique_ptr<DirectX::SpriteBatch> m_fontSpriteBatch;
	std::wstring m_graphicInformations;

	//Physique
	bool intersect(std::unique_ptr<DirectX::GeometricPrimitive> g1, std::unique_ptr<DirectX::GeometricPrimitive> g2) {
		
	}


    // Rendering loop timer.
    DX::StepTimer                                   m_timer;
};