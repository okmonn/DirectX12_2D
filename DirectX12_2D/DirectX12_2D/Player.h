#pragma once
#include "Input.h"
#include "Texture.h"

class Player
{
	// �񎟌����W
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
		//���S���W
		Position pos;

		//��
		Position size;

		Rect() : pos({ 0,0 }), size({ 0,0 })
		{}
		Rect(int x, int y, int w, int h) : pos({ x, y }), size({ w,h })
		{}
		Rect(Position& pos, int w, int h) : pos(pos), size({ w,h })
		{}
		//���S�ʒu�̍X�V
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

		//��؂���W�̎擾
		Position GetPos(void)
		{
			return pos;
		}

		//�����W�̎擾
		int GetLeft(void)
		{
			return pos.x - size.x / 2;
		}
		//����W�̎擾
		int GetTop(void)
		{
			return pos.y - size.y / 2;
		}
		//�E���W�̎擾
		int GetRight(void)
		{
			return pos.x + size.x / 2;
		}
		//�����W�̎擾
		int GetBottom(void)
		{
			return pos.y + size.y / 2;
		}
		//�����̎擾
		int GetWidth(void)
		{
			return size.x;
		}
		//�c���̎擾
		int GetHeight(void)
		{
			return size.y;
		}
	};

	// �摜�w�b�_�[�\����
	struct ImageHeader
	{
		//������̒���
		int pathNum;
		//�t�@�C���p�X
		std::string path;
		//�f�[�^��
		int dataNum;
	};
	// �摜�f�[�^�\����
	struct ImageData
	{
		//������̒���
		int nameNum;
		//�A�N�V������
		std::string name;
		//���[�v�t���O
		char loop;
		//�A�j���[�V������
		int animCnt;
	};
	//�����f�[�^
	struct CutData
	{
		//�؂蔲���Z�`
		Rect rect;
		//���S���W
		Position center;
		//�A�j���[�V��������
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
	// �R���X�g���N�^
	Player(std::weak_ptr<Input>in, std::weak_ptr<Texture>tex);
	// �f�X�g���N�^
	~Player();

	// �ǂݍ���
	void Load(std::string fileName);
	
	// �`��
	void Draw(void);

	// ����
	void UpData(void);

	// ��Ԃ̐؂�ւ�
	void SetMode(std::string m, bool r);

	// ���݂̏�Ԃ̎擾
	std::string GetMode(void);
	// ���]�t���O�̎擾
	bool GetReverse(void);

private:
	// �����̏���
	void Walk(void);
	// �W�����v�̏���
	void Jump(void);



	// �C���v�b�g
	std::weak_ptr<Input>in;

	// �e�N�X�`��
	std::weak_ptr<Texture>tex;

	// �摜ID
	USHORT id;

	USHORT back;

	// �w�b�_�[
	ImageHeader header;
	// �摜�f�[�^
	std::map<std::string, ImageData>data;
	// �����f�[�^
	std::map<std::string, std::vector<CutData>>cut;

	// ���
	std::string mode;

	// ���W
	Pos pos;

	// ��
	Pos vel;

	// �C���f�b�N�X
	UINT index;

	// �t���[��
	UINT flam;

	// ���[�v�t���O
	bool loop;

	// ���]�t���O
	bool reverse;
};

