#include "Device.h"
#include <tchar.h>

// 機能レベル一覧
D3D_FEATURE_LEVEL levels[] = 
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

// コンストラクタ
Device::Device(std::weak_ptr<Window> win, std::weak_ptr<Input> in) : win(win), in(in)
{
	//参照結果
	result = S_OK;

	//機能レベル
	level = D3D_FEATURE_LEVEL_12_1;

	//デバイス
	dev = nullptr;
	
	//コマンド
	com = {};

	//スワップチェイン
	swap = {};


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


	Init();
}

// デストラクタ
Device::~Device()
{
	com.queue->Release();
	com.list->Release();
	com.allocator->Release();
	dev->Release();
}

// 初期化
void Device::Init(void)
{
	CreateCommand();
}

// 処理
void Device::UpData(void)
{
}

// デバイスの生成
HRESULT Device::CreateDevice(void)
{
	for (auto& i : levels)
	{
		result = D3D12CreateDevice(nullptr, i, IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			level = i;

			break;
		}
	}

	return result;
}

// コマンド周りの生成
HRESULT Device::CreateCommand(void)
{
	if (CreateDevice() != S_OK)
	{
		OutputDebugString(_T("\nデバイスの生成：失敗\n"));
		return S_FALSE;
	}

	//コマンドアロケータの生成
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&com.allocator));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nコマンドアロケータの生成：失敗\n"));
		return result;
	}

	//コマンドリストの生成
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, com.allocator, nullptr, IID_PPV_ARGS(&com.list));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nコマンドリストの生成：失敗\n"));
		return result;
	}

	//コマンドキュー設定用構造体
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask	= 0;
	desc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;

	//コマンドキューの生成
	result = dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&com.queue));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nコマンドキューの生成：失敗"));
		return result;
	}

	return result;
}

// ファクトリーの生成
HRESULT Device::CreateFactory(void)
{
	result = CreateDXGIFactory1(IID_PPV_ARGS(&swap.factory));

	return result;
}

// スワップチェインの生成
HRESULT Device::CreateSwapChain(void)
{
	if (CreateFactory() != S_OK)
	{
		OutputDebugString(_T("\nファクトリーの生成：失敗\n"));
		return S_FALSE;
	}
}
