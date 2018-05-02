#pragma once
#include "Device.h"

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
	// �R���X�g���N�^
	Texture(std::weak_ptr<Device>dev);
	// �f�X�g���N�^
	~Texture();

	// ���j�R�[�h�ϊ�
	static std::wstring ChangeUnicode(const CHAR * str);

	// �摜ID�̃Z�b�g
	int SetID(void);

	// �ǂݍ���
	HRESULT LoadBMP(USHORT index, std::string fileName);
	// WIC�ǂݍ���
	HRESULT LoadWIC(USHORT index, std::wstring fileName);

	// �`�揀��
	void SetDraw(USHORT index);
	// �`�揀��
	void SetDrawWIC(USHORT index);
	
	// �`��
	void Draw(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size);
	// �`��
	void DrawWIC(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size);
	// �����`��
	void DrawRect(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size, Vector2<FLOAT>rect, Vector2<FLOAT>rSize, bool turn = false);
	// �����`��
	void DrawRectWIC(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size, Vector2<FLOAT>rect, Vector2<FLOAT>rSize, bool turn = false);

private:
	// �萔�o�b�t�@�p�̃q�[�v�̐���	
	HRESULT CreateConstantHeap(USHORT index);
	// �萔�o�b�t�@�̐���
	HRESULT CreateConstant(USHORT index);
	// �V�F�[�_���\�[�X�r���[�̐���
	HRESULT CreateShaderResourceView(USHORT index);

	// �萔�o�b�t�@�p�q�[�v�̐���
	HRESULT CreateConstantHeapWIC(USHORT index);
	// �V�F�[�_���\�[�X�r���[�̐���
	HRESULT CreateShaderResourceViewWIC(USHORT index);

	// ���_���\�[�X�̐���
	HRESULT CreateVertex(USHORT index);
	
	// ���_���\�[�X�̐���
	HRESULT CreateVertexWIC(USHORT index);

	// �������
	void Release(ID3D12Resource* resource);
	// �������
	void Release(ID3D12DescriptorHeap* heap);


	// �f�o�C�X
	std::weak_ptr<Device>dev;

	// �摜ID
	int id;

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

