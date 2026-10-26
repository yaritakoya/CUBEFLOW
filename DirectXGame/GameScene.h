#pragma once
#include "KamataEngine.h"
#include <array>
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

	// 画面遷移が完了した時点で開始演出を始める。
	void BeginStartCountdown();
	bool IsStartCountdownComplete() const { return isStartCountdownReady_ && !isStartCountdownActive_; }

	// 描画
	void Draw();

	// ブロック用のワールドトランスフォーム
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

private:
	// 3Dモデルデータ
	Model* skySphere_ = nullptr;
	uint32_t skyTexture_ = 0;
	WorldTransform skySphereTransform_;
	Model* modelBlock_ = nullptr;
	Model* modelPrediction_ = nullptr;
	ObjectColor hazardBlockColor_;
	float playerDamageTimer_ = 0.0f;
	std::array<WorldTransform, 10> predictionTransforms_{};
	size_t predictionCount_ = 0;

	// ワールドトランスフォーム
	WorldTransform worldTransform_;
	Vector3 playerStagePosition_ = {9.0f, 9.0f, 0.0f};
	Vector3 playerStageVelocity_ = {};
	bool isPlayerGrounded_ = false;
	int playerJumpCount_ = 0;

	// カメラ
	Camera camera_;

	// デバックカメラ有効
	bool isDebugCameraActive_ = false;

	// デバックカメラの生成
	DebugCamera* debugCamera_ = nullptr;

	// アニメーションの種類
	BlockAnimation blockAnimation_ = BlockAnimation::Wave;
	bool isWaveReverse_ = false;

	// アニメーションタイマー
	float animationTimer_ = 0.0f;
	// 開始演出中はゲームの更新を止める。
	float startCountdownTimer_ = 0.0f;
	bool isStartCountdownActive_ = true;
	bool isStartCountdownReady_ = false;
	std::array<Sprite*, 4> startCountdownSprites_{};
	Sprite* startCountdownGlowSprite_ = nullptr;

	// ステージ回転（Z軸まわり、ラジアン）
	float stageRotation_ = 0.0f;
	float targetStageRotation_ = 0.0f;

	// ランダムアニメーション用のタイマー
	float randomTimer_ = 0.0f;

	// ランダムで動かすブロックの番号
	std::array<int, 4> randomIndices_{};
	void Release();

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
	std::vector<WorldTransform*> floatingBlocks_;
};
