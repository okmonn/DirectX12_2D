#pragma once
#include "Input.h"
#include "Texture.h"

class Player
{
	// 二次元座標
	template<typename T>
	struct Vector2
	{
		Vector2() : x(0), y(0) {};
		Vector2(T x, T y) : x(x), y(y) {};

		T x;
		T y;

		Vector2 operator+(const Vector2& vec) const
		{
			Vector2<T> tmp;
			tmp.x = x + vec.x;
			tmp.y = y + vec.y;
			return tmp;
		}

		void operator+=(const T a)
		{
			x += a;
			y += a;
		}

		void operator+=(const Vector2& vec)
		{
			x += vec.x;
			y += vec.y;
		}
	};

	typedef Vector2<int>Position;

	struct Rect
	{
		//中心座標
		Position pos;

		//幅
		Position size;

		Rect() : pos({ 0,0 }), size({ 0,0 })
		{}
		Rect(int x, int y, int w, int h) : pos({ x, y }), size({ w,h })
		{}
		Rect(Position& pos, int w, int h) : pos(pos), size({ w,h })
		{}
		//中心位置の更新
		void SetCenter(int x, int y)
		{
			pos = { x, y };
		}
		void SetCenter(Position& pos)
		{
			this->pos = pos;
		}

		//
		void SetLeftTopWidthHeight(int x, int y, int w, int h)
		{
			size = { w, h };
			pos = { (x + size.x / 2), (y + size.y / 2) };
		}

		//区切り座標の取得
		Position GetPos(void)
		{
			return pos;
		}

		//左座標の取得
		int GetLeft(void)
		{
			return pos.x - size.x / 2;
		}
		//上座標の取得
		int GetTop(void)
		{
			return pos.y - size.y / 2;
		}
		//右座標の取得
		int GetRight(void)
		{
			return pos.x + size.x / 2;
		}
		//下座標の取得
		int GetBottom(void)
		{
			return pos.y + size.y / 2;
		}
		//横幅の取得
		int GetWidth(void)
		{
			return size.x;
		}
		//縦幅の取得
		int GetHeight(void)
		{
			return size.y;
		}
	};

	// 画像ヘッダー構造体
	struct ImageHeader
	{
		//文字列の長さ
		int pathNum;
		//ファイルパス
		std::string path;
		//データ数
		int dataNum;
	};
	// 画像データ構造体
	struct ImageData
	{
		//文字列の長さ
		int nameNum;
		//アクション名
		std::string name;
		//ループフラグ
		char loop;
		//アニメーション数
		int animCnt;
	};
	//分割データ
	struct CutData
	{
		//切り抜き短形
		Rect rect;
		//中心座標
		Position center;
		//アニメーション時間
		int flam;
	};

	struct Pos
	{
		FLOAT x;
		FLOAT y;

		void operator+=(const Pos& p)
		{
			x += p.x;
			y += p.y;
		}
	};

public:
	// コンストラクタ
	Player(std::weak_ptr<Input>in, std::weak_ptr<Texture>tex);
	// デストラクタ
	~Player();

	// 読み込み
	void Load(std::string fileName);
	
	// 描画
	void Draw(void);

	// 処理
	void UpData(void);

	// 状態の切り替え
	void SetMode(std::string m, bool r);

	// 現在の状態の取得
	std::string GetMode(void);
	// 反転フラグの取得
	bool GetReverse(void);

private:
	// 歩きの処理
	void Walk(void);
	// ジャンプの処理
	void Jump(void);



	// インプット
	std::weak_ptr<Input>in;

	// テクスチャ
	std::weak_ptr<Texture>tex;

	// 画像ID
	USHORT id;

	USHORT back;

	// ヘッダー
	ImageHeader header;
	// 画像データ
	std::map<std::string, ImageData>data;
	// 分割データ
	std::map<std::string, std::vector<CutData>>cut;

	// 状態
	std::string mode;

	// 座標
	Pos pos;

	// 力
	Pos vel;

	// インデックス
	UINT index;

	// フレーム
	UINT flam;

	// ループフラグ
	bool loop;

	// 反転フラグ
	bool reverse;
};

