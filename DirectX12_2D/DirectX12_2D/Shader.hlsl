Texture2D<float4> tex : register(t0);
SamplerState      smp : register(s0);

//ワールドビュープロジェクション
cbuffer wvp : register(b0)
{
	//ワールド行列
	float4x4 world;
	//ビュープロジェクション行列
	float4x4 viewProjection;
}

//出力
struct Out
{
	//システム座標
	float4 svpos  : SV_POSITION;
	//座標
	float4 pos    : POSITION;
	//uv値
	float2 uv     : TEXCOORD;
};

//入力
struct VSInput
{
	//座標
	float4 pos    : POSITION;
	//uv
	float2 uv     : TEXCOORD;
};

//テクスチャ用頂点シェーダ
Out BasicVS(VSInput input)
{
	/*座標補正
	pos.xy = float2(-1, 1) + (pos.xy / float2((640 / 2), -(480 / 2)));*/

	//input.pos = mul(mul(viewProjection, world), input.pos);

	Out o;
	o.svpos     = input.pos;
	o.pos       = input.pos;
	o.uv        = input.uv;
	
	return o;
}

//テクスチャ用ピクセルシェーダ
float4 BasicPS(Out o) : SV_TARGET
{
	//return float4(tex.Sample(smp, o.uv).rgb, 0);
    float4 t = tex.Sample(smp, o.uv);
    if (t.a <= 0.0)
    {
        discard;
    }
        
    return t;
}