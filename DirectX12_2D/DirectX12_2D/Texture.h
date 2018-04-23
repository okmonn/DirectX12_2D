#pragma once
#include <d3d12.h>
#include <string>
#include <vector>

class Texture
{
	//�T�C�Y
	struct Size
	{
		LONG width;
		LONG height;
	};
	
	//BMP�f�[�^�̍\����
	typedef struct
	{
		//�摜�T�C�Y
		Size				size;
		//bmp�f�[�^
		std::vector<UCHAR>		bmp;
		//�q�[�v
		ID3D12DescriptorHeap*	heap;
		//���\�[�X
		ID3D12Resource*			resource;
	}Data;

public:
	// �f�X�g���N�^
	~Texture();

	// �C���X�^���X��
	static void Create(void);
	// �j��
	static void Destroy(void);

	// �C���X�^���X�ϐ��̎擾
	Texture* GetInstance(void)
	{
		return s_Instance;
	}

	// �ǂݍ���
	HRESULT LoadBMP(std::string fileName);
private:
	// �R���X�g���N�^
	Texture();

	// �C���X�^���X�ϐ�
	static Texture* s_Instance;

	// �Q�ƌ���
	HRESULT result;
};

