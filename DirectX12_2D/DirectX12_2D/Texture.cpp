#include "Texture.h"
#include <sstream>
#include <tchar.h>

#pragma comment (lib,"d3d12.lib")

Texture* Texture::s_Instance = nullptr;

// コンストラクタ
Texture::Texture()
{
	//WICの初期処理
	CoInitialize(nullptr);

	//参照結果の初期化
	result = S_OK;

	//BMPデータ
	bmp.clear();

	//頂点データ
	v.clear();


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
	//頂点データ
	for (auto itr = v.begin(); itr != v.end(); ++itr)
	{
		Release(itr->second.resource);
	}

	//定数バッファ
	for (auto itr = bmp.begin(); itr != bmp.end(); ++itr)
	{
		Release(itr->second.resource);
		Release(itr->second.heap);
	}
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
HRESULT Texture::LoadBMP(USHORT index, std::string fileName, ID3D12Device* dev)
{
	//BMPヘッダー構造体
	BITMAPINFOHEADER header = {};

	//BMPファイルヘッダー
	BITMAPFILEHEADER fileheader = {};

	//ファイル
	FILE* file;

	//ファイル開らく
	if ((fopen_s(&file, fileName.c_str(), "rb")) != 0)
	{
		//エラーナンバー確認
		auto a = (fopen_s(&file, fileName.c_str(), "rb"));
		std::stringstream s;
		s << a;
		OutputDebugString(_T("\nファイルを開けませんでした：失敗\n"));
		OutputDebugStringA(s.str().c_str());
		return S_FALSE;
	}

	//BMPファイルヘッダー読み込み
	fread(&fileheader, sizeof(fileheader), 1, file);

	//BMPヘッダー読み込み
	fread(&header, sizeof(header), 1, file);

	//画像の幅と高さの保存
	bmp[index].size = { header.biWidth, header.biHeight };

	//データサイズ分のメモリ確保(ビットの深さが32bitの場合)
	//bmp.resize(header.biSizeImage);
	//データサイズ分のメモリ確保(ビットの深さが24bitの場合)
	bmp[index].data.resize(header.biWidth * header.biHeight * 4);

	for (int line = header.biHeight - 1; line >= 0; --line)
	{
		for (int count = 0; count < header.biWidth * 4; count += 4)
		{
			//一番左の配列番号
			UINT address = line * header.biWidth * 4;
			bmp[index].data[address + count] = 0;
			fread(&bmp[index].data[address + count + 1], sizeof(UCHAR), 3, file);
		}
	}

	//ファイルを閉じる
	fclose(file);

	result = CreateShaderResourceView(index, dev);
	result = CreateVertex(index, dev);

	return result;
}

// 定数バッファ用のヒープの生成	
HRESULT Texture::CreateConstantHeap(USHORT index, ID3D12Device* dev)
{
	//ヒープ設定用構造体の設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask			= 0;
	desc.NumDescriptors		= 2;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//ヒープ生成
	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&bmp[index].heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャの定数バッファ用ヒープの生成：失敗\n"));
		return result;
	}

	return result;
}

// 定数バッファの生成
HRESULT Texture::CreateConstant(USHORT index, ID3D12Device* dev)
{
	if (CreateConstantHeap(index, dev) != S_OK)
	{
		OutputDebugString(_T("\nテクスチャの定数バッファ用ヒープの生成：失敗\n"));
		return S_FALSE;
	}

	//ヒープステート設定用構造体の設定
	D3D12_HEAP_PROPERTIES prop = {};
	prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	prop.CreationNodeMask		= 1;
	prop.MemoryPoolPreference	= D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0;
	prop.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_CUSTOM;
	prop.VisibleNodeMask		= 1;

	//リソース設定用構造体の設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension				= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width					= bmp[index].size.width;
	desc.Height					= bmp[index].size.height;
	desc.DepthOrArraySize		= 1;
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count		= 1;
	desc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout					= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;

	//リソース生成
	result = dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&bmp[index].resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャの定数バッファ用リソースの生成：失敗\n"));
		return result;
	}

	return result;
}

// シェーダリソースビューの生成
HRESULT Texture::CreateShaderResourceView(USHORT index, ID3D12Device * dev)
{
	if (CreateConstant(index, dev) != S_OK)
	{
		OutputDebugString(_T("\nテクスチャの定数バッファ用リソースの生成：失敗\n"));
		return S_FALSE;
	}

	//シェーダリソースビュー設定用構造体の設定
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format						= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension				= D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels		= 1;
	desc.Texture2D.MostDetailedMip	= 0;
	desc.Shader4ComponentMapping	= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//ヒープの先頭ハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE handle = bmp[index].heap->GetCPUDescriptorHandleForHeapStart();

	//シェーダーリソースビューの生成
	dev->CreateShaderResourceView(bmp[index].resource, &desc, handle);

	return S_OK;
}

