#include "Texture.h"
#include "WICTextureLoader12.h"
#include <sstream>
#include <tchar.h>

#pragma comment (lib,"d3d12.lib")

Texture* Texture::s_Instance = nullptr;

// �R���X�g���N�^
Texture::Texture()
{
	//WIC�̏�������
	CoInitialize(nullptr);

	//�Q�ƌ��ʂ̏�����
	result = S_OK;

	//BMP�f�[�^
	bmp.clear();

	//���_�f�[�^
	v.clear();
	
	//WIC�f�[�^
	wic.clear();

	//���_�f�[�^
	vic.clear();


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
}

// �f�X�g���N�^
Texture::~Texture()
{
	//���_�f�[�^
	for (auto itr = v.begin(); itr != v.end(); ++itr)
	{
		if (itr->second.resource != nullptr)
		{
			itr->second.resource->Unmap(0, nullptr);
		}
		Release(itr->second.resource);
	}
	for (auto itr = vic.begin(); itr != vic.end(); ++itr)
	{
		if (itr->second.resource != nullptr)
		{
			itr->second.resource->Unmap(0, nullptr);
		}
		Release(itr->second.resource);
	}

	//�萔�o�b�t�@
	for (auto itr = bmp.begin(); itr != bmp.end(); ++itr)
	{
		Release(itr->second.resource);
		Release(itr->second.heap);
	}
	for (auto itr = wic.begin(); itr != wic.end(); ++itr)
	{
		itr->second.decode.release();
		Release(itr->second.resource);
		Release(itr->second.heap);
	}
}

// �C���X�^���X��
void Texture::Create(void)
{
	if (s_Instance == nullptr)
	{
		s_Instance = new Texture;
	}
}

// �j��
void Texture::Destroy(void)
{
	if (s_Instance != nullptr)
	{
		delete s_Instance;
	}
	s_Instance = nullptr;
}

// ���j�R�[�h�ϊ�
std::wstring Texture::ChangeUnicode(const CHAR * str)
{
	//�������̎擾
	auto byteSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str, -1, nullptr, 0);

	std::wstring wstr;
	wstr.resize(byteSize);

	//�ϊ�
	byteSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str, -1, &wstr[0], byteSize);

	return wstr;
}

// �ǂݍ���
HRESULT Texture::LoadBMP(USHORT index, std::string fileName, ID3D12Device* dev)
{
	//BMP�w�b�_�[�\����
	BITMAPINFOHEADER header = {};

	//BMP�t�@�C���w�b�_�[
	BITMAPFILEHEADER fileheader = {};

	//�t�@�C��
	FILE* file;

	//�t�@�C���J�炭
	if ((fopen_s(&file, fileName.c_str(), "rb")) != 0)
	{
		//�G���[�i���o�[�m�F
		auto a = (fopen_s(&file, fileName.c_str(), "rb"));
		std::stringstream s;
		s << a;
		OutputDebugString(_T("\n�t�@�C�����J���܂���ł����F���s\n"));
		OutputDebugStringA(s.str().c_str());
		return S_FALSE;
	}

	//BMP�t�@�C���w�b�_�[�ǂݍ���
	fread(&fileheader, sizeof(fileheader), 1, file);

	//BMP�w�b�_�[�ǂݍ���
	fread(&header, sizeof(header), 1, file);

	//�摜�̕��ƍ����̕ۑ�
	bmp[index].size = { header.biWidth, header.biHeight };

	if (header.biBitCount == 24)
	{
		//�f�[�^�T�C�Y���̃������m��(�r�b�g�̐[����24bit�̏ꍇ)
		bmp[index].data.resize(header.biWidth * header.biHeight * 4);

		for (int line = header.biHeight - 1; line >= 0; --line)
		{
			for (int count = 0; count < header.biWidth * 4; count += 4)
			{
				//��ԍ��̔z��ԍ�
				UINT address = line * header.biWidth * 4;
				bmp[index].data[address + count] = 0;
				fread(&bmp[index].data[address + count + 1], sizeof(UCHAR), 3, file);
			}
		}
	}
	else if (header.biBitCount == 32)
	{
		//�f�[�^�T�C�Y���̃������m��(�r�b�g�̐[����32bit�̏ꍇ)
		bmp[index].data.resize(header.biSizeImage);

		for (int line = header.biHeight - 1; line >= 0; --line)
		{
			for (int count = 0; count < header.biWidth * 4; count += 4)
			{
				//��ԍ��̔z��ԍ�
				UINT address = line * header.biWidth * 4;
				fread(&bmp[index].data[address + count], sizeof(UCHAR), 4, file);
			}
		}
	}

	//�t�@�C�������
	fclose(file);

	result = CreateShaderResourceView(index, dev);
	result = CreateVertex(index, dev);

	return result;
}

