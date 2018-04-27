#pragma once
#include "Typedef.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <map>

class Texture
{
	//サイズ
	struct Size
	{
		LONG width;
		LONG height;
	};
	
	//BMPデータの構造体
	struct BMP
	{
		//画像サイズ
		Size					size;
		//bmpデータ
		std::vector<UCHAR>		data;
		//ヒープ
		ID3D12DescriptorHeap*	heap;
		//リソース
		ID3D12Resource*			resource;
	};

	//頂点データ
	struct VertexData
	{
		//頂点データ
		Vertex vertex[6];
		//リソース
		ID3D12Resource* resource;
		//送信データ
		UCHAR* data;
		// 頂点バッファビュー
		D3D12_VERTEX_BUFFER_VIEW vertexView;
	};

public:
	// デストラクタ
	~Texture();

	// インスタンス化
	static void Create(void);
	// 破棄
	static void Destroy(void);

	// インスタンス変数の取得
	static Texture* GetInstance(void)
	{
		return s_Instance;
	}

	// 読み込み
	HRESULT LoadBMP(USHORT index, std::string fileName, ID3D12Device* dev);

	// 描画準備
	void SetDraw(USHORT index, ID3D12GraphicsCommandList* list, UINT rootParamIndex);
	
	// 描画
	void Draw(USHORT index, FLOAT x, FLOAT y, ID3D12GraphicsCommandList * list, UINT rootParamIndex);

private:
	// コンストラクタ
	Texture();

	// 定数バッファ用のヒープの生成	
	HRESULT CreateConstantHeap(USHORT index, ID3D12Device* dev);
	// 定数バッファの生成
	HRESULT CreateConstant(USHORT index, ID3D12Device* dev);
	// シェーダリソースビューの生成
	HRESULT CreateShaderResourceView(USHORT index, ID3D12Device* dev);

	// 頂点リソースの生成
	HRESULT CreateVertex(USHORT index, ID3D12Device* dev);

	// 解放処理
	void Release(ID3D12Resource* resource);
	// 解放処理
	void Release(ID3D12DescriptorHeap* heap);


	// インスタンス変数
	static Texture* s_Instance;

	// 参照結果
	HRESULT result;

	// BMPデータ
	std::map<USHORT, BMP>bmp;

	// 頂点データ
	std::map<USHORT, VertexData>v;
};

