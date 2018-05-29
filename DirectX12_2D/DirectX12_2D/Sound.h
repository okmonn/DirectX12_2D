#pragma once
#include "Input.h"
#include <d3d12.h>
#include <dsound.h>
#include <memory>

class Window;

class Sound
{
public:
	// コンストラクタ
	Sound(std::weak_ptr<Window>win, std::weak_ptr<Input>input);
	// デストラクタ
	~Sound();

	// 初期処理
	HRESULT CreateDirectSound(void);

	// プライマリサウンドバッファの生成
	HRESULT CreatePrimaryBuffer(void);

	// サウンドバッファの生成
	HRESULT LoadWAV(LPDIRECTSOUNDBUFFER *wavData, const char *fileName);

	// サウンドの再生
	HRESULT Play(LPDIRECTSOUNDBUFFER wavData, DWORD type);

	// 処理
	void UpData(void);

private:
	// ウィンドウクラス参照
	std::weak_ptr<Window>win;

	// インプットクラス参照
	std::weak_ptr<Input>input;

	// 参照結果
	HRESULT result;

	// セカンダリーバッファ
	LPDIRECTSOUNDBUFFER bgm;

	// プライマリサウンドバッファ
	LPDIRECTSOUNDBUFFER buffer;

	// ダイレクトサウンド
	LPDIRECTSOUND8 sound;
};