// WIC�ǂݍ���
HRESULT Texture::LoadWIC(USHORT index, std::wstring fileName, ID3D12Device * dev)
{
	result = DirectX::LoadWICTextureFromFile(dev, fileName.c_str(), &wic[index].resource, wic[index].decode, wic[index].sub);
	if (FAILED(result))
	{
		OutputDebugString(_T("\nWIC�e�N�X�`���̓ǂݍ��݁F���s\n"));
		return result;
	}

	result = CreateShaderResourceViewWIC(index, dev);
	result = CreateVertexWIC(index, dev);

	return result;
}

// �萔�o�b�t�@�p�̃q�[�v�̐���	
HRESULT Texture::CreateConstantHeap(USHORT index, ID3D12Device* dev)
{
	//�q�[�v�ݒ�p�\���̂̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask			= 0;
	desc.NumDescriptors		= 2;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&bmp[index].heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�e�N�X�`���̒萔�o�b�t�@�p�q�[�v�̐����F���s\n"));
		return result;
	}

	return result;
}

// �萔�o�b�t�@�̐���
HRESULT Texture::CreateConstant(USHORT index, ID3D12Device* dev)
{
	if (CreateConstantHeap(index, dev) != S_OK)
	{
		OutputDebugString(_T("\n�e�N�X�`���̒萔�o�b�t�@�p�q�[�v�̐����F���s\n"));
		return S_FALSE;
	}

	//�q�[�v�X�e�[�g�ݒ�p�\���̂̐ݒ�
	D3D12_HEAP_PROPERTIES prop = {};
	prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	prop.CreationNodeMask		= 1;
	prop.MemoryPoolPreference	= D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_L0;
	prop.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_CUSTOM;
	prop.VisibleNodeMask		= 1;

	//���\�[�X�ݒ�p�\���̂̐ݒ�
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension				= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width					= bmp[index].size.width;
	desc.Height					= bmp[index].size.height;
	desc.DepthOrArraySize		= 1;
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count		= 1;
	desc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout					= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;

	//���\�[�X����
	result = dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&bmp[index].resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�e�N�X�`���̒萔�o�b�t�@�p���\�[�X�̐����F���s\n"));
		return result;
	}

	return result;
}

// �V�F�[�_���\�[�X�r���[�̐���
HRESULT Texture::CreateShaderResourceView(USHORT index, ID3D12Device * dev)
{
	if (CreateConstant(index, dev) != S_OK)
	{
		OutputDebugString(_T("\n�e�N�X�`���̒萔�o�b�t�@�p���\�[�X�̐����F���s\n"));
		return S_FALSE;
	}

	//�V�F�[�_���\�[�X�r���[�ݒ�p�\���̂̐ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format						= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension				= D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels		= 1;
	desc.Texture2D.MostDetailedMip	= 0;
	desc.Shader4ComponentMapping	= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//�q�[�v�̐擪�n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE handle = bmp[index].heap->GetCPUDescriptorHandleForHeapStart();

	//�V�F�[�_�[���\�[�X�r���[�̐���
	dev->CreateShaderResourceView(bmp[index].resource, &desc, handle);

	return S_OK;
}

