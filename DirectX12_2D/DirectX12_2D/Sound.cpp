#include "Sound.h"
#include "Window.h"
#include "Typedef.h"
#include <cguid.h>
#include <tchar.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")

// �R���X�g���N�^
Sound::Sound(std::weak_ptr<Window>win, std::weak_ptr<Input>inputAdr) : win(win)
{
	//�C���v�b�g�N���X�Q��
	input = inputAdr;

	//�Q�ƌ��ʂ̏�����
	result = S_OK;

	//�Z�J���_���[�o�b�t�@�̏�����
	bgm = nullptr;

	//�v���C�}���T�E���h�o�b�t�@�̏�����
	buffer = nullptr;

	//�_�C���N�g�T�E���h
	sound = nullptr;

	//COM�̏�����
	CoInitialize(nullptr);


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

	result = CreateDirectSound();
	if (result == S_OK)
	{
		result = CreatePrimaryBuffer();
	}
	//result = LoadWAV(&bgm, "sample/�T���v��.wav");
}

// �f�X�g���N�^
Sound::~Sound()
{
	bgm->Stop();

	RELEASE(bgm);
	RELEASE(buffer);
	RELEASE(sound);

	// COM�̏I��
	CoUninitialize();
}

// ��������
HRESULT Sound::CreateDirectSound(void)
{
	//�_�C���N�g�T�E���h����
	result = DirectSoundCreate8(nullptr, &sound, nullptr);
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�_�C���N�g�T�E���h�̐����F���s\n"));
		return result;
	}

	/*�������[�h�Ƃ�Windows�ő��̃A�v���Ƃ̐���������邽�߂̂��̂ŁA
	�Ⴆ�Ύ����̃v���O��������t�H�[�J�X���ڂ����ꍇ�́A�T�E���h��炳�Ȃ��悤�ɂ���Ƃ������ݒ�*/
	result = sound->SetCooperativeLevel(win.lock()->GetWindowHandle(), DSSCL_EXCLUSIVE | DSSCL_PRIORITY);
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�������[�h�̃Z�b�g�F���s\n"));
		return result;
	}

	return result;
}

// �v���C�}���T�E���h�o�b�t�@�̐���
HRESULT Sound::CreatePrimaryBuffer(void)
{
	//�o�b�t�@�ݒ�p�\���̂̐ݒ�
	DSBUFFERDESC desc = {};
	desc.dwBufferBytes		= 0;
	desc.dwFlags			= DSBCAPS_PRIMARYBUFFER;
	desc.dwReserved			= 0;
	desc.dwSize				= sizeof(DSBUFFERDESC);
	desc.guid3DAlgorithm	= GUID_NULL;
	desc.lpwfxFormat		= NULL;

	//�v���C�}���T�E���h�o�b�t�@����
	result = sound->CreateSoundBuffer(&desc, &buffer, nullptr);
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�v���C�}���T�E���h�o�b�t�@�̐����F���s\n"));
		return result;
	}

	//�t�H�[�}�b�g�ݒ�p�\���̂̐ݒ�
	WAVEFORMATEX wav = {};
	wav.cbSize				= sizeof(WAVEFORMATEX);
	wav.nChannels			= 2;
	wav.nSamplesPerSec		= 44100;
	wav.wBitsPerSample		= 16;
	wav.nBlockAlign			= wav.nChannels * wav.wBitsPerSample / 8;
	wav.nAvgBytesPerSec		= wav.nSamplesPerSec * wav.nBlockAlign;
	wav.wFormatTag			= WAVE_FORMAT_PCM;

	//�t�H�[�}�b�g�̃Z�b�g
	result = buffer->SetFormat(&wav);
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�v���C�}���T�E���h�o�b�t�@�̃t�H�[�}�b�g�̃Z�b�g�F���s\n"));
		return result;
	}

	return result;
}

