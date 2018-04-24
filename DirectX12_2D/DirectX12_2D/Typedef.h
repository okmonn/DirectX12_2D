#pragma once
#include <DirectXMath.h>
#include <string>

// ウィンドウサイズ
#define WINDOW_X 640
#define WINDOW_Y 480

// 円周率
#define PI ((FLOAT)3.14159265359f)
// ラジアン変換
#define RAD(X) (X) * (PI / 180.0f)

// 空間行列
struct WVP
{
	//ワールド
	DirectX::XMMATRIX world;
	//ビュープロジェクション
	DirectX::XMMATRIX viewProjection;
};

// 頂点データ
struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 uv;
};