// �萔�o�b�t�@�p�q�[�v�̐���
HRESULT Texture::CreateConstantHeapWIC(USHORT index, ID3D12Device * dev)
{
	//�q�[�v�ݒ�p�\���̂̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask			= 0;
	desc.NumDescriptors		= 2;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&wic[index].heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nWIC�e�N�X�`���̒萔�o�b�t�@�p�q�[�v�̐����F���s\n"));
		return result;
	}

	return result;
}

// �V�F�[�_���\�[�X�r���[�̐���
HRESULT Texture::CreateShaderResourceViewWIC(USHORT index, ID3D12Device * dev)
{
	if (CreateConstantHeapWIC(index, dev) != S_OK)
	{
		OutputDebugString(_T("\nWIC�e�N�X�`���̒萔�o�b�t�@�p�q�[�v�̐����F���s\n"));
		return S_FALSE;
	}

	//�V�F�[�_���\�[�X�r���[�ݒ�p�\���̂̐ݒ�
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format						= wic[index].resource->GetDesc().Format;
	desc.ViewDimension				= D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels		= 1;
	desc.Texture2D.MostDetailedMip	= 0;
	desc.Shader4ComponentMapping	= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//�q�[�v�̐擪�n���h�����擾
	D3D12_CPU_DESCRIPTOR_HANDLE handle = wic[index].heap->GetCPUDescriptorHandleForHeapStart();

	//�V�F�[�_�[���\�[�X�r���[�̐���
	dev->CreateShaderResourceView(wic[index].resource, &desc, handle);

	return S_OK;
}

// ���_���\�[�X�̐���
HRESULT Texture::CreateVertex(USHORT index, ID3D12Device * dev)
{
	//�z��̃������m��
	v[index].data = nullptr;

	for (UINT i = 0; i < 6; ++i)
	{
		v[index].vertex[i] = {};
	}

	//�q�[�v�ݒ�p�\���̂̐ݒ�
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference	= D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask		= 1;
	prop.VisibleNodeMask		= 1;

	//���\�[�X�ݒ�p�\���̂̐ݒ�
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension				= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width					= sizeof(v[index].vertex);//((sizeof(v[index].vertex) + 0xff) &~0xff);
	desc.Height					= 1;
	desc.DepthOrArraySize		= 1;
	desc.MipLevels				= 1;
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count		= 1;
	desc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout					= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//���_�p���\�[�X����
	result = dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&v[index].resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�e�N�X�`���̒��_�o�b�t�@�p���\�[�X�̐����F���s\n"));
		return result;
	}

	//���M�͈�
	D3D12_RANGE range = { 0,0 };

	//�}�b�s���O
	result = v[index].resource->Map(0, &range, reinterpret_cast<void**>(&v[index].data));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n���_�p���\�[�X�̃}�b�s���O�F���s\n"));
		return result;
	}

	//���_�f�[�^�̃R�s�[
	memcpy(v[index].data, &v[index].vertex, sizeof(v[index].vertex));

	//�A���}�b�s���O
	//v[index].resource->Unmap(0, nullptr);

	//���_�o�b�t�@�ݒ�p�\���̂̐ݒ�
	v[index].vertexView.BufferLocation  = v[index].resource->GetGPUVirtualAddress();
	v[index].vertexView.SizeInBytes	    = sizeof(v[index].vertex);
	v[index].vertexView.StrideInBytes   = sizeof(Vertex);

	return result;
}

// ���_���\�[�X�̐���
HRESULT Texture::CreateVertexWIC(USHORT index, ID3D12Device * dev)
{
	//�z��̃������m��
	vic[index].data = nullptr;

	for (UINT i = 0; i < 6; ++i)
	{
		vic[index].vertex[i] = {};
	}

	//�q�[�v�ݒ�p�\���̂̐ݒ�
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	//���\�[�X�ݒ�p�\���̂̐ݒ�
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = sizeof(vic[index].vertex);//((sizeof(v[index].vertex) + 0xff) &~0xff);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//���_�p���\�[�X����
	result = dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vic[index].resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�e�N�X�`���̒��_�o�b�t�@�p���\�[�X�̐����F���s\n"));
		return result;
	}

	//���M�͈�
	D3D12_RANGE range = { 0,0 };

	//�}�b�s���O
	result = vic[index].resource->Map(0, &range, reinterpret_cast<void**>(&vic[index].data));
	if (FAILED(result))
	{
		OutputDebugString(_T("\nWIC���_�p���\�[�X�̃}�b�s���O�F���s\n"));
		return result;
	}

	//���_�f�[�^�̃R�s�[
	memcpy(vic[index].data, &vic[index].vertex, sizeof(vic[index].vertex));

	//���_�o�b�t�@�ݒ�p�\���̂̐ݒ�
	vic[index].vertexView.BufferLocation = vic[index].resource->GetGPUVirtualAddress();
	vic[index].vertexView.SizeInBytes = sizeof(vic[index].vertex);
	vic[index].vertexView.StrideInBytes = sizeof(Vertex);

	return result;
}

