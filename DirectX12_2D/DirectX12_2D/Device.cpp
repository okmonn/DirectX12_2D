#include "Device.h"
#include "Typedef.h"
#include "Texture.h"
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <tchar.h>

#pragma comment (lib,"d3d12.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"d3dcompiler.lib")

//�N���A�J���[�̎w��
const FLOAT clearColor[] = { 1.0f,0.0f,0.0f,0.0f };

// �@�\���x���ꗗ
D3D_FEATURE_LEVEL levels[] = 
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

// �R���X�g���N�^
Device::Device(std::weak_ptr<Window> win, std::weak_ptr<Input> in) : win(win), in(in)
{
	//�Q�ƌ���
	result = S_OK;

	//�@�\���x��
	level = D3D_FEATURE_LEVEL_12_1;

	//��]�p�x
	angle = 0.0f;

	//�f�o�C�X
	dev = nullptr;
	
	//�R�}���h
	com = {};

	//�X���b�v�`�F�C��
	swap = {};

	//�����_�[�^�[�Q�b�g
	render = {};

	//�[�x�X�e���V��
	depth = {};

	//�t�F���X
	fen = {};

	//���[�g�V�O�l�`��
	sig = {};

	//�p�C�v���C��
	pipe = {};

	//�萔�o�b�t�@
	con = {};

	//WVP
	wvp = {};

	//�r���[�|�[�g
	viewPort = {};

	//�V�U�[
	scissor = {};

	// �o���A
	barrier = {};

	x = 0.0f;
	y = 0.0f;


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


	Init();
}

// �f�X�g���N�^
Device::~Device()
{
	//�萔�o�b�t�@�̃A���}�b�v
	con.resource->Unmap(0, nullptr);

	Texture::Destroy();

	Release(con.resource);
	Release(con.heap);
	Release(pipe.vertex);
	Release(pipe.pixel);
	Release(pipe.pipeline);
	Release(sig.signature);
	Release(sig.error);
	Release(sig.rootSignature);
	Release(fen.fence);
	Release(depth.resource);
	Release(depth.heap);
	for (UINT i = 0; i < render.resource.size(); ++i)
	{
		Release(render.resource[i]);
	}
	Release(render.heap);
	Release(swap.factory);
	Release(swap.swapChain);
	Release(com.queue);
	Release(com.list);
	Release(com.allocator);
	Release(dev);
}

// ������
void Device::Init(void)
{
	SetWorldViewProjection();

	CreateCommand();

	CreateSwapChain();

	CreateRenderTarget();

	CreateDepthStencil();

	CreateFence();

	CreateRootSigunature();

	ShaderCompile(_T("Shader.hlsl"));

	CreatePipeline();

	CreateConstant();

	SetViewPort();

	SetScissor();

	Texture::Create();
	
	Texture::GetInstance()->LoadWIC(0, Texture::ChangeUnicode("img/rick.png"), dev);
	Texture::GetInstance()->LoadWIC(1, Texture::ChangeUnicode("img/a.png"), dev);
}

// �`��
void Device::Draw(void)
{
	//Texture::GetInstance()->Draw(0, { x, y }, {100, 100}, com.list, 1);
	//Texture::GetInstance()->DrawRect(0, { x, y }, { 100.0f,100.0f }, com.list, 1, { 0.0f,0.0f }, { 100.0f,100.0f });
	Texture::GetInstance()->DrawRectWIC(0, { x,y }, { 100,100 }, com.list, 1, { 0,0 }, { 100,100 }, true);
	Texture::GetInstance()->DrawRectWIC(1, { 100,0 }, { 100,100 }, com.list, 1, { 0,0 }, { 100,100 });
}

