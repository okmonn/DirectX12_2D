#include "Texture.h"
#include "WICTextureLoader12.h"
#include <sstream>
#include <tchar.h>

#pragma comment (lib,"d3d12.lib")

// コンストラクタ
Texture::Texture(std::weak_ptr<Device>dev) : dev(dev)
{
	//WICの初期処理
	CoInitialize(nullptr);

	//参照結果の初期化
	result = S_OK;

	//起源
	origin.clear();

	//BMPデータ
	bmp.clear();

	//WICデータ
	wic.clear();


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
	//定数バッファ
	for (auto itr = bmp.begin(); itr != bmp.end(); ++itr)
	{
		Release(itr->second.v.resource);
		Release(itr->second.resource);
		Release(itr->second.heap);
	}
	for (auto itr = origin.begin(); itr != origin.end(); ++itr)
	{
		Release(itr->second.resource);
		Release(itr->second.heap);
	}
	for (auto itr = wic.begin(); itr != wic.end(); ++itr)
	{
		itr->second.decode.release();
		Release(itr->second.v.resource);
		Release(itr->second.resource);
		Release(itr->second.heap);
	}
}

// ユニコード変換
std::wstring Texture::ChangeUnicode(const CHAR * str)
{
	//文字数の取得
	auto byteSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str, -1, nullptr, 0);

	std::wstring wstr;
	wstr.resize(byteSize);

	//変換
	byteSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str, -1, &wstr[0], byteSize);

	return wstr;
}

// 読み込み
HRESULT Texture::LoadBMP(USHORT* index, std::string fileName)
{
	for (auto itr = origin.begin(); itr != origin.end(); ++itr)
	{
		if (itr->first == fileName)
		{
			bmp[index] = itr->second;
			result = CreateShaderResourceView(index, fileName);
			result = CreateVertex(index);

			return result;
		}
	}

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
	origin[fileName].size = { header.biWidth, header.biHeight };

	if (header.biBitCount == 24)
	{
		//データサイズ分のメモリ確保(ビットの深さが24bitの場合)
		origin[fileName].data.resize(header.biWidth * header.biHeight * 4);

		for (int line = header.biHeight - 1; line >= 0; --line)
		{
			for (int count = 0; count < header.biWidth * 4; count += 4)
			{
				//一番左の配列番号
				UINT address = line * header.biWidth * 4;
				origin[fileName].data[address + count] = 0;
				fread(&origin[fileName].data[address + count + 1], sizeof(UCHAR), 3, file);
			}
		}
	}
	else if (header.biBitCount == 32)
	{
		//データサイズ分のメモリ確保(ビットの深さが32bitの場合)
		origin[fileName].data.resize(header.biSizeImage);

		for (int line = header.biHeight - 1; line >= 0; --line)
		{
			for (int count = 0; count < header.biWidth * 4; count += 4)
			{
				//一番左の配列番号
				UINT address = line * header.biWidth * 4;
				fread(&origin[fileName].data[address + count], sizeof(UCHAR), 4, file);
			}
		}
	}

	//ファイルを閉じる
	fclose(file);

	bmp[index] = origin[fileName];

	result = CreateShaderResourceView(index, fileName);
	result = CreateVertex(index);

	return result;
}

// WIC読み込み
HRESULT Texture::LoadWIC(USHORT* index, std::wstring fileName)
{
	result = DirectX::LoadWICTextureFromFile(dev.lock()->GetDevice(), fileName.c_str(), &wic[index].resource, wic[index].decode, wic[index].sub);
	if (FAILED(result))
	{
		OutputDebugString(_T("\nWICテクスチャの読み込み：失敗\n"));
		return result;
	}

	result = CreateShaderResourceViewWIC(index);
	result = CreateVertexWIC(index);

	return result;
}

// 定数バッファ用のヒープの生成	
HRESULT Texture::CreateConstantHeap(USHORT* index, std::string fileName)
{
	//ヒープ設定用構造体の設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask			= 0;
	desc.NumDescriptors		= 2;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//ヒープ生成
	result = dev.lock()->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&origin[fileName].heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャの定数バッファ用ヒープの生成：失敗\n"));
		return result;
	}

	return result;
}