// �`�揀��
void Texture::SetDraw(USHORT index, ID3D12GraphicsCommandList* list, UINT rootParamIndex)
{
	//���_�o�b�t�@�r���[�̃Z�b�g
	list->IASetVertexBuffers(0, 1, &v[index].vertexView);

	//�g�|���W�[�ݒ�
	list->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�{�b�N�X�ݒ�p�\���̂̐ݒ�
	D3D12_BOX box = {};
	box.back	= 1;
	box.bottom	= bmp[index].size.height;
	box.front	= 0;
	box.left	= 0;
	box.right	= bmp[index].size.width;
	box.top		= 0;

	//�T�u���\�[�X�ɏ�������
	result = bmp[index].resource->WriteToSubresource(0, &box, &bmp[index].data[0], (box.right * 4), (box.bottom * 4));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�e�N�X�`���̃T�u���\�[�X�ւ̏������݁F���s\n"));
		return;
	}

	//�q�[�v�̐擪�n���h�����擾
	D3D12_GPU_DESCRIPTOR_HANDLE handle = bmp[index].heap->GetGPUDescriptorHandleForHeapStart();

	//�q�[�v�̃Z�b�g
	list->SetDescriptorHeaps(1, &bmp[index].heap);

	//�f�B�X�N���v�^�[�e�[�u���̃Z�b�g
	list->SetGraphicsRootDescriptorTable(rootParamIndex, handle);

	//�`��
	list->DrawInstanced(6, 1, 0, 0);
}

// �`�揀��
void Texture::SetDrawWIC(USHORT index, ID3D12GraphicsCommandList * list, UINT rootParamIndex)
{
	//���_�o�b�t�@�r���[�̃Z�b�g
	list->IASetVertexBuffers(0, 1, &vic[index].vertexView);

	//�g�|���W�[�ݒ�
	list->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//���\�[�X�ݒ�p�\����
	D3D12_RESOURCE_DESC desc = {};
	desc = wic[index].resource->GetDesc();

	//�{�b�N�X�ݒ�p�\���̂̐ݒ�
	D3D12_BOX box = {};
	box.back	= 1;
	box.bottom	= desc.Height;
	box.front	= 0;
	box.left	= 0;
	box.right	= (UINT)desc.Width;
	box.top		= 0;

	//�T�u���\�[�X�ɏ�������
	result = wic[index].resource->WriteToSubresource(0, &box, wic[index].decode.get(), wic[index].sub.RowPitch, wic[index].sub.SlicePitch);
	if (FAILED(result))
	{
		OutputDebugString(_T("WIC�T�u���\�[�X�ւ̏������݁F���s\n"));
		return;
	}

	//�q�[�v�̐擪�n���h�����擾
	D3D12_GPU_DESCRIPTOR_HANDLE handle = wic[index].heap->GetGPUDescriptorHandleForHeapStart();

	//�q�[�v�̃Z�b�g
	list->SetDescriptorHeaps(1, &wic[index].heap);

	//�f�B�X�N���v�^�[�e�[�u���̃Z�b�g
	list->SetGraphicsRootDescriptorTable(rootParamIndex, handle);

	//�`��
	list->DrawInstanced(6, 1, 0, 0);
}

// �`��
void Texture::Draw(USHORT index, Vector2<FLOAT>pos, Vector2<FLOAT>size, ID3D12GraphicsCommandList * list, UINT rootParamIndex)
{
	v[index].vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f}, {0.0f, 0.0f} };//����
	v[index].vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f}, {1.0f, 0.0f} };//�E��
	v[index].vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f}, {1.0f, 1.0f} };//�E��
	v[index].vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f}, {1.0f, 1.0f} };//�E��
	v[index].vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f}, {0.0f, 1.0f} };//����
	v[index].vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f}, {0.0f, 0.0f} };//����

	//���_�f�[�^�̃R�s�[
	memcpy(v[index].data, &v[index].vertex, (sizeof(v[index].vertex)));

	//���_�o�b�t�@�ݒ�p�\���̂̐ݒ�
	v[index].vertexView.BufferLocation	= v[index].resource->GetGPUVirtualAddress();
	v[index].vertexView.SizeInBytes		= sizeof(v[index].vertex);
	v[index].vertexView.StrideInBytes	= sizeof(Vertex);
	
	SetDraw(index, list, rootParamIndex);
}