// ����
void Device::UpData(void)
{
	if (in.lock()->InputKey(DIK_RIGHT) == TRUE)
	{
		//��]
		angle++;
		//�s��X�V
		wvp.world = DirectX::XMMatrixRotationY(RAD(angle));
	}
	else if (in.lock()->InputKey(DIK_LEFT) == TRUE)
	{
		//��]
		angle--;
		//�s��X�V
		wvp.world = DirectX::XMMatrixRotationY(RAD(angle));
	}

	//�s��f�[�^�X�V
	memcpy(con.data, &wvp, sizeof(WVP));

	if (in.lock()->InputKey(DIK_RIGHT))
	{
		x += 1.0f;
	}
	else if (in.lock()->InputKey(DIK_LEFT))
	{
		x -= 1.0f;
	}
	else if (in.lock()->InputKey(DIK_DOWN))
	{
		y += 1.0f;
	}
	else if (in.lock()->InputKey(DIK_UP))
	{
		y -= 1.0f;
	}

	//�R�}���h�A���P�[�^�̃��Z�b�g
	com.allocator->Reset();
	//���X�g�̃��Z�b�g
	com.list->Reset(com.allocator, pipe.pipeline);

	//���[�g�V�O�l�`���̃Z�b�g
	com.list->SetGraphicsRootSignature(sig.rootSignature);

	//�p�C�v���C���̃Z�b�g
	com.list->SetPipelineState(pipe.pipeline);

	//�萔�o�b�t�@�q�[�v�̐擪�n���h�����擾
	D3D12_GPU_DESCRIPTOR_HANDLE c_handle = con.heap->GetGPUDescriptorHandleForHeapStart();

	//�萔�o�b�t�@�q�[�v�̃Z�b�g
	com.list->SetDescriptorHeaps(1, &con.heap);

	//�萔�o�b�t�@�f�B�X�N���v�^�[�e�[�u���̃Z�b�g
	com.list->SetGraphicsRootDescriptorTable(0, c_handle);

	//�r���[�̃Z�b�g
	com.list->RSSetViewports(1, &viewPort);

	//�V�U�[�̃Z�b�g
	com.list->RSSetScissorRects(1, &scissor);

	//Present ---> RenderTarget
	Barrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//���_�q�[�v�̐擪�n���h���̎擾
	CD3DX12_CPU_DESCRIPTOR_HANDLE r_handle(render.heap->GetCPUDescriptorHandleForHeapStart(), swap.swapChain->GetCurrentBackBufferIndex(), render.size);

	//�����_�[�^�[�Q�b�g�̃Z�b�g
	com.list->OMSetRenderTargets(1, &r_handle, false, &depth.heap->GetCPUDescriptorHandleForHeapStart());

	//�����_�[�^�[�Q�b�g�̃N���A
	com.list->ClearRenderTargetView(r_handle, clearColor, 0, nullptr);

	//�[�x�X�e���V���q�[�v�̐擪�n���h���̎擾
	D3D12_CPU_DESCRIPTOR_HANDLE d_handle = depth.heap->GetCPUDescriptorHandleForHeapStart();

	//�[�x�X�e���V���r���[�̃N���A
	com.list->ClearDepthStencilView(d_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//�`��
	Draw();

	// RenderTarget ---> Present
	Barrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	//�R�}���h���X�g�̋L�^�I��
	com.list->Close();

	//���X�g�̔z��
	ID3D12CommandList *commandList[] = { com.list };
	//�z��łȂ��ꍇ�Fqueue->ExecuteCommandLists(1, (ID3D12CommandList*const*)&list);
	com.queue->ExecuteCommandLists(_countof(commandList), commandList);

	//���A�\��ʂ𔽓]
	swap.swapChain->Present(1, 0);

	Wait();
}

//���[���h�r���[�v���W�F�N�V�����̃Z�b�g
void Device::SetWorldViewProjection(void)
{
	//�_�~�[�錾
	FLOAT pos = 0.0f;
	DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
	//�J�����̈ʒu
	DirectX::XMVECTOR eye = { 0, pos,  -1 };
	//�J�����̏œ_
	DirectX::XMVECTOR target = { 0, pos,   0 };
	//�J�����̏����
	DirectX::XMVECTOR upper = { 0, 1,     0 };

	view = DirectX::XMMatrixLookAtLH(eye, target, upper);

	//�_�~�[�錾
	DirectX::XMMATRIX projection = DirectX::XMMatrixIdentity();

	projection = DirectX::XMMatrixPerspectiveFovLH(RAD(90), ((static_cast<FLOAT>(WINDOW_X) / static_cast<FLOAT>(WINDOW_Y))), 0.5f, 500.0f);

	//�X�V
	wvp.world = DirectX::XMMatrixIdentity();
	wvp.viewProjection = view * projection;
}

// �f�o�C�X�̐���
HRESULT Device::CreateDevice(void)
{
	for (auto& i : levels)
	{
		result = D3D12CreateDevice(nullptr, i, IID_PPV_ARGS(&dev));
		if (result == S_OK)
		{
			level = i;

			break;
		}
	}

	return result;
}

// �R�}���h����̐���
HRESULT Device::CreateCommand(void)
{
	if (CreateDevice() != S_OK)
	{
		OutputDebugString(_T("\n�f�o�C�X�̐����F���s\n"));
		return S_FALSE;
	}

	//�R�}���h�A���P�[�^�̐���
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&com.allocator));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�R�}���h�A���P�[�^�̐����F���s\n"));
		return result;
	}

	//�R�}���h���X�g�̐���
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, com.allocator, nullptr, IID_PPV_ARGS(&com.list));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�R�}���h���X�g�̐����F���s\n"));
		return result;
	}

	//�����������
	com.list->Close();

	//�R�}���h�L���[�ݒ�p�\����
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags		= D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask	= 0;
	desc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type		= D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;

	//�R�}���h�L���[�̐���
	result = dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&com.queue));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�R�}���h�L���[�̐����F���s"));
		return result;
	}

	return result;
}

