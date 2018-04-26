#pragma once
#include "Window.h"
#include "Input.h"
#include "Typedef.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <memory>
#include <vector>

class Device
{
	//�T�C�Y
	struct Size
	{
		LONG width;
		LONG height;
	};

	//BMP�f�[�^�̍\����
	struct BMP
	{
		//�摜�T�C�Y
		Size					size;
		//bmp�f�[�^
		std::vector<UCHAR>		data;
		//�q�[�v
		ID3D12DescriptorHeap*	heap;
		//���\�[�X
		ID3D12Resource*			resource;
	};

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

	// �����_�[�^�[�Q�b�g
	struct RenderTarget
	{
		//�q�[�v
		ID3D12DescriptorHeap* heap;
		//���\�[�X
		std::vector<ID3D12Resource*>resource;
		//�q�[�v�T�C�Y
		UINT size;
	};

	// �[�x�X�e���V��
	struct DepthStencil
	{
		//�q�[�v
		ID3D12DescriptorHeap* heap;
		//���\�[�X
		ID3D12Resource* resource;
		//�q�[�v�T�C�Y
		UINT size;
	};

	// �t�F���X
	struct Fence
	{
		//�t�F���X
		ID3D12Fence* fence;
		//�t�F���X�l
		UINT64			fenceCnt;
		//�t�F���X�C�x���g
		HANDLE			fenceEvent;
	};

	// ���[�g�V�O�l�`��
	struct RootSignature
	{
		//���b�Z�[�W
		ID3DBlob*				signature;
		//�G���[���b�Z�[�W
		ID3DBlob*				error;
		//���[�g�V�O�l�`��
		ID3D12RootSignature*	rootSignature;
	};

	// �p�C�v���C��
	struct PipeLine
	{
		//���_�f�[�^
		ID3DBlob* vertex;
		//�s�N�Z���f�[�^
		ID3DBlob* pixel;
		//�p�C�v���C��
		ID3D12PipelineState* pipeline;
	};

	// �萔�o�b�t�@
	struct Constant
	{
		//�q�[�v
		ID3D12DescriptorHeap* heap;
		//���\�[�X
		ID3D12Resource* resource;
		//�q�[�v�T�C�Y
		UINT size;
		//���M�f�[�^
		UINT8* data;
	};

	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
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
	//���[���h�r���[�v���W�F�N�V�����̃Z�b�g
	void SetWorldViewProjection(void);

	// �f�o�C�X�̐���
	HRESULT CreateDevice(void);
	// �R�}���h����̐���
	HRESULT CreateCommand(void);

	// �t�@�N�g���[�̐���
	HRESULT CreateFactory(void);
	// �X���b�v�`�F�C���̐���
	HRESULT CreateSwapChain(void);

	// �����_�[�^�[�Q�b�g�p�q�[�v�̐���
	HRESULT CreateRenderHeap(void);
	// �����_�[�^�[�Q�b�g�̐���
	HRESULT CreateRenderTarget(void);

	// �[�x�X�e���V���p�q�[�v�̐���
	HRESULT CreateDepthHeap(void);
	// �[�x�X�e���V���̐���
	HRESULT CreateDepthStencil(void);

	// �t�F���X�̐���
	HRESULT CreateFence(void);

	// �V�O�l�`���̃V���A���C�Y
	HRESULT Serialize(void);
	// ���[�g�V�O�l�`���̐���
	HRESULT CreateRootSigunature(void);

	// �V�F�[�_�̃R���p�C��
	HRESULT ShaderCompile(LPCWSTR fileName);

	// �p�C�v���C���̐���
	HRESULT CreatePipeline(void);

	// �萔�o�b�t�@�p�q�[�v�̐���	
	HRESULT CreateConstantHeap(void);
	// �萔�o�b�t�@�̐���
	HRESULT CreateConstant(void);

	// �r���[�|�[�g�̃Z�b�g
	void SetViewPort(void);

	// �V�U�[�̃Z�b�g
	void SetScissor(void);

	// �o���A�̍X�V
	void Barrier(D3D12_RESOURCE_STATES befor, D3D12_RESOURCE_STATES affter);

	// �ҋ@����
	void Wait(void);


	// �E�B���h�E�N���X
	std::weak_ptr<Window>win;

	// �C���v�b�g�N���X
	std::weak_ptr<Input>in;

	// �Q�ƌ���
	HRESULT result;

	// �@�\���x��
	D3D_FEATURE_LEVEL level;

	// ��]�p�x
	FLOAT angle;

	// �f�o�C�X
	ID3D12Device* dev;

	// �R�}���h
	Command com;

	// �X���b�v�`�F�C��
	Swap swap;

	// �����_�[�^�[�Q�b�g
	RenderTarget render;

	// �[�x�X�e���V��
	DepthStencil depth;

	// �t�F���X
	Fence fen;

	// ���[�g�V�O�l�`��
	RootSignature sig;

	// �p�C�v���C��
	PipeLine pipe;

	// �萔�o�b�t�@
	Constant con;

	// WVP
	WVP wvp;

	// �r���[�|�[�g
	D3D12_VIEWPORT viewPort;

	// �V�U�[
	RECT scissor;

	// �o���A
	D3D12_RESOURCE_BARRIER barrier;

	D3D12_VERTEX_BUFFER_VIEW view;
	ID3D12Resource* vi;

	BMP bmp;
};