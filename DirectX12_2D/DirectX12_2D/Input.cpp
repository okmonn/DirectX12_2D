#include "Input.h"
#include "KeyTbl.h"
#include <tchar.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dinput8.lib")

// �R���X�g���N�^
Input::Input(std::weak_ptr<Window>winAdr) : win(winAdr)
{
	//�Q�ƌ��ʂ̏�����
	result = S_OK;

	//�C���v�b�g�̏�����
	input = nullptr;

	//�C���v�b�g�f�o�C�X�̏�����
	dev = nullptr;

	//�L�[���z��̏�����
	for (UINT i = 0; i < 256; i++)
	{
		key[i] = 0;
	}


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

	result = SetInputDevice();
}

// �f�X�g���N�^
Input::~Input()
{
	//�C���v�b�g�f�o�C�X
	if (dev != nullptr)
	{
		//�L�[�{�[�h�ւ̃A�N�Z�X�����̊J��
		dev->Unacquire();

		dev->Release();
	}

	//�C���v�b�g
	if (input != nullptr)
	{
		input->Release();
	}
}

// �C���v�b�g�̐���
HRESULT Input::CreateInput(void)
{
	//�C���v�b�g����
	result = DirectInput8Create(GetModuleHandle(0), VERSION, IID_IDirectInput8, (void**)(&input), NULL);
	if (FAILED(result))
	{
		OutputDebugString(_T("�C���v�b�g�̐����F���s\n"));

		return result;
	}

	return result;
}

// �C���v�b�g�f�o�C�X�̐���
HRESULT Input::CreateInputDevice(void)
{
	result = CreateInput();
	if (FAILED(result))
	{
		return result;
	}

	//�C���v�b�g�f�o�C�X����
	result = input->CreateDevice(GUID_SysKeyboard, &dev, NULL);
	if (FAILED(result))
	{
		OutputDebugString(_T("�C���v�b�g�f�o�C�X�̐����F���s\n"));

		return result;
	}

	return result;
}

// �C���v�b�g�f�o�C�X���L�[�{�[�h�ɃZ�b�g
HRESULT Input::SetInputDevice(void)
{
	result = CreateInputDevice();
	if (FAILED(result))
	{
		return result;
	}

	//�L�[�{�[�h�ɃZ�b�g
	result = dev->SetDataFormat(&keybord);
	if (FAILED(result))
	{
		OutputDebugString(_T("�C���v�b�g�f�o�C�X�̃L�[�{�[�h�Z�b�g�F���s\n"));
		return result;
	}

	//�������x�����Z�b�g
	result = dev->SetCooperativeLevel(win.lock()->GetWindowHandle(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	if (FAILED(result))
	{
		OutputDebugString(_T("�������x���̃Z�b�g�F���s\n"));

		return result;
	}

	//���̓f�o�C�X�ւ̃A�N�Z�X�������擾
	dev->Acquire();

	return result;
}

// �L�[����
BOOL Input::InputKey(UINT data)
{
	//�_�~�[�錾
	BOOL flag = FALSE;

	//�L�[�����擾
	dev->GetDeviceState(sizeof(key), &key);

	if (key[data] & 0x80)
	{
		flag = TRUE;
	}

	return flag;
}
