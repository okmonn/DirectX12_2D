Texture2D<float4> tex : register(t0);
Texture2D<float4> spa : register(t1);
SamplerState      smp : register(s0);

//���[���h�r���[�v���W�F�N�V����
cbuffer wvp : register(b0)
{
	//���[���h�s��
	float4x4 world;
	//�r���[�v���W�F�N�V�����s��
	float4x4 viewProjection;
}

//�}�e���A��
cbuffer mat : register(b1)
{
	//��{�F
	float3 diffuse;
	//�e�N�X�`���Ή��t���O
	bool existTexture;
}

//�{�[��
cbuffer born : register(b2)
{
	//�{�[��
	matrix borns[256];
}

//�o��
struct Out
{
	//�V�X�e�����W
	float4 svpos  : SV_POSITION;
	//���W
	float4 pos    : POSITION;
	//uv�l
	float2 uv     : TEXCOORD;
	//�@��
	float3 normal : NORMAL;
	//�F
	float4 color  : COLOR;
};

//����
struct VSInput
{
	//���W
	float4 pos        : POSITION;
	//�@��
	float4 normal     : NORMAL;
	//uv
	float2 uv         : TEXCOORD;
	//�{�[��
	min16uint2 bornID : BORN;
	//�E�F�C�g
	min16uint weight  : WEIGHT;
};

//�e�N�X�`���p���_�V�F�[�_
Out TextureVS(VSInput input)
{
	/*���W�␳
	pos.xy = float2(-1, 1) + (pos.xy / float2((640 / 2), -(480 / 2)));*/

	input.pos = mul(mul(viewProjection, world), input.pos);

	Out o;
	o.svpos = input.pos;
	o.pos = input.pos;
	o.uv = input.normal;
	o.normal = input.normal;
	o.color = float4(1, 0, 0, 1);

	return o;
}

//�e�N�X�`���p�s�N�Z���V�F�[�_
float4 TexturePS(Out o) : SV_TARGET
{
	return float4(tex.Sample(smp, o.uv).abg, 1);
}


//���f���p���_�V�F�[�_�[
Out ModelVS(VSInput input)
{
	float w1 = (float)(input.weight) / 100.0f;
	float w2 = (float)(1.0f - w1);
	matrix m = borns[input.bornID[0]] * w1 + borns[input.bornID[1]] * w2;

	m = mul(world, m);

	input.pos = mul(mul(viewProjection, m), input.pos);

	Out o;
	o.svpos = input.pos;
	o.pos = input.pos;
	o.uv = input.uv;
	o.normal = mul(world, input.normal);
	if (input.bornID[0] == 19 || input.bornID[1] == 19)
	{
		o.color = float4(1, 0, 0, 1) * (float(input.weight) / 100.0);
	}

	return o;
}

//���f���p�s�N�Z���V�F�[�_�[
float4 ModelPS(Out o) : SV_TARGET
{
	//�����ւ̃x�N�g��
	float3 light = normalize(float3(-1, 1, -1));
	//����
	float brightness = dot(o.normal, light);
	//�F
	float3 color = (existTexture == true ? tex.Sample(smp, o.uv).rgb : diffuse);

	return float4(color * brightness, 1);
}