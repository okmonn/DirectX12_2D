#pragma once
#include <d3d12.h>
#include <dsound.h>
#include <string>
#include <map>
#include <memory>

class Window;

class Sound
{
public:
	// �R���X�g���N�^
	Sound(std::weak_ptr<Window>win);
	// �f�X�g���N�^
	~Sound();

	// �T�E���h�o�b�t�@�̐���
	HRESULT LoadWAV(USHORT* index, std::string fileName);

	// �T�E���h�̍Đ�
	HRESULT Play(USHORT* index, DWORD type);

	// �T�E���h�̒�~
	HRESULT Stop(USHORT* index);

	// �T�E���h�f�[�^�̏���
	void DeleteSoundWAV(USHORT* index);

private:
	// ��������
	HRESULT CreateDirectSound(void);


	// �v���C�}���T�E���h�o�b�t�@�̐���
	HRESULT CreatePrimaryBuffer(void);


	// �E�B���h�E�N���X�Q��
	std::weak_ptr<Window>win;

	// �Q�ƌ���
	HRESULT result;

	// �Z�J���_���[�o�b�t�@��
	std::map<std::string, LPDIRECTSOUNDBUFFER>origin;

	// �Z�J���_���[�o�b�t�@
	std::map<USHORT*, LPDIRECTSOUNDBUFFER>bgm;

	// �v���C�}���T�E���h�o�b�t�@
	LPDIRECTSOUNDBUFFER buffer;

	// �_�C���N�g�T�E���h
	LPDIRECTSOUND8 sound;
};