// �`��
void Texture::DrawWIC(USHORT index, Vector2<FLOAT> pos, Vector2<FLOAT> size, ID3D12GraphicsCommandList * list, UINT rootParamIndex)
{
	vic[index].vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ 0.0f, 0.0f } };//����
	vic[index].vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ 1.0f, 0.0f } };//�E��
	vic[index].vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ 1.0f, 1.0f } };//�E��
	vic[index].vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ 1.0f, 1.0f } };//�E��
	vic[index].vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ 0.0f, 1.0f } };//����
	vic[index].vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ 0.0f, 0.0f } };//����

	//���_�f�[�^�̃R�s�[
	memcpy(vic[index].data, &vic[index].vertex, (sizeof(vic[index].vertex)));

	//���_�o�b�t�@�ݒ�p�\���̂̐ݒ�
	vic[index].vertexView.BufferLocation	= vic[index].resource->GetGPUVirtualAddress();
	vic[index].vertexView.SizeInBytes		= sizeof(vic[index].vertex);
	vic[index].vertexView.StrideInBytes		= sizeof(Vertex);

	SetDrawWIC(index, list, rootParamIndex);
}

// �����`��
void Texture::DrawRect(USHORT index, Vector2<FLOAT> pos, Vector2<FLOAT> size, ID3D12GraphicsCommandList * list, UINT rootParamIndex, Vector2<FLOAT> rect, Vector2<FLOAT> rSize, bool turn)
{
	if (turn == false)
	{
		v[index].vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             rect.y / (FLOAT)bmp[index].size.height } };//����
		v[index].vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, rect.y / (FLOAT)bmp[index].size.height } };//�E��
		v[index].vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, (rect.y + (FLOAT)rSize.y) / bmp[index].size.height } };//�E��
		v[index].vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, (rect.y + (FLOAT)rSize.y) / bmp[index].size.height } };//�E��
		v[index].vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             (rect.y + rSize.y) / (FLOAT)bmp[index].size.height } };//����
		v[index].vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             rect.y / (FLOAT)bmp[index].size.height } };//����
	}
	else
	{
		v[index].vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, rect.y / (FLOAT)bmp[index].size.height } };//����
		v[index].vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             rect.y / (FLOAT)bmp[index].size.height } };//�E��
		v[index].vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             (rect.y + rSize.y) / (FLOAT)bmp[index].size.height } };//�E��
		v[index].vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)bmp[index].size.width,             (rect.y + rSize.y) / (FLOAT)bmp[index].size.height } };//�E��
		v[index].vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, (rect.y + (FLOAT)rSize.y) / bmp[index].size.height } };//����
		v[index].vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			 1.0f - (pos.y / (float)(WINDOW_Y / 2)),			0.0f },{ (rect.x + rSize.x) / (FLOAT)bmp[index].size.width, rect.y / (FLOAT)bmp[index].size.height } };//����
	}

	//���_�f�[�^�̃R�s�[
	memcpy(v[index].data, &v[index].vertex, (sizeof(v[index].vertex)));

	//���_�o�b�t�@�ݒ�p�\���̂̐ݒ�
	v[index].vertexView.BufferLocation	= v[index].resource->GetGPUVirtualAddress();
	v[index].vertexView.SizeInBytes		= sizeof(v[index].vertex);
	v[index].vertexView.StrideInBytes	= sizeof(Vertex);

	//���_�o�b�t�@�r���[�̃Z�b�g
	list->IASetVertexBuffers(0, 1, &v[index].vertexView);

	SetDraw(index, list, rootParamIndex);
}

