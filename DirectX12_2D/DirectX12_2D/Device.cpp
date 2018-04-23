#include "Device.h"
#include <tchar.h>

// �@�\���x���ꗗ
D3D_FEATURE_LEVEL levels[] = 
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

// �R���X�g���N�^
Device::Device(std::weak_ptr<Window> win, std::weak_ptr<Input> in) : win(win), in(in)
{
	//�Q�ƌ���
	result = S_OK;

	//�@�\���x��
	level = D3D_FEATURE_LEVEL_12_1;

	//�f�o�C�X
	dev = nullptr;
	
	//�R�}���h
	com = {};

	//�X���b�v�`�F�C��
	swap = {};


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


	Init();
}

// �f�X�g���N�^
Device::~Device()
{
	com.queue->Release();
	com.list->Release();
	com.allocator->Release();
	dev->Release();
}

// ������
void Device::Init(void)
{
	CreateCommand();
}

// ����
void Device::UpData(void)
{
}

// �f�o�C�X�̐���
HRESULT Device::CreateDevice(void)
{
	for (auto& i : levels)
	{
		result = D3D12CreateDevice(nullptr, i, IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			level = i;

			break;
		}
	}

	return result;
}

// �R�}���h����̐���
HRESULT Device::CreateCommand(void)
{
	if (CreateDevice() != S_OK)
	{
		OutputDebugString(_T("\n�f�o�C�X�̐����F���s\n"));
		return S_FALSE;
	}

	//�R�}���h�A���P�[�^�̐���
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&com.allocator));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�R�}���h�A���P�[�^�̐����F���s\n"));
		return result;
	}

	//�R�}���h���X�g�̐���
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, com.allocator, nullptr, IID_PPV_ARGS(&com.list));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�R�}���h���X�g�̐����F���s\n"));
		return result;
	}

	//�R�}���h�L���[�ݒ�p�\����
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask	= 0;
	desc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;

	//�R�}���h�L���[�̐���
	result = dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&com.queue));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�R�}���h�L���[�̐����F���s"));
		return result;
	}

	return result;
}

// �t�@�N�g���[�̐���
HRESULT Device::CreateFactory(void)
{
	result = CreateDXGIFactory1(IID_PPV_ARGS(&swap.factory));

	return result;
}

// �X���b�v�`�F�C���̐���
HRESULT Device::CreateSwapChain(void)
{
	if (CreateFactory() != S_OK)
	{
		OutputDebugString(_T("\n�t�@�N�g���[�̐����F���s\n"));
		return S_FALSE;
	}
}
