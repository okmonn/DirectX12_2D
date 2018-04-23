#include "Texture.h"

#pragma comment (lib,"d3d12.lib")

Texture* Texture::s_Instance = nullptr;

// コンストラクタ
Texture::Texture()
{
	//参照結果の初期化
	result = S_OK;


	//エラーを出力に表示させる
#ifdef _DEBUG
	ID3D12Debug *debug = nullptr;
	result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	if (FAILED(result))
		int i = 0;
	debug->EnableDebugLayer();
	debug->Release();
	debug = nullptr;
#endif
}

// デストラクタ
Texture::~Texture()
{
}

// インスタンス化
void Texture::Create(void)
{
	if (s_Instance == nullptr)
	{
		s_Instance = new Texture;
	}
}

// 破棄
void Texture::Destroy(void)
{
	if (s_Instance != nullptr)
	{
		delete s_Instance;
	}
	s_Instance = nullptr;
}

// 読み込み
HRESULT Texture::LoadBMP(std::string fileName)
{
	return result;
}
