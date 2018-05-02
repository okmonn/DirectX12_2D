#pragma once

#ifdef EXPORT_MAIN
#define EXTERN_MAIN
#else
#define EXTERN_MAIN extern
#endif

#include "Window.h"
#include "Texture.h"
#include "Input.h"
#include "Player.h"
#include "Device.h"

// インスタンス処理
void Create(void);

// メモリ解放処理
void Destroy(void);


// ウィンドウクラス参照
EXTERN_MAIN std::shared_ptr<Window>win;

// インプットクラス参照
EXTERN_MAIN std::shared_ptr<Input>input;

// デバイスクラス参照
EXTERN_MAIN std::shared_ptr<Device>dev;

// テクスチャ
EXTERN_MAIN std::shared_ptr<Texture>tex;

// プレイヤー
EXTERN_MAIN std::shared_ptr<Player>pl;