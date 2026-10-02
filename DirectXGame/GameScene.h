#pragma once
#include "KamataEngine.h"
#include <vector>

using namespace KamataEngine;

// ゲームシーン
class GameScene {
public:
	enum class BlockAnimation { Wave, OddEven, Random, Center };

	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// ブロック用のワールドトランスフォーム
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

private:
	// 3Dモデルデータ
	Model* modelBlock_ = Model::CreateFromOBJ("cube");

	// ワールドトランスフォーム
	WorldTransform worldTransform_;

	// カメラ
	Camera camera_;

	// デバックカメラ有効
	bool isDebugCameraActive_ = false;

	// デバックカメラの生成
	DebugCamera* debugCamera_ = nullptr;

	// アニメーションの種類
	BlockAnimation blockAnimation_ = BlockAnimation::Wave;

	// アニメーションタイマー
	float animationTimer_ = 0.0f;

	// ランダムアニメーション用のタイマー
	float randomTimer_ = 0.0f;

	// ランダムで動かすブロックの番号
	int randomIndex_ = 0;

	// ランダムで動かすブロックの列
	int randomLine_ = 0;

	// 2番のアニメーションで動かすブロックの列
	int oddEvenLine_ = 0;

	// 2番のアニメーション用タイマー
	float oddEvenTimer_ = 0.0f;

	// 1番のアニメーションで動かすブロックの列
	int waveLine_ = 0;

	// 1番のアニメーション用タイマー
	float waveTimer_ = 0.0f;

	// 1番のアニメーションで現在動かしているブロック番号
	//int waveIndex_ = 0;

	// 追加ブロック列を生成
	void InitializeBlockLine1();
	void InitializeBlockLine2();
	void InitializeBlockLine3();

	// 追加ブロック列
	std::vector<WorldTransform*> blockLine1_;
	std::vector<WorldTransform*> blockLine2_;
	std::vector<WorldTransform*> blockLine3_;
};