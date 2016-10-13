#include "Graphics.h"

Graphics::Graphics()
{
	mD3D = nullptr;
	mText = nullptr;
	mModel = nullptr;
	mLight = nullptr;
	mCamera = nullptr;
	mShader = nullptr;
}

Graphics::Graphics(const Graphics& other)
{
}

Graphics::~Graphics()
{
}

bool Graphics::Init(HWND hwnd, int width, int height)
{
	mD3D = new D3D;
	Check(mD3D->Init(hwnd, width, height, GRAPHICS_VSYNC_ENABLE, GRAPHICS_FULL_SCREEN, GRAPHICS_SCREEN_DEPTH, GRAPHICS_SCREEN_NEAR));

	mCamera = new Camera;
	mCamera->SetPosition(0.0f, 0.0f, -10.0f);

	mLight = new Light;
	mLight->SetAmbientColor(0.2f, 0.2f, 0.2f, 1.0f);
	mLight->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	mLight->SetDiffuseDir(0.0f, 0.0f, 1.0f);
	mLight->SetSpecularPower(32.0f);
	mLight->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);

	mModel = new Model;
	Check(mModel->Init(mD3D->GetDevice(), "Models/Cube.txt", L"Textures/flare.dds"));

	mCamera->Render();
	mText = new Text;
	mText->Init(mD3D->GetDevice(), mD3D->GetDeviceContext(), hwnd, width, height, "Fonts/fontdata.txt", L"Fonts/font.dds", 32, mCamera->GetViewMatrix());

	mShader = new Shader;
	Check(mShader->Init(hwnd, mD3D->GetDevice()));

	return true;
}

void Graphics::Shutdown()
{
	if (mShader)
	{
		mShader->Shutdown();
		SafeDelete(mShader);
	}

	if (mModel)
	{
		mModel->Shutdown();
		SafeDelete(mModel);
	}

	if (mText)
	{
		mText->Shutdown();
		SafeDelete(mText);
	}


	SafeDelete(mLight);
	SafeDelete(mCamera);

	if (mD3D)
	{
		mD3D->Shutdown();
		SafeDelete(mD3D);
	}
}

bool Graphics::Frame(float dt, int fps, int cpuRate)
{
	mText->SetFps(fps);
	mText->SetCpuRate(cpuRate);

	static float angle = 0.0f;
	angle += dt;
	if (angle >= 6.28f)
		angle = 0.0f;

	XMMATRIX rotate = XMMatrixRotationX(angle) * XMMatrixRotationY(angle) * XMMatrixRotationZ(angle);
	mD3D->SetWorldMatrix(rotate);

	return true;
}

bool Graphics::Render()
{
	mD3D->BeginScene(0.6f, 0.74f, 0.92f, 1.0f);

	mCamera->Render();

	mModel->Render(mD3D->GetDeviceContext());

	XMFLOAT4X4 world = mD3D->GetWorldMatrix();
	XMMATRIX scale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX offset = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMMATRIX newWorld = XMLoadFloat4x4(&world) * scale * offset;
	XMStoreFloat4x4(&world, newWorld);

	Check(mShader->Render(mD3D->GetDeviceContext(), mModel->GetIndexCount(), 
		world, mCamera->GetViewMatrix(), mD3D->GetProjMatrix(), mModel->GetTexture(),
		mLight->GetAmbientColor(), mLight->GetDiffuseColor(), mLight->GetDiffuseDir(),
		mLight->GetSpecularPower(), mLight->GetSpecularColor(), mCamera->GetPosition()));

	// Before rendering 2D objects, disable depth test and turn on blending.
	mD3D->TurnDepthOff();
	mD3D->TurnBlendOn();

	// Render fps and using rate of cpu.
	XMFLOAT4X4 I;
	XMStoreFloat4x4(&I, XMMatrixIdentity());
	mText->ShowPerformance(mD3D->GetDeviceContext(), I, mD3D->GetOrthoMatrix());

	// Render primary card name.
	char cardName[128];
	int cardMemory = 0;
	mD3D->GetVideoCardInfo(cardName, cardMemory);
	mText->Render(mD3D->GetDeviceContext(), cardName, 20, 60, XMFLOAT3(1.0f, 0.0f, 0.0f), I, mD3D->GetOrthoMatrix());

	// After rendering 2D objects, restore render states.
	mD3D->TurnBlendOff();
	mD3D->TurnDepthOn();

	mD3D->EndScene();

	return true;
}