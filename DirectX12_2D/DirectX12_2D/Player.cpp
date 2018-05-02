#include "Player.h"

const FLOAT g = 9.8f;

// コンストラクタ
Player::Player(std::weak_ptr<Input>in, std::weak_ptr<Texture>tex) : in(in), tex(tex)
{
	//画像ID
	id = 0;

	//ヘッダー
	header = {};

	//画像データ
	data.clear();

	//分割データ
	cut.clear();

	index = 0;

	pos = {50.0f, 250.0f};

	vel = { 20.0f, -g * 4 };

	flam = 0;

	loop = true;

	reverse = false;

	Load("アクション/player.act");
}

// デストラクタ
Player::~Player()
{
}

// 読み込み
void Player::Load(std::string fileName)
{
	id = tex.lock()->SetID();

	FILE* file;

	//ファイル開らく
	if ((fopen_s(&file, fileName.c_str(), "rb")) != 0)
	{
		return;
	}

	fread(&header.pathNum, sizeof(header.pathNum), 1, file);

	header.path.resize(header.pathNum);
	fread(&header.path[0], header.pathNum, 1, file);

	fread(&header.dataNum, sizeof(header.dataNum), 1, file);

	ImageData dummy = {};
	for (int i = 0; i < header.dataNum; ++i)
	{
		fread(&dummy.nameNum, sizeof(dummy.nameNum), 1, file);

		dummy.name.resize(dummy.nameNum);
		fread(&dummy.name[0], dummy.nameNum, 1, file);

		fread(&dummy.loop, sizeof(dummy.loop), 1, file);

		fread(&dummy.animCnt, sizeof(dummy.animCnt), 1, file);

		data[dummy.name] = dummy;

		cut[dummy.name].resize(dummy.animCnt);
		for (int o = 0; o < dummy.animCnt; ++o)
		{
			fread(&cut[dummy.name][o], sizeof(cut[dummy.name][o]), 1, file);
		}

		dummy = {};
	}

	fclose(file);

	mode = "Walk";

	std::string path = "アクション/" + header.path;

	tex.lock()->LoadWIC(id, Texture::ChangeUnicode(path.c_str()));

	back = tex.lock()->SetID();
	tex.lock()->LoadWIC(back, Texture::ChangeUnicode("img/splatterhouse.png"));
}

// 描画
void Player::Draw(void)
{
	UpData();
	if (data[mode].loop)
	{
		index = (flam++ / cut[mode][index].flam) % cut[mode].size();
	}
	else
	{
		if (index < cut[mode].size() && loop == true)
		{
			index = (flam++ / cut[mode][index].flam) % cut[mode].size();

			if (flam >= cut[mode][index].flam * cut[mode].size())
			{
				loop = false;
			}
		}
	}

	tex.lock()->DrawRectWIC(id, {pos.x,pos.y},
		{ (FLOAT)(cut[mode][index].rect.GetRight() - cut[mode][index].rect.GetLeft()) * 2.0f, (FLOAT)(cut[mode][index].rect.GetBottom() - cut[mode][index].rect.GetTop()) * 2.0f },
		{ (FLOAT)cut[mode][index].rect.GetLeft() , (FLOAT)cut[mode][index].rect.GetTop() },
		{ (FLOAT)(cut[mode][index].rect.GetRight() - cut[mode][index].rect.GetLeft()), (FLOAT)(cut[mode][index].rect.GetBottom() - cut[mode][index].rect.GetTop()) }, reverse);

	tex.lock()->DrawWIC(back, { 0.0f, 0.0f }, { (FLOAT)WINDOW_X, (FLOAT)WINDOW_Y });
}

// 処理
void Player::UpData(void)
{
	if (mode == "Walk")
	{
		Walk();
	}
	else if (mode == "Jump")
	{
		Jump();
	}

	if (in.lock()->InputKey(DIK_RIGHT))
	{
		SetMode("Walk", false);
	}
	else if (in.lock()->InputKey(DIK_LEFT))
	{
		SetMode("Walk", true);
	}
	else if (in.lock()->InputKey(DIK_SPACE))
	{
		SetMode("Jump", reverse);
	}
}

// 状態の切り替え
void Player::SetMode(std::string m, bool r)
{
	if (mode == m && reverse == r)
	{
		return;
	}
	flam = 0;
	index = 0;
	mode = m;
	reverse = r;
	loop = true;
	if (reverse == false)
	{
		vel = { 20.0f, -g * 4 };
	}
	else
	{
		vel = { -20.0f, -g * 4 };
	}
}

// 歩きの処理
void Player::Walk(void)
{
	if (mode != "Walk")
	{
		return;
	}

	if (in.lock()->InputKey(DIK_RIGHT))
	{
		SetMode("Walk", false);
		pos.x += 1.0f;
	}
	else if (in.lock()->InputKey(DIK_LEFT))
	{
		SetMode("Walk", true);
		pos.x -= 1.0f;
	}
}

// ジャンプの処理
void Player::Jump(void)
{
	if (mode != "Jump")
	{
		return;
	}

	pos += vel;
	vel.y += g;

	if (pos.y >= 250.0f)
	{
		SetMode("Ground", reverse);
		pos.y = 250.0f;
	}
}

// 現在の状態の取得
std::string Player::GetMode(void)
{
	return mode;
}

// 反転フラグの取得
bool Player::GetReverse(void)
{
	return reverse;
}
