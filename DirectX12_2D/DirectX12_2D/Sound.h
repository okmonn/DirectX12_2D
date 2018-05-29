#pragma once
#include "Input.h"
#include <d3d12.h>
#include <dsound.h>
#include <memory>

class Window;

class Sound
{
public:
	// �R���X�g���N�^
	Sound(std::weak_ptr<Window>win, std::weak_ptr<Input>input);
	// �f�X�g���N�^
	~Sound();

	// ��������
	HRESULT CreateDirectSound(void);

	// �v���C�}���T�E���h�o�b�t�@�̐���
	HRESULT CreatePrimaryBuffer(void);

	// �T�E���h�o�b�t�@�̐���
	HRESULT LoadWAV(LPDIRECTSOUNDBUFFER *wavData, const char *fileName);

	// �T�E���h�̍Đ�
	HRESULT Play(LPDIRECTSOUNDBUFFER wavData, DWORD type);

	// ����
	void UpData(void);

private:
	// �E�B���h�E�N���X�Q��
	std::weak_ptr<Window>win;

	// �C���v�b�g�N���X�Q��
	std::weak_ptr<Input>input;

	// �Q�ƌ���
	HRESULT result;

	// �Z�J���_���[�o�b�t�@
	LPDIRECTSOUNDBUFFER bgm;

	// �v���C�}���T�E���h�o�b�t�@
	LPDIRECTSOUNDBUFFER buffer;

	// �_�C���N�g�T�E���h
	LPDIRECTSOUND8 sound;
};