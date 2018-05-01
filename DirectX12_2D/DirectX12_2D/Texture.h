#pragma once
#include "Typedef.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

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

	//WIC�f�[�^
	struct WIC
	{
		//�q�[�v
		ID3D12DescriptorHeap* heap;
		//���\�[�X
		ID3D12Resource* resource;
		//�f�R�[�h
		std::unique_ptr<uint8_t[]>decode;
		//�T�u
		D3D12_SUBRESOURCE_DATA sub;
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

	// ���j�R�[�h�ϊ�
	static std::wstring ChangeUnicode(const CHAR * str);

	// �ǂݍ���
	HRESULT LoadBMP(USHORT index, std::string fileName, ID3D12Device* dev);
	// WIC�ǂݍ���
	HRESULT LoadWIC(USHORT index, std::wstring fileName, ID3D12Device* dev);

	// �`�揀��
	void SetDraw(USHORT index, ID3D12GraphicsCommandList* list, UINT rootParamIndex);
	// �`�揀��
	void SetDrawWIC(USHORT index, ID3D12GraphicsCommandList* list, UINT rootParamIndex);
	
	// �`��
	void Draw(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size, ID3D12GraphicsCommandList * list, UINT rootParamIndex);
	// �`��
	void DrawWIC(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size, ID3D12GraphicsCommandList * list, UINT rootParamIndex);
	// �����`��
	void DrawRect(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size, ID3D12GraphicsCommandList * list, UINT rootParamIndex, Vector2<FLOAT>rect, Vector2<FLOAT>rSize, bool turn = false);
	// �����`��
	void DrawRectWIC(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size, ID3D12GraphicsCommandList * list, UINT rootParamIndex, Vector2<FLOAT>rect, Vector2<FLOAT>rSize, bool turn = false);

private:
	// �R���X�g���N�^
	Texture();

	// �萔�o�b�t�@�p�̃q�[�v�̐���	
	HRESULT CreateConstantHeap(USHORT index, ID3D12Device* dev);
	// �萔�o�b�t�@�̐���
	HRESULT CreateConstant(USHORT index, ID3D12Device* dev);
	// �V�F�[�_���\�[�X�r���[�̐���
	HRESULT CreateShaderResourceView(USHORT index, ID3D12Device* dev);

	// �萔�o�b�t�@�p�q�[�v�̐���
	HRESULT CreateConstantHeapWIC(USHORT index, ID3D12Device* dev);
	// �V�F�[�_���\�[�X�r���[�̐���
	HRESULT CreateShaderResourceViewWIC(USHORT index, ID3D12Device* dev);

	// ���_���\�[�X�̐���
	HRESULT CreateVertex(USHORT index, ID3D12Device* dev);
	
	// ���_���\�[�X�̐���
	HRESULT CreateVertexWIC(USHORT index, ID3D12Device* dev);

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

	// WIC�f�[�^
	std::map<USHORT, WIC>wic;

	// ���_�f�[�^
	std::map<USHORT, VertexData>vic;
};