// 定数バッファの生成
HRESULT Texture::CreateConstant(USHORT* index, std::string fileName)
{
	if (CreateConstantHeap(index, fileName) != S_OK)
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
	desc.Width					= origin[fileName].size.width;
	desc.Height					= origin[fileName].size.height;
	desc.DepthOrArraySize		= 1;
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count		= 1;
	desc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout					= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;

	//リソース生成
	result = dev.lock()->GetDevice()->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&origin[fileName].resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャの定数バッファ用リソースの生成：失敗\n"));
		return result;
	}

	return result;
}

// シェーダリソースビューの生成
HRESULT Texture::CreateShaderResourceView(USHORT* index, std::string fileName)
{
	if (CreateConstant(index, fileName) != S_OK)
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
	D3D12_CPU_DESCRIPTOR_HANDLE handle = origin[fileName].heap->GetCPUDescriptorHandleForHeapStart();

	//シェーダーリソースビューの生成
	dev.lock()->GetDevice()->CreateShaderResourceView(origin[fileName].resource, &desc, handle);

	return S_OK;
}

// 定数バッファ用ヒープの生成
HRESULT Texture::CreateConstantHeapWIC(USHORT* index)
{
	//ヒープ設定用構造体の設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask			= 0;
	desc.NumDescriptors		= 2;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//ヒープ生成
	result = dev.lock()->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&wic[index].heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nWICテクスチャの定数バッファ用ヒープの生成：失敗\n"));
		return result;
	}

	return result;
}

// シェーダリソースビューの生成
HRESULT Texture::CreateShaderResourceViewWIC(USHORT* index)
{
	if (CreateConstantHeapWIC(index) != S_OK)
	{
		OutputDebugString(_T("\nWICテクスチャの定数バッファ用ヒープの生成：失敗\n"));
		return S_FALSE;
	}

	//シェーダリソースビュー設定用構造体の設定
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format						= wic[index].resource->GetDesc().Format;
	desc.ViewDimension				= D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels		= 1;
	desc.Texture2D.MostDetailedMip	= 0;
	desc.Shader4ComponentMapping	= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//ヒープの先頭ハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE handle = wic[index].heap->GetCPUDescriptorHandleForHeapStart();

	//シェーダーリソースビューの生成
	dev.lock()->GetDevice()->CreateShaderResourceView(wic[index].resource, &desc, handle);

	return S_OK;
}

// 頂点リソースの生成
HRESULT Texture::CreateVertex(USHORT* index)
{
	//配列のメモリ確保
	bmp[index].v.data = nullptr;

	for (UINT i = 0; i < 6; ++i)
	{
		bmp[index].v.vertex[i] = {};
	}

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
	desc.Width					= sizeof(bmp[index].v.vertex);//((sizeof(v[index].vertex) + 0xff) &~0xff);
	desc.Height					= 1;
	desc.DepthOrArraySize		= 1;
	desc.MipLevels				= 1;
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count		= 1;
	desc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout					= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点用リソース生成
	result = dev.lock()->GetDevice()->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&bmp[index].v.resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャの頂点バッファ用リソースの生成：失敗\n"));
		return result;
	}

	//送信範囲
	D3D12_RANGE range = { 0,0 };

	//マッピング
	result = bmp[index].v.resource->Map(0, &range, reinterpret_cast<void**>(&bmp[index].v.data));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n頂点用リソースのマッピング：失敗\n"));
		return result;
	}

	//頂点データのコピー
	memcpy(bmp[index].v.data, &bmp[index].v.vertex, sizeof(bmp[index].v.vertex));

	//アンマッピング
	//v[index].resource->Unmap(0, nullptr);

	//頂点バッファ設定用構造体の設定
	bmp[index].v.vertexView.BufferLocation  = bmp[index].v.resource->GetGPUVirtualAddress();
	bmp[index].v.vertexView.SizeInBytes	    = sizeof(bmp[index].v.vertex);
	bmp[index].v.vertexView.StrideInBytes   = sizeof(Vertex);

	return result;
}