// �����`��
void Texture::DrawRectWIC(USHORT index, Vector2<FLOAT> pos, Vector2<FLOAT> size, ID3D12GraphicsCommandList * list, UINT rootParamIndex, Vector2<FLOAT> rect, Vector2<FLOAT> rSize, bool turn)
{
	if (turn == false)
	{
		vic[index].vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//����
		vic[index].vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//�E��
		vic[index].vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//�E��
		vic[index].vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//�E��
		vic[index].vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//����
		vic[index].vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//����
	}
	else
	{
		vic[index].vertex[0] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//����
		vic[index].vertex[1] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//�E��
		vic[index].vertex[2] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//�E��
		vic[index].vertex[3] = { { ((pos.x + size.x) / (float)(WINDOW_X / 2)) - 1.0f,  1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ rect.x / (FLOAT)(wic[index].resource->GetDesc().Width),             (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//�E��
		vic[index].vertex[4] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - ((pos.y + size.y) / (float)(WINDOW_Y / 2)), 0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), (rect.y + rSize.y) / (FLOAT)(wic[index].resource->GetDesc().Height) } };//����
		vic[index].vertex[5] = { { (pos.x / (float)(WINDOW_X / 2)) - 1.0f,			   1.0f - (pos.y / (float)(WINDOW_Y / 2)),			  0.0f },{ (rect.x + rSize.x) / (FLOAT)(wic[index].resource->GetDesc().Width), rect.y / (FLOAT)(wic[index].resource->GetDesc().Height) } };//����
	}

	//���_�f�[�^�̃R�s�[
	memcpy(vic[index].data, &vic[index].vertex, (sizeof(vic[index].vertex)));

	//���_�o�b�t�@�ݒ�p�\���̂̐ݒ�
	vic[index].vertexView.BufferLocation	= vic[index].resource->GetGPUVirtualAddress();
	vic[index].vertexView.SizeInBytes		= sizeof(vic[index].vertex);
	vic[index].vertexView.StrideInBytes		= sizeof(Vertex);

	//���_�o�b�t�@�r���[�̃Z�b�g
	list->IASetVertexBuffers(0, 1, &vic[index].vertexView);

	SetDrawWIC(index, list, rootParamIndex);
}

// �������
void Texture::Release(ID3D12Resource * resource)
{
	if (resource != nullptr)
	{
		resource->Release();
	}
}

// �������
void Texture::Release(ID3D12DescriptorHeap * heap)
{
	if (heap != nullptr)
	{
		heap->Release();
	}
}
