#pragma once
#include "Window.h"
#include "Input.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <memory>
#include <vector>

class Device
{
	// コマンド周り
	struct Command
	{
		// コマンドアロケータ
		ID3D12CommandAllocator* allocator;
		// コマンドリスト
		ID3D12GraphicsCommandList* list;
		// コマンドキュー
		ID3D12CommandQueue* queue;
	};

	// スワップチェイン
	struct Swap
	{
		//インターフェースファクトリー
		IDXGIFactory4*		factory;
		//スワップチェイン
		IDXGISwapChain3*	swapChain;
		//バックバッファ数
		UINT				bufferCnt;
	};

public:
	// コンストラクタ
	Device(std::weak_ptr<Window>win, std::weak_ptr<Input>in);
	// デストラクタ
	~Device();

	// 初期化
	void Init(void);

	// 処理
	void UpData(void);
private:
	// デバイスの生成
	HRESULT CreateDevice(void);
	// コマンド周りの生成
	HRESULT CreateCommand(void);

	// ファクトリーの生成
	HRESULT CreateFactory(void);
	// スワップチェインの生成
	HRESULT CreateSwapChain(void);


	// ウィンドウクラス
	std::weak_ptr<Window>win;

	// インプットクラス
	std::weak_ptr<Input>in;

	// 参照結果
	HRESULT result;

	// 機能レベル
	D3D_FEATURE_LEVEL level;

	// デバイス
	ID3D12Device* dev;

	// コマンド
	Command com;

	// スワップチェイン
	Swap swap;
};