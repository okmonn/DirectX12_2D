#pragma once
#include <d3d12.h>
#include <dsound.h>
#include <string>
#include <map>
#include <memory>

class Window;

class Sound
{
public:
	// コンストラクタ
	Sound(std::weak_ptr<Window>win);
	// デストラクタ
	~Sound();

	// サウンドバッファの生成
	HRESULT LoadWAV(USHORT* index, std::string fileName);

	// サウンドの再生
	HRESULT Play(USHORT* index, DWORD type);

	// サウンドの停止
	HRESULT Stop(USHORT* index);

	// サウンドデータの消去
	void DeleteSoundWAV(USHORT* index);

private:
	// 初期処理
	HRESULT CreateDirectSound(void);


	// プライマリサウンドバッファの生成
	HRESULT CreatePrimaryBuffer(void);


	// ウィンドウクラス参照
	std::weak_ptr<Window>win;

	// 参照結果
	HRESULT result;

	// セカンダリーバッファ源
	std::map<std::string, LPDIRECTSOUNDBUFFER>origin;

	// セカンダリーバッファ
	std::map<USHORT*, LPDIRECTSOUNDBUFFER>bgm;

	// プライマリサウンドバッファ
	LPDIRECTSOUNDBUFFER buffer;

	// ダイレクトサウンド
	LPDIRECTSOUND8 sound;
};