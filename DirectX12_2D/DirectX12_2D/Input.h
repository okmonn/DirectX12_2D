#pragma once
#define INITGUID
#include "Window.h"
#include <d3d12.h>
#include <dinput.h>
#include <memory>

#define VERSION 0x0800

class Input
{
public:
	// �R���X�g���N�^
	Input(std::weak_ptr<Window>winAdr);
	// �f�X�g���N�^
	~Input();

	// �C���v�b�g�̐���
	HRESULT CreateInput(void);
	// �C���v�b�g�f�o�C�X�̐���
	HRESULT CreateInputDevice(void);

	// �C���v�b�g�f�o�C�X���L�[�{�[�h�ɃZ�b�g
	HRESULT SetInputDevice(void);

	// �L�[����
	BOOL InputKey(UINT data);

private:
	// �E�B���h�E�N���X�Q��
	std::weak_ptr<Window>win;

	// �Q�ƌ���
	HRESULT result;

	// �C���v�b�g
	LPDIRECTINPUT8 input;

	// �C���v�b�g�f�o�C�X
	LPDIRECTINPUTDEVICE8 dev;

	// �L�[���
	BYTE key[256];
};