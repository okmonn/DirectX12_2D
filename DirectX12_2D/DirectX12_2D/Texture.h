#pragma once
#include <d3d12.h>
#include <string>
#include <vector>

class Texture
{
	//サイズ
	struct Size
	{
		LONG width;
		LONG height;
	};
	
	//BMPデータの構造体
	typedef struct
	{
		//画像サイズ
		Size				size;
		//bmpデータ
		std::vector<UCHAR>		bmp;
		//ヒープ
		ID3D12DescriptorHeap*	heap;
		//リソース
		ID3D12Resource*			resource;
	}Data;

public:
	// デストラクタ
	~Texture();

	// インスタンス化
	static void Create(void);
	// 破棄
	static void Destroy(void);

	// インスタンス変数の取得
	Texture* GetInstance(void)
	{
		return s_Instance;
	}

	// 読み込み
	HRESULT LoadBMP(std::string fileName);
private:
	// コンストラクタ
	Texture();

	// インスタンス変数
	static Texture* s_Instance;

	// 参照結果
	HRESULT result;
};

