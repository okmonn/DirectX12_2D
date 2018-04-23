#pragma once
#include <string>

// ウィンドウサイズ
#define WINDOW_X 640
#define WINDOW_Y 480

// 円周率
#define PI ((FLOAT)3.14159265359f)
// ラジアン変換
#define RAD(X) (X) * (PI / 180.0f)

//ビューモード
static std::string mode[] =
{
	"頂点",
	"深度",
	"定数",
};

