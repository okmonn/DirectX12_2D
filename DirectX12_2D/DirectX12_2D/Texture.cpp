#include "Texture.h"

#pragma comment (lib,"d3d12.lib")

Texture* Texture::s_Instance = nullptr;

// �R���X�g���N�^
Texture::Texture()
{
	//�Q�ƌ��ʂ̏�����
	result = S_OK;


	//�G���[���o�͂ɕ\��������
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

// �f�X�g���N�^
Texture::~Texture()
{
}

// �C���X�^���X��
void Texture::Create(void)
{
	if (s_Instance == nullptr)
	{
		s_Instance = new Texture;
	}
}

// �j��
void Texture::Destroy(void)
{
	if (s_Instance != nullptr)
	{
		delete s_Instance;
	}
	s_Instance = nullptr;
}

// �ǂݍ���
HRESULT Texture::LoadBMP(std::string fileName)
{
	return result;
}