// �t�@�N�g���[�̐���
HRESULT Device::CreateFactory(void)
{
	result = CreateDXGIFactory1(IID_PPV_ARGS(&swap.factory));

	return result;
}

// �X���b�v�`�F�C���̐���
HRESULT Device::CreateSwapChain(void)
{
	if (CreateFactory() != S_OK)
	{
		OutputDebugString(_T("\n�t�@�N�g���[�̐����F���s\n"));
		return S_FALSE;
	}

	//�X���b�v�`�F�C���ݒ�p�\����
	DXGI_SWAP_CHAIN_DESC1 desc;
	SecureZeroMemory(&desc, sizeof(desc));
	desc.AlphaMode		= DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.BufferCount	= 2;
	desc.BufferUsage	= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.Flags			= 0;
	desc.Format			= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Height			= WINDOW_Y;
	desc.SampleDesc		= { 1, 0 };
	desc.Scaling		= DXGI_SCALING::DXGI_SCALING_STRETCH;
	desc.Stereo			= false;
	desc.SwapEffect		= DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Width			= WINDOW_X;

	//�X���b�v�`�F�C������
	result = swap.factory->CreateSwapChainForHwnd(com.queue, win.lock()->GetWindowHandle(), &desc, nullptr, nullptr, (IDXGISwapChain1**)(&swap.swapChain));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�X���b�v�`�F�C���̐����F���s\n"));
		return result;
	}

	//�o�b�N�o�b�t�@���ۑ�
	swap.bufferCnt = desc.BufferCount;

	return result;
}

// �����_�[�^�[�Q�b�g�p�q�[�v�̐���
HRESULT Device::CreateRenderHeap(void)
{
	//�q�[�v�ݒ�p�\����
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask			= 0;
	desc.NumDescriptors		= swap.bufferCnt;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&render.heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�����_�[�^�[�Q�b�g�p�q�[�v�̐����F���s\n"));
		return result;
	}

	//�q�[�v�T�C�Y�ݒ�
	render.size = dev->GetDescriptorHandleIncrementSize(desc.Type);

	return result;
}

// �����_�[�^�[�Q�b�g�̐���
HRESULT Device::CreateRenderTarget(void)
{
	if (CreateRenderHeap() != S_OK)
	{
		OutputDebugString(_T("\n�����_�[�^�[�Q�b�g�p�q�[�v�̐����F���s\n"));
		return S_FALSE;
	}

	//�����_�[�^�[�Q�b�g�ݒ�p�\����
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension			= D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice		= 0;
	desc.Texture2D.PlaneSlice	= 0;

	//�z��̃������m��
	render.resource.resize(swap.bufferCnt);

	//�擪�n���h���擾
	D3D12_CPU_DESCRIPTOR_HANDLE handle = render.heap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < render.resource.size(); i++)
	{
		//�o�b�t�@�̎擾
		result = swap.swapChain->GetBuffer(i, IID_PPV_ARGS(&render.resource[i]));
		if (FAILED(result))
		{
			OutputDebugString(_T("\n�����_�[�^�[�Q�b�g�̐����F���s\n"));
			return result;
		}

		//�����_�[�^�[�Q�b�g����
		dev->CreateRenderTargetView(render.resource[i], &desc, handle);

		//�n���h���̈ʒu���ړ�
		handle.ptr += render.size;
	}

	return result;
}

// �[�x�X�e���V���p�q�[�v�̐���
HRESULT Device::CreateDepthHeap(void)
{
	//�q�[�v�ݒ�p�\����
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask			= 0;
	desc.NumDescriptors		= swap.bufferCnt;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	//�[�x�X�e���V���p�q�[�v����
	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&depth.heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�[�x�X�e���V���p�q�[�v�̐����F���s\n"));
		return result;
	}

	//�q�[�v�T�C�Y�ݒ�
	depth.size = dev->GetDescriptorHandleIncrementSize(desc.Type);
	return result;
}

// �[�x�X�e���V���̐���
HRESULT Device::CreateDepthStencil(void)
{
	if (CreateDepthHeap() != S_OK)
	{
		OutputDebugString(_T("\n�[�x�X�e���V���p�q�[�v�̐����F���s\n"));
		return S_FALSE;
	}

	//�q�[�v�v���p�e�B�ݒ�p�\���̂̐ݒ�
	D3D12_HEAP_PROPERTIES prop = {};
	prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.CreationNodeMask		= 1;
	prop.MemoryPoolPreference	= D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	prop.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	prop.VisibleNodeMask		= 1;

	//���\�[�X�ݒ�p�\���̂̐ݒ�
	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension				= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Alignment				= 0;
	rDesc.Width					= WINDOW_X;
	rDesc.Height				= WINDOW_Y;
	rDesc.DepthOrArraySize		= 1;
	rDesc.MipLevels				= 0;
	rDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	rDesc.SampleDesc.Count		= 1;
	rDesc.SampleDesc.Quality	= 0;
	rDesc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	rDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;

	//�N���A�l�ݒ�p�\���̂̐ݒ�
	D3D12_CLEAR_VALUE clear = {};
	clear.Format				= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	clear.DepthStencil.Depth	= 1.0f;
	clear.DepthStencil.Stencil	= 0;

	//�[�x�X�e���V���p���\�[�X����
	result = dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear, IID_PPV_ARGS(&depth.resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�[�x�X�e���V���p���\�[�X�̐����F���s\n"));
		return result;
	}

	//�[�x�X�e���V���r���[�ݒ�p�\���̂̐ݒ�
	D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
	desc.Format					= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	desc.ViewDimension			= D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Flags					= D3D12_DSV_FLAGS::D3D12_DSV_FLAG_NONE;

	//�[�x�X�e���V���r���[����
	dev->CreateDepthStencilView(depth.resource, &desc, depth.heap->GetCPUDescriptorHandleForHeapStart());

	return result;
}

// �t�F���X�̐���
HRESULT Device::CreateFence(void)
{
	if (dev == nullptr)
	{
		OutputDebugString(_T("\n�f�o�C�X����������Ă��܂���\n"));
		return S_FALSE;
	}

	//�t�F���X����
	result = dev->CreateFence(fen.fenceCnt, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fen.fence));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�t�F���X�̐����F���s\n"));
		return result;
	}

	//�t�F���X�l�X�V
	fen.fenceCnt = 1;

	//�t�F���X�C�x���g����
	fen.fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (fen.fenceEvent == nullptr)
	{
		OutputDebugString(_T("\n�t�F���X�C�x���g�̐����F���s\n"));
		return S_FALSE;
	}

	return result;
}

// �V�O�l�`���̃V���A���C�Y
HRESULT Device::Serialize(void)
{
	// �f�B�X�N���v�^�����W�̐ݒ�.
	D3D12_DESCRIPTOR_RANGE range[2];
	SecureZeroMemory(&range, sizeof(range));

	//���[�g�p�����[�^�̐ݒ�.
	D3D12_ROOT_PARAMETER param[2];
	SecureZeroMemory(&param, sizeof(param));

	//�萔�o�b�t�@�p
	range[0].RangeType								= D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[0].NumDescriptors							= 1;
	range[0].BaseShaderRegister						= 0;
	range[0].RegisterSpace							= 0;
	range[0].OffsetInDescriptorsFromTableStart		= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	param[0].ParameterType							= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param[0].ShaderVisibility						= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	param[0].DescriptorTable.NumDescriptorRanges	= 1;
	param[0].DescriptorTable.pDescriptorRanges		= &range[0];

	//�e�N�X�`���p
	range[1].RangeType								= D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[1].NumDescriptors							= 1;
	range[1].BaseShaderRegister						= 0;
	range[1].RegisterSpace							= 0;
	range[1].OffsetInDescriptorsFromTableStart		= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	param[1].ParameterType							= D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param[1].ShaderVisibility						= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;
	param[1].DescriptorTable.NumDescriptorRanges	= 1;
	param[1].DescriptorTable.pDescriptorRanges		= &range[1];

	//�ÓI�T���v���[�̐ݒ�
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter									= D3D12_FILTER::D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampler.AddressU								= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV								= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW								= D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias								= 0;
	sampler.MaxAnisotropy							= 0;
	sampler.ComparisonFunc							= D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor								= D3D12_STATIC_BORDER_COLOR::D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD									= 0.0f;
	sampler.MaxLOD									= D3D12_FLOAT32_MAX;
	sampler.ShaderRegister							= 0;
	sampler.RegisterSpace							= 0;
	sampler.ShaderVisibility						= D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

	//���[�g�V�O�l�`���ݒ�p�\���̂̐ݒ�
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters								= _countof(param);
	desc.pParameters								= param;
	desc.NumStaticSamplers							= 1;
	desc.pStaticSamplers							= &sampler;
	desc.Flags										= D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//���[�g�V�O�l�`���̃V���A���C�Y��
	result = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &sig.signature, &sig.error);

	return result;
}

