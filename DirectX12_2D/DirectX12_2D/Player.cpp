#include "Player.h"

const FLOAT g = 9.8f;

// �R���X�g���N�^
Player::Player(std::weak_ptr<Input>in, std::weak_ptr<Texture>tex) : in(in), tex(tex)
{
	//�摜ID
	id = 0;

	//�w�b�_�[
	header = {};

	//�摜�f�[�^
	data.clear();

	//�����f�[�^
	cut.clear();

	index = 0;

	pos = {50.0f, 250.0f};

	vel = { 20.0f, -g * 4 };

	flam = 0;

	loop = true;

	reverse = false;

	Load("�A�N�V����/player.act");
}

// �f�X�g���N�^
Player::~Player()
{
}

// �ǂݍ���
void Player::Load(std::string fileName)
{
	id = tex.lock()->SetID();

	FILE* file;

	//�t�@�C���J�炭
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

	std::string path = "�A�N�V����/" + header.path;

	tex.lock()->LoadWIC(id, Texture::ChangeUnicode(path.c_str()));

	back = tex.lock()->SetID();
	tex.lock()->LoadWIC(back, Texture::ChangeUnicode("img/splatterhouse.png"));
}

// �`��
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

// ����
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

// ��Ԃ̐؂�ւ�
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

// �����̏���
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

// �W�����v�̏���
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

// ���݂̏�Ԃ̎擾
std::string Player::GetMode(void)
{
	return mode;
}

// ���]�t���O�̎擾
bool Player::GetReverse(void)
{
	return reverse;
}