// �T�E���h�o�b�t�@�̐���
HRESULT Sound::LoadWAV(LPDIRECTSOUNDBUFFER *wavData, const char *fileName)
{
	//�t�@�C���n���h���̍\����
	HMMIO handle = {};

	//wav�t�@�C�����J��
	handle = mmioOpenA((LPSTR)fileName, NULL, MMIO_ALLOCBUF | MMIO_READ | MMIO_COMPAT);
	if (handle == nullptr)
	{
		OutputDebugString(_T("\n�t�@�C�����J���܂���ł����F���s\n"));
		return S_FALSE;
	}

	//RIFF�`�����N���\����
	MMCKINFO riff = {};

	//'W''A''V''E'�`�����N�̃`�F�b�N
	result = mmioDescend(handle, &riff, NULL, MMIO_FINDRIFF);
	if (riff.fccType != mmioFOURCC('W', 'A', 'V', 'E'))
	{
		//wav�t�@�C�������
		mmioClose(handle, 0);

		OutputDebugString(_T("\nWAVE�`�����N�̃`�F�b�N�F���s\n"));
		return S_FALSE;
	}

	//fmt�`�����N���\����
	MMCKINFO fmt = {};

	//'f''m''t'�`�����N�̃`�F�b�N
	result = mmioDescend(handle, &fmt, &riff, MMIO_FINDCHUNK);
	if (fmt.ckid != mmioFOURCC('f', 'm', 't', ' '))
	{
		//wav�t�@�C�������
		mmioClose(handle, 0);

		OutputDebugString(_T("\nfmt�`�����N�̃`�F�b�N�F���s\n"));
		return S_FALSE;
	}

	//�w�b�_�[�T�C�Y���擾
	UINT headerSize = fmt.cksize;

	//�w�b�_�[�T�C�Y�̏C��
	if (headerSize < sizeof(WAVEFORMATEX))
	{
		headerSize = sizeof(WAVEFORMATEX);
	}

	//wav�̃w�b�_�[���
	LPWAVEFORMATEX header;

	//�w�b�_�[�̃������m��
	header = (LPWAVEFORMATEX)malloc(headerSize);
	if (header == NULL)
	{
		//wav�t�@�C�������
		mmioClose(handle, 0);

		OutputDebugString(_T("\n�������m�ہF���s\n"));

		return S_FALSE;
	}
	SecureZeroMemory(header, headerSize);

	//wav�̃��[�h
	result = mmioRead(handle, (char*)header, fmt.cksize);
	if (FAILED(result))
	{
		//�������J��
		free(header);

		//wav�t�@�C�������
		mmioClose(handle, 0);

		OutputDebugString(_T("\nWAV�f�[�^�̃��[�h�F���s\n"));
		return result;
	}

	//fmt�`�����N�ɖ߂�
	mmioAscend(handle, &fmt, 0);

	//data�`�����N���\����
	MMCKINFO data = {};

	while (TRUE)
	{
		//����
		result = mmioDescend(handle, &data, &riff, 0);
		if (FAILED(result))
		{
			//�������J��
			free(header);

			//wav�t�@�C�������
			mmioClose(handle, 0);

			OutputDebugString(_T("\ndata�`�����N�̃`�F�b�N�F���s\n"));
			return result;
		}

		//data�`�����N�̂Ƃ�
		if (data.ckid == mmioStringToFOURCCA("data", 0))
		{
			break;
		}

		//���̃`�����N��
		result = mmioAscend(handle, &data, 0);
	}

	//�o�b�t�@�ݒ�p�\���̂̐ݒ�
	DSBUFFERDESC desc = {};
	desc.dwBufferBytes		= data.cksize;
	desc.dwFlags			= DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STATIC | DSBCAPS_LOCDEFER;
	desc.dwReserved			= 0;
	desc.dwSize				= sizeof(DSBUFFERDESC);
	desc.guid3DAlgorithm	= DS3DALG_DEFAULT;
	desc.lpwfxFormat		= header;

	//�o�b�t�@����
	result = sound->CreateSoundBuffer(&desc, wavData, NULL);
	if (FAILED(result))
	{
		//�������J��
		free(header);

		//wav�t�@�C�������
		mmioClose(handle, 0);

		OutputDebugString(_T("\n�Z�J���_���[�o�b�t�@�̐����F���s\n"));
		return result;
	}

	//�ǂݎ��f�[�^�i�[�p
	LPVOID mem[2];

	//�ǂݎ��T�C�Y
	DWORD size[2];

	//���b�N�J�n
	result = (*wavData)->Lock(0, data.cksize, &mem[0], &size[0], &mem[1], &size[1], 0);
	if (FAILED(result))
	{
		//�������J��
		free(header);

		//wav�t�@�C�������
		mmioClose(handle, 0);

		OutputDebugString(_T("\n�Z�J���_���[�o�b�t�@�̃��b�N�F���s\n"));
		return result;
	}

	for (UINT i = 0; i < 2; i++)
	{
		//�f�[�^��������
		mmioRead(handle, (char*)mem[i], size[i]);
	}

	//���b�N����
	(*wavData)->Unlock(mem[0], size[0], mem[1], size[1]);

	//�������J��
	free(header);

	//wav�t�@�C�������
	mmioClose(handle, 0);

	return result;
}

// �T�E���h�̍Đ�
HRESULT Sound::Play(LPDIRECTSOUNDBUFFER wavData, DWORD type)
{
	//�Đ�
	result = wavData->Play(0, 0, type);

	return result;
}

// ����
void Sound::UpData(void)
{
	if (input.lock()->InputKey(DIK_SPACE) == TRUE)
	{
		//���[�v�Đ�
		result = Play(bgm, DSBPLAY_LOOPING);
		if (FAILED(result))
		{
			OutputDebugString(_T("\n�Đ��F���s\n"));
			return;
		}
	}
}
