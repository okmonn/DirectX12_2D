#pragma once
#include "Window.h"
#include "Input.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <memory>
#include <vector>

class Device
{
	// �R�}���h����
	struct Command
	{
		// �R�}���h�A���P�[�^
		ID3D12CommandAllocator* allocator;
		// �R�}���h���X�g
		ID3D12GraphicsCommandList* list;
		// �R�}���h�L���[
		ID3D12CommandQueue* queue;
	};

	// �X���b�v�`�F�C��
	struct Swap
	{
		//�C���^�[�t�F�[�X�t�@�N�g���[
		IDXGIFactory4*		factory;
		//�X���b�v�`�F�C��
		IDXGISwapChain3*	swapChain;
		//�o�b�N�o�b�t�@��
		UINT				bufferCnt;
	};

public:
	// �R���X�g���N�^
	Device(std::weak_ptr<Window>win, std::weak_ptr<Input>in);
	// �f�X�g���N�^
	~Device();

	// ������
	void Init(void);

	// ����
	void UpData(void);
private:
	// �f�o�C�X�̐���
	HRESULT CreateDevice(void);
	// �R�}���h����̐���
	HRESULT CreateCommand(void);

	// �t�@�N�g���[�̐���
	HRESULT CreateFactory(void);
	// �X���b�v�`�F�C���̐���
	HRESULT CreateSwapChain(void);


	// �E�B���h�E�N���X
	std::weak_ptr<Window>win;

	// �C���v�b�g�N���X
	std::weak_ptr<Input>in;

	// �Q�ƌ���
	HRESULT result;

	// �@�\���x��
	D3D_FEATURE_LEVEL level;

	// �f�o�C�X
	ID3D12Device* dev;

	// �R�}���h
	Command com;

	// �X���b�v�`�F�C��
	Swap swap;
};