// 頂点リソースの生成
HRESULT Texture::CreateVertex(USHORT index, ID3D12Device * dev)
{
	//配列のメモリ確保
	v[index].data = nullptr;

	v[index].vertex[0] = { { -1.0f / 2.0f,  1.0f / 2.0f, 0.0f },{ 0.0f, 0.0f } };//左上
	v[index].vertex[1] = { { 1.0f / 2.0f,  1.0f / 2.0f, 0.0f },{ 1.0f, 0.0f } };//右上
	v[index].vertex[2] = { { 1.0f / 2.0f, -1.0f / 2.0f, 0.0f },{ 1.0f, 1.0f } };//右下
	v[index].vertex[3] = { { 1.0f / 2.0f, -1.0f / 2.0f, 0.0f },{ 1.0f, 1.0f } };//右下
	v[index].vertex[4] = { { -1.0f / 2.0f, -1.0f / 2.0f, 0.0f },{ 0.0f, 1.0f } };//左下
	v[index].vertex[5] = { { -1.0f / 2.0f,  1.0f / 2.0f, 0.0f },{ 0.0f, 0.0f } };//左上

	//ヒープ設定用構造体の設定
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference	= D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask		= 1;
	prop.VisibleNodeMask		= 1;

	//リソース設定用構造体の設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension				= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width					= sizeof(v[index].vertex);//((sizeof(v[index].vertex) + 0xff) &~0xff);
	desc.Height					= 1;
	desc.DepthOrArraySize		= 1;
	desc.MipLevels				= 1;
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count		= 1;
	desc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout					= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点用リソース生成
	result = dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&v[index].resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャの頂点バッファ用リソースの生成：失敗\n"));
		return result;
	}

	//送信範囲
	D3D12_RANGE range = { 0,0 };

	//マッピング
	result = v[index].resource->Map(0, &range, reinterpret_cast<void**>(&v[index].data));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n頂点用リソースのマッピング：失敗\n"));
		return result;
	}

	//頂点データのコピー
	memcpy(v[index].data, &v[index].vertex, sizeof(v[index].vertex));

	//アンマッピング
	v[index].resource->Unmap(0, nullptr);

	//頂点バッファ設定用構造体の設定
	v[index].vertexView.BufferLocation  = v[index].resource->GetGPUVirtualAddress();
	v[index].vertexView.SizeInBytes	    = sizeof(v[index].vertex);
	v[index].vertexView.StrideInBytes   = sizeof(Vertex);

	return result;
}

// 描画準備
void Texture::SetDraw(USHORT index, ID3D12GraphicsCommandList* list, UINT rootParamIndex)
{
	//頂点バッファビューのセット
	list->IASetVertexBuffers(0, 1, &v[index].vertexView);

	//トポロジー設定
	list->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ボックス設定用構造体の設定
	D3D12_BOX box = {};
	box.back	= 1;
	box.bottom	= bmp[index].size.height;
	box.front	= 0;
	box.left	= 0;
	box.right	= bmp[index].size.width;
	box.top		= 0;

	//サブリソースに書き込み
	result = bmp[index].resource->WriteToSubresource(0, &box, &bmp[index].data[0], (box.right * 4), (box.bottom * 4));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャのサブリソースへの書き込み：失敗\n"));
		return;
	}

	//ヒープの先頭ハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE handle = bmp[index].heap->GetGPUDescriptorHandleForHeapStart();

	//ヒープのセット
	list->SetDescriptorHeaps(1, &bmp[index].heap);

	//ディスクラプターテーブルのセット
	list->SetGraphicsRootDescriptorTable(rootParamIndex, handle);

	//描画
	list->DrawInstanced(6, 1, 0, 0);
}

// 描画
void Texture::Draw(USHORT index, FLOAT x, FLOAT y, ID3D12GraphicsCommandList * list, UINT rootParamIndex)
{
	/*v[index].vertex[0] = { { -1.0f / 2.0f,  1.0f / 2.0f, 0.0f}, {0.0f, 0.0f} };//左上
	v[index].vertex[1] = { {  1.0f / 2.0f,  1.0f / 2.0f, 0.0f}, {1.0f, 0.0f} };//右上
	v[index].vertex[2] = { {  1.0f / 2.0f, -1.0f / 2.0f, 0.0f}, {1.0f, 1.0f} };//右下
	v[index].vertex[3] = { {  1.0f / 2.0f, -1.0f / 2.0f, 0.0f}, {1.0f, 1.0f} };//右下
	v[index].vertex[4] = { { -1.0f / 2.0f, -1.0f / 2.0f, 0.0f}, {0.0f, 1.0f} };//左下
	v[index].vertex[5] = { { -1.0f / 2.0f,  1.0f / 2.0f, 0.0f}, {0.0f, 0.0f} };//左上

	//頂点データのコピー
	memcpy(v[index].data, &v[index].vertex, (sizeof(v[index].vertex)));

	//頂点バッファ設定用構造体の設定
	v[index].vertexView.BufferLocation	= v[index].resource->GetGPUVirtualAddress();
	v[index].vertexView.SizeInBytes		= sizeof(v[index].vertex);
	v[index].vertexView.StrideInBytes	= sizeof(VERTEX);*/

	//頂点バッファビューのセット
	list->IASetVertexBuffers(0, 1, &v[index].vertexView);
	
	SetDraw(index, list, rootParamIndex);
}

// 解放処理
void Texture::Release(ID3D12Resource * resource)
{
	if (resource != nullptr)
	{
		resource->Release();
	}
}

// 解放処理
void Texture::Release(ID3D12DescriptorHeap * heap)
{
	if (heap != nullptr)
	{
		heap->Release();
	}
}
