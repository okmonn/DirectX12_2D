#pragma once
#include "Window.h"
#include "Input.h"
#include "Typedef.h"
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

	// レンダーターゲット
	struct RenderTarget
	{
		//ヒープ
		ID3D12DescriptorHeap* heap;
		//リソース
		std::vector<ID3D12Resource*>resource;
		//ヒープサイズ
		UINT size;
	};

	// 深度ステンシル
	struct DepthStencil
	{
		//ヒープ
		ID3D12DescriptorHeap* heap;
		//リソース
		ID3D12Resource* resource;
		//ヒープサイズ
		UINT size;
	};

	// フェンス
	struct Fence
	{
		//フェンス
		ID3D12Fence* fence;
		//フェンス値
		UINT64			fenceCnt;
		//フェンスイベント
		HANDLE			fenceEvent;
	};

	// ルートシグネチャ
	struct RootSignature
	{
		//メッセージ
		ID3DBlob*				signature;
		//エラーメッセージ
		ID3DBlob*				error;
		//ルートシグネチャ
		ID3D12RootSignature*	rootSignature;
	};

	// パイプライン
	struct PipeLine
	{
		//頂点データ
		ID3DBlob* vertex;
		//ピクセルデータ
		ID3DBlob* pixel;
		//パイプライン
		ID3D12PipelineState* pipeline;
	};

	// 定数バッファ
	struct Constant
	{
		//ヒープ
		ID3D12DescriptorHeap* heap;
		//リソース
		ID3D12Resource* resource;
		//ヒープサイズ
		UINT size;
		//送信データ
		UINT8* data;
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
	//ワールドビュープロジェクションのセット
	void SetWorldViewProjection(void);

	// デバイスの生成
	HRESULT CreateDevice(void);
	// コマンド周りの生成
	HRESULT CreateCommand(void);

	// ファクトリーの生成
	HRESULT CreateFactory(void);
	// スワップチェインの生成
	HRESULT CreateSwapChain(void);

	// レンダーターゲット用ヒープの生成
	HRESULT CreateRenderHeap(void);
	// レンダーターゲットの生成
	HRESULT CreateRenderTarget(void);

	// 深度ステンシル用ヒープの生成
	HRESULT CreateDepthHeap(void);
	// 深度ステンシルの生成
	HRESULT CreateDepthStencil(void);

	// フェンスの生成
	HRESULT CreateFence(void);

	// シグネチャのシリアライズ
	HRESULT Serialize(void);
	// ルートシグネチャの生成
	HRESULT CreateRootSigunature(void);

	// シェーダのコンパイル
	HRESULT ShaderCompile(LPCWSTR fileName);

	// パイプラインの生成
	HRESULT CreatePipeline(void);

	// 定数バッファ用ヒープの生成	
	HRESULT CreateConstantHeap(void);
	// 定数バッファの生成
	HRESULT CreateConstant(void);


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

	// レンダーターゲット
	RenderTarget render;

	// 深度ステンシル
	DepthStencil depth;

	// フェンス
	Fence fen;

	// ルートシグネチャ
	RootSignature sig;

	// パイプライン
	PipeLine pipe;

	// 定数バッファ
	Constant con;

	// WVP
	WVP wvp;
};