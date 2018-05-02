Texture2D<float4> tex : register(t0);
SamplerState      smp : register(s0);

//���[���h�r���[�v���W�F�N�V����
cbuffer wvp : register(b0)
{
	//���[���h�s��
	float4x4 world;
	//�r���[�v���W�F�N�V�����s��
	float4x4 viewProjection;
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
};

//����
struct VSInput
{
	//���W
	float4 pos    : POSITION;
	//uv
	float2 uv     : TEXCOORD;
};

//�e�N�X�`���p���_�V�F�[�_
Out TextureVS(VSInput input)
{
	/*���W�␳
	pos.xy = float2(-1, 1) + (pos.xy / float2((640 / 2), -(480 / 2)));*/

	//input.pos = mul(mul(viewProjection, world), input.pos);

	Out o;
	o.svpos = input.pos;
	o.pos = input.pos;
	o.uv = input.uv;
	
	return o;
}

//�e�N�X�`���p�s�N�Z���V�F�[�_
float4 TexturePS(Out o) : SV_TARGET
{
	//return float4(tex.Sample(smp, o.uv).rgb, 0);
    float4 t = tex.Sample(smp, o.uv);
    if (t.a <= 0.0)
    {
        discard;
    }
        
    return t;
}