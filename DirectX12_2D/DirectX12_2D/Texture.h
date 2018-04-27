#pragma once
#include "Typedef.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <map>

class Texture
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

	//���_�f�[�^
	struct VertexData
	{
		//���_�f�[�^
		Vertex vertex[6];
		//���\�[�X
		ID3D12Resource* resource;
		//���M�f�[�^
		UCHAR* data;
		// ���_�o�b�t�@�r���[
		D3D12_VERTEX_BUFFER_VIEW vertexView;
	};

public:
	// �f�X�g���N�^
	~Texture();

	// �C���X�^���X��
	static void Create(void);
	// �j��
	static void Destroy(void);

	// �C���X�^���X�ϐ��̎擾
	static Texture* GetInstance(void)
	{
		return s_Instance;
	}

	// �ǂݍ���
	HRESULT LoadBMP(USHORT index, std::string fileName, ID3D12Device* dev);

	// �`�揀��
	void SetDraw(USHORT index, ID3D12GraphicsCommandList* list, UINT rootParamIndex);
	
	// �`��
	void Draw(USHORT index, FLOAT x, FLOAT y, ID3D12GraphicsCommandList * list, UINT rootParamIndex);

private:
	// �R���X�g���N�^
	Texture();

	// �萔�o�b�t�@�p�̃q�[�v�̐���	
	HRESULT CreateConstantHeap(USHORT index, ID3D12Device* dev);
	// �萔�o�b�t�@�̐���
	HRESULT CreateConstant(USHORT index, ID3D12Device* dev);
	// �V�F�[�_���\�[�X�r���[�̐���
	HRESULT CreateShaderResourceView(USHORT index, ID3D12Device* dev);

	// ���_���\�[�X�̐���
	HRESULT CreateVertex(USHORT index, ID3D12Device* dev);

	// �������
	void Release(ID3D12Resource* resource);
	// �������
	void Release(ID3D12DescriptorHeap* heap);


	// �C���X�^���X�ϐ�
	static Texture* s_Instance;

	// �Q�ƌ���
	HRESULT result;

	// BMP�f�[�^
	std::map<USHORT, BMP>bmp;

	// ���_�f�[�^
	std::map<USHORT, VertexData>v;
};