// ���[�g�V�O�l�`���̐���
HRESULT Device::CreateRootSigunature(void)
{
	if (Serialize() != S_OK)
	{
		OutputDebugString(_T("\n�V���A���C�Y���F���s\n"));
		return S_FALSE;
	}

	//���[�g�V�O�l�`������
	result = dev->CreateRootSignature(0, sig.signature->GetBufferPointer(), sig.signature->GetBufferSize(), IID_PPV_ARGS(&sig.rootSignature));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n���[�g�V�O�l�`���̐����F���s\n"));
		return result;
	}

	return result;
}

// �V�F�[�_�̃R���p�C��
HRESULT Device::ShaderCompile(LPCWSTR fileName)
{
	//���_�V�F�[�_�̃R���p�C��
	result = D3DCompileFromFile(fileName, nullptr, nullptr, "TextureVS", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pipe.vertex, &sig.error);
	if (FAILED(result))
	{
		OutputDebugString(_T("\n���_�V�F�[�_�R���p�C���F���s\n"));
		return result;
	}

	//�s�N�Z���V�F�[�_�̃R���p�C��
	result = D3DCompileFromFile(fileName, nullptr, nullptr, "TexturePS", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pipe.pixel, &sig.error);
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�s�N�Z���V�F�[�_�R���p�C���F���s\n"));
		return result;
	}

	return result;
}

// �p�C�v���C���̐���
HRESULT Device::CreatePipeline(void)
{
	//���_���C�A�E�g�ݒ�p�\���̂̐ݒ�
	D3D12_INPUT_ELEMENT_DESC input[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//���X�^���C�U�[�X�e�[�g�ݒ�p�\���̂̐ݒ�
	D3D12_RASTERIZER_DESC rasterizer = {};
	rasterizer.FillMode						= D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	rasterizer.CullMode						= D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	rasterizer.FrontCounterClockwise		= FALSE;
	rasterizer.DepthBias					= D3D12_DEFAULT_DEPTH_BIAS;
	rasterizer.DepthBiasClamp				= D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizer.SlopeScaledDepthBias			= D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizer.DepthClipEnable				= TRUE;
	rasterizer.MultisampleEnable			= FALSE;
	rasterizer.AntialiasedLineEnable		= FALSE;
	rasterizer.ForcedSampleCount			= 0;
	rasterizer.ConservativeRaster			= D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//�����_�[�^�[�Q�b�g�u�����h�ݒ�p�\����
	D3D12_RENDER_TARGET_BLEND_DESC renderBlend = {};
	renderBlend.BlendEnable					= FALSE;
	renderBlend.BlendOp						= D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
	renderBlend.BlendOpAlpha				= D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
	renderBlend.DestBlend					= D3D12_BLEND::D3D12_BLEND_ZERO;
	renderBlend.DestBlendAlpha				= D3D12_BLEND::D3D12_BLEND_ZERO;
	renderBlend.LogicOp						= D3D12_LOGIC_OP::D3D12_LOGIC_OP_NOOP;
	renderBlend.LogicOpEnable				= FALSE;
	renderBlend.RenderTargetWriteMask		= D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;
	renderBlend.SrcBlend					= D3D12_BLEND::D3D12_BLEND_ONE;
	renderBlend.SrcBlendAlpha				= D3D12_BLEND::D3D12_BLEND_ONE;

	//�u�����h�X�e�[�g�ݒ�p�\����
	D3D12_BLEND_DESC descBS = {};
	descBS.AlphaToCoverageEnable			= TRUE;
	descBS.IndependentBlendEnable			= FALSE;
	for (UINT i = 0; i < swap.bufferCnt; i++)
	{
		descBS.RenderTarget[i]				= renderBlend;
	}

	//�p�C�v���C���X�e�[�g�ݒ�p�\����
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.InputLayout						= { input, _countof(input) };
	desc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.pRootSignature						= sig.rootSignature;
	desc.VS									= CD3DX12_SHADER_BYTECODE(pipe.vertex);
	desc.PS									= CD3DX12_SHADER_BYTECODE(pipe.pixel);
	desc.RasterizerState					= rasterizer;
	desc.BlendState							= descBS;
	desc.DepthStencilState.DepthEnable		= true;
	desc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
	desc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_LESS;
	desc.DepthStencilState.StencilEnable	= FALSE;
	desc.SampleMask							= UINT_MAX;
	desc.NumRenderTargets					= 1;
	desc.RTVFormats[0]						= DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat							= DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count					= 1;

	//�p�C�v���C������
	result = dev->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipe.pipeline));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�p�C�v���C���̐����F���s\n"));
		return result;
	}

	return result;
}

// �萔�o�b�t�@�p�q�[�v�̐���
HRESULT Device::CreateConstantHeap(void)
{
	//�萔�o�b�t�@�ݒ�p�\���̂̐ݒ�
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors		= 2;
	desc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	//�q�[�v����
	result = dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&con.heap));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�萔�o�b�t�@�p�q�[�v�̐����F���s\n"));
		return result;
	}

	//�q�[�v�T�C�Y�ݒ�
	con.size = dev->GetDescriptorHandleIncrementSize(desc.Type);

	return result;
}

// �萔�o�b�t�@�̐���
HRESULT Device::CreateConstant(void)
{
	if (CreateConstantHeap() != S_OK)
	{
		OutputDebugString(_T("\n�萔�o�b�t�@�p�q�[�v�̐����F���s\n"));
		return S_FALSE;
	}

	//�q�[�v�ݒ�p�\���̂̐ݒ�
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type					= D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference	= D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask		= 1;
	prop.VisibleNodeMask		= 1;

	//���\�[�X�ݒ�p�\���̂̐ݒ�
	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension				= D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Width					= ((sizeof(WVP) + 0xff) &~0xff);
	rDesc.Height				= 1;
	rDesc.DepthOrArraySize		= 1;
	rDesc.MipLevels				= 1;
	rDesc.Format				= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count		= 1;
	rDesc.Flags					= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;
	rDesc.Layout				= D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//���\�[�X����
	result = dev->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&con.resource));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�萔�o�b�t�@�p���\�[�X�̐����F���s\n"));
		return result;
	}

	//�萔�o�b�t�@�r���[�ݒ�p�\���̂̐ݒ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation			= con.resource->GetGPUVirtualAddress();
	desc.SizeInBytes			= (sizeof(WVP) + 0xff) &~0xff;

	//�萔�o�b�t�@�r���[����
	dev->CreateConstantBufferView(&desc, con.heap->GetCPUDescriptorHandleForHeapStart());

	//���M�͈�
	D3D12_RANGE range = { 0, 0 };

	//�}�b�s���O
	result = con.resource->Map(0, &range, (void**)(&con.data));
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�萔�o�b�t�@�p���\�[�X�̃}�b�s���O�F���s\n"));
		return result;
	}

	//�R�s�[
	memcpy(con.data, &wvp, sizeof(DirectX::XMMATRIX));

	return result;
}

// �r���[�|�[�g�̃Z�b�g
void Device::SetViewPort(void)
{
	viewPort.TopLeftX	= 0;
	viewPort.TopLeftY	= 0;
	viewPort.Width		= WINDOW_X;
	viewPort.Height		= WINDOW_Y;
	viewPort.MinDepth	= 0;
	viewPort.MaxDepth	= 1;
}

// �V�U�[�̃Z�b�g
void Device::SetScissor(void)
{
	scissor.left	= 0;
	scissor.right	= WINDOW_X;
	scissor.top		= 0;
	scissor.bottom	= WINDOW_Y;
}

// �o���A�̍X�V
void Device::Barrier(D3D12_RESOURCE_STATES befor, D3D12_RESOURCE_STATES affter)
{
	barrier.Type					= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource	= render.resource[swap.swapChain->GetCurrentBackBufferIndex()];
	barrier.Transition.StateBefore	= befor;
	barrier.Transition.StateAfter	= affter;
	barrier.Transition.Subresource	= D3D12_RESOURCE_BARRIER_FLAG_NONE;

	//�o���A�ݒu
	com.list->ResourceBarrier(1, &barrier);
}

// �ҋ@����
void Device::Wait(void)
{
	//�t�F���X�l�X�V
	fen.fenceCnt++;

	//�t�F���X�l��ύX
	result = com.queue->Signal(fen.fence, fen.fenceCnt);
	if (FAILED(result))
	{
		OutputDebugString(_T("\n�t�F���X�l�̍X�V�F���s\n"));
		return;
	}

	//������ҋ@(�|�[�����O)
	while (fen.fence->GetCompletedValue() != fen.fenceCnt)
	{
		/*auto a = fence.fenceCnt;
		std::stringstream s;
		s << a;
		OutputDebugStringA(s.str().c_str());*/

		//�t�F���X�C�x���g�̃Z�b�g
		result = fen.fence->SetEventOnCompletion(fen.fenceCnt, fen.fenceEvent);
		if (FAILED(result))
		{
			OutputDebugString(_T("\n�t�F���X�C�x���g�̃Z�b�g�F���s\n"));
			return;
		}

		//�t�F���X�C�x���g�̑ҋ@
		WaitForSingleObject(fen.fenceEvent, INFINITE);
	}
}

// �������
void Device::Release(ID3D12Resource * resource)
{
	if (resource != nullptr)
	{
		resource->Release();
	}
}

// �������
void Device::Release(ID3D12DescriptorHeap * heap)
{
	if (heap != nullptr)
	{
		heap->Release();
	}
}

// �������
void Device::Release(ID3DBlob * blob)
{
	if (blob != nullptr)
	{
		blob->Release();
	}
}

// �������
void Device::Release(ID3D12PipelineState * pipe)
{
	if (pipe != nullptr)
	{
		pipe->Release();
	}
}

// �������
void Device::Release(ID3D12RootSignature * signature)
{
	if (signature != nullptr)
	{
		signature->Release();
	}
}

// �������
void Device::Release(ID3D12Fence * fence)
{
	if (fence != nullptr)
	{
		fence->Release();
	}
}

// �������
void Device::Release(IDXGIFactory4 * factory)
{
	if (factory != nullptr)
	{
		factory->Release();
	}
}

// �������
void Device::Release(IDXGISwapChain3 * swap)
{
	if (swap != nullptr)
	{
		swap->Release();
	}
}

// �������
void Device::Release(ID3D12CommandQueue * queue)
{
	if (queue != nullptr)
	{
		queue->Release();
	}
}

// �������
void Device::Release(ID3D12GraphicsCommandList * list)
{
	if (list != nullptr)
	{
		list->Release();
	}
}

// �������
void Device::Release(ID3D12CommandAllocator * allocator)
{
	if (allocator != nullptr)
	{
		allocator->Release();
	}
}

// �������
void Device::Release(ID3D12Device * dev)
{
	if (dev != nullptr)
	{
		dev->Release();
	}
}