// 頂点リソースの生成
HRESULT Texture::CreateVertexWIC(USHORT* index)
{
	//配列のメモリ確保
	wic[index].v.data = nullptr;

	for (UINT i = 0; i < 6; ++i)
	{
		wic[index].v.vertex[i] = {};
	}

	//ヒープ設定用構造体の設定
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	//リソース設定用構造体の設定
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = sizeof(wic[index].v.vertex);//((sizeof(v[index].vertex) + 0xff) &~0xff);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//頂点用リソース生成
	result = dev.lock()->GetDevice()->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&wic[index].v.resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nテクスチャの頂点バッファ用リソースの生成：失敗\n"));
		return result;
	}

	//送信範囲
	D3D12_RANGE range = { 0,0 };

	//マッピング
	result = wic[index].v.resource->Map(0, &range, reinterpret_cast<void**>(&wic[index].v.data));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nWIC頂点用リソースのマッピング：失敗\n"));
		return result;
	}

	//頂点データのコピー
	memcpy(wic[index].v.data, &wic[index].v.vertex, sizeof(wic[index].v.vertex));

	//頂点バッファ設定用構造体の設定
	wic[index].v.vertexView.BufferLocation = wic[index].v.resource->GetGPUVirtualAddress();
	wic[index].v.vertexView.SizeInBytes = sizeof(wic[index].v.vertex);
	wic[index].v.vertexView.StrideInBytes = sizeof(Vertex);

	return result;
}

// 描画準備
void Texture::SetDraw(USHORT* index)
{
	//頂点バッファビューのセット
	dev.lock()->GetComList()->IASetVertexBuffers(0, 1, &bmp[index].v.vertexView);

	//トポロジー設定
	dev.lock()->GetComList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
	dev.lock()->GetComList()->SetDescriptorHeaps(1, &bmp[index].heap);

	//ディスクラプターテーブルのセット
	dev.lock()->GetComList()->SetGraphicsRootDescriptorTable(1, handle);

	//描画
	dev.lock()->GetComList()->DrawInstanced(6, 1, 0, 0);
}

// 描画準備
void Texture::SetDrawWIC(USHORT* index)
{
	//頂点バッファビューのセット
	dev.lock()->GetComList()->IASetVertexBuffers(0, 1, &wic[index].v.vertexView);

	//トポロジー設定
	dev.lock()->GetComList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//リソース設定用構造体
	D3D12_RESOURCE_DESC desc = {};
	desc = wic[index].resource->GetDesc();

	//ボックス設定用構造体の設定
	D3D12_BOX box = {};
	box.back	= 1;
	box.bottom	= desc.Height;
	box.front	= 0;
	box.left	= 0;
	box.right	= (UINT)desc.Width;
	box.top		= 0;

	//サブリソースに書き込み
	result = wic[index].resource->WriteToSubresource(0, &box, wic[index].decode.get(), wic[index].sub.RowPitch, wic[index].sub.SlicePitch);
	if (FAILED(result))
	{
		OutputDebugString(_T("WICサブリソースへの書き込み：失敗\n"));
		return;
	}

	//ヒープの先頭ハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE handle = wic[index].heap->GetGPUDescriptorHandleForHeapStart();

	//ヒープのセット
	dev.lock()->GetComList()->SetDescriptorHeaps(1, &wic[index].heap);

	//ディスクラプターテーブルのセット
	dev.lock()->GetComList()->SetGraphicsRootDescriptorTable(1, handle);

	//描画
	dev.lock()->GetComList()->DrawInstanced(6, 1, 0, 0);
}

// 描画
void Texture::Draw(USHORT* index, Vector2<FLOAT>pos, Vector2<FLOAT>size)
{
	bmp[index].v.vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f}, {0.0f, 0.0f} };//左上
	bmp[index].v.vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f}, {1.0f, 0.0f} };//右上
	bmp[index].v.vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f}, {1.0f, 1.0f} };//右下
	bmp[index].v.vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f}, {1.0f, 1.0f} };//右下
	bmp[index].v.vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f}, {0.0f, 1.0f} };//左下
	bmp[index].v.vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f}, {0.0f, 0.0f} };//左上

	//頂点データのコピー
	memcpy(bmp[index].v.data, &bmp[index].v.vertex, (sizeof(bmp[index].v.vertex)));

	//頂点バッファ設定用構造体の設定
	bmp[index].v.vertexView.BufferLocation	= bmp[index].v.resource->GetGPUVirtualAddress();
	bmp[index].v.vertexView.SizeInBytes		= sizeof(bmp[index].v.vertex);
	bmp[index].v.vertexView.StrideInBytes	= sizeof(Vertex);
	
	SetDraw(index);
}

// 描画
void Texture::DrawWIC(USHORT* index, Vector2<FLOAT> pos, Vector2<FLOAT> size)
{
	wic[index].v.vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ 0.0f, 0.0f } };//左上
	wic[index].v.vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ 1.0f, 0.0f } };//右上
	wic[index].v.vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ 1.0f, 1.0f } };//右下
	wic[index].v.vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ 1.0f, 1.0f } };//右下
	wic[index].v.vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ 0.0f, 1.0f } };//左下
	wic[index].v.vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ 0.0f, 0.0f } };//左上

	//頂点データのコピー
	memcpy(wic[index].v.data, &wic[index].v.vertex, (sizeof(wic[index].v.vertex)));

	//頂点バッファ設定用構造体の設定
	wic[index].v.vertexView.BufferLocation	= wic[index].v.resource->GetGPUVirtualAddress();
	wic[index].v.vertexView.SizeInBytes		= sizeof(wic[index].v.vertex);
	wic[index].v.vertexView.StrideInBytes		= sizeof(Vertex);

	SetDrawWIC(index);
}

// 分割描画
void Texture::DrawRect(USHORT* index, Vector2<FLOAT> pos, Vector2<FLOAT> size, Vector2<FLOAT> rect, Vector2<FLOAT> rSize, bool turn)
{
	if (turn == false)
	{
		bmp[index].v.vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             rect.y / (FLOAT)bmp[index].size.height } };//左上
		bmp[index].v.vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, rect.y / (FLOAT)bmp[index].size.height } };//右上
		bmp[index].v.vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, (rect.y + (FLOAT)rSize.y) / bmp[index].size.height } };//右下
		bmp[index].v.vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, (rect.y + (FLOAT)rSize.y) / bmp[index].size.height } };//右下
		bmp[index].v.vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             (rect.y + rSize.y) / (FLOAT)bmp[index].size.height } };//左下
		bmp[index].v.vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             rect.y / (FLOAT)bmp[index].size.height } };//左上
	}
	else
	{
		bmp[index].v.vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, rect.y / (FLOAT)bmp[index].size.height } };//左上
		bmp[index].v.vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             rect.y / (FLOAT)bmp[index].size.height } };//右上
		bmp[index].v.vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             (rect.y + rSize.y) / (FLOAT)bmp[index].size.height } };//右下
		bmp[index].v.vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             (rect.y + rSize.y) / (FLOAT)bmp[index].size.height } };//右下
		bmp[index].v.vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, (rect.y + (FLOAT)rSize.y) / bmp[index].size.height } };//左下
		bmp[index].v.vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, rect.y / (FLOAT)bmp[index].size.height } };//左上
	}

	//頂点データのコピー
	memcpy(bmp[index].v.data, &bmp[index].v.vertex, (sizeof(bmp[index].v.vertex)));

	//頂点バッファ設定用構造体の設定
	bmp[index].v.vertexView.BufferLocation	= bmp[index].v.resource->GetGPUVirtualAddress();
	bmp[index].v.vertexView.SizeInBytes		= sizeof(bmp[index].v.vertex);
	bmp[index].v.vertexView.StrideInBytes	= sizeof(Vertex);

	//頂点バッファビューのセット
	dev.lock()->GetComList()->IASetVertexBuffers(0, 1, &bmp[index].v.vertexView);

	SetDraw(index);
}

// 分割描画
void Texture::DrawRectWIC(USHORT* index, Vector2<FLOAT> pos, Vector2<FLOAT> size, Vector2<FLOAT> rect, Vector2<FLOAT> rSize, bool turn)
{
	if (turn == false)
	{
		wic[index].v.vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//左上
		wic[index].v.vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//右上
		wic[index].v.vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//右下
		wic[index].v.vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//右下
		wic[index].v.vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//左下
		wic[index].v.vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//左上
	}
	else
	{
		wic[index].v.vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//左上
		wic[index].v.vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//右上
		wic[index].v.vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//右下
		wic[index].v.vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//右下
		wic[index].v.vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//左下
		wic[index].v.vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//左上
	}

	//頂点データのコピー
	memcpy(wic[index].v.data, &wic[index].v.vertex, (sizeof(wic[index].v.vertex)));

	//頂点バッファ設定用構造体の設定
	wic[index].v.vertexView.BufferLocation	= wic[index].v.resource->GetGPUVirtualAddress();
	wic[index].v.vertexView.SizeInBytes		= sizeof(wic[index].v.vertex);
	wic[index].v.vertexView.StrideInBytes		= sizeof(Vertex);

	//頂点バッファビューのセット
	dev.lock()->GetComList()->IASetVertexBuffers(0, 1, &wic[index].v.vertexView);

	SetDrawWIC(index);
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
