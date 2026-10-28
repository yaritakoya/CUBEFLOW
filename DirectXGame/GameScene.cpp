#include "GameScene.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

// デストラクタ
GameScene::~GameScene() {
	Release();
}

void GameScene::Release() {
	delete skySphere_;
	skySphere_ = nullptr;
	delete modelBlock_;
	modelBlock_ = nullptr;
	delete modelPrediction_;
	modelPrediction_ = nullptr;
	delete debugCamera_;
	debugCamera_ = nullptr;
	for (Sprite* sprite : startCountdownSprites_) {
		delete sprite;
	}
	startCountdownSprites_.fill(nullptr);
	delete startCountdownGlowSprite_;
	startCountdownGlowSprite_ = nullptr;
	delete gameOverSprite_;
	gameOverSprite_ = nullptr;
	delete gameOverDimSprite_;
	gameOverDimSprite_ = nullptr;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	for (WorldTransform* block : blockLine1_) {
		delete block;
	}
	blockLine1_.clear();

	for (WorldTransform* block : blockLine2_) {
		delete block;
	}
	blockLine2_.clear();

	for (WorldTransform* block : blockLine3_) {
		delete block;
	}
	blockLine3_.clear();

	for (WorldTransform* block : floatingBlocks_) {
		delete block;
	}
	floatingBlocks_.clear();
}

// ブロックBの生成
void GameScene::InitializeBlockLine1() {
	const uint32_t kNumBlock = 10;
	const float kBlockWidth = 2.0f;

	for (uint32_t i = 0; i < kNumBlock; i++) {
		WorldTransform* block = new WorldTransform();
		block->Initialize();

		block->translation_.x = kBlockWidth * i;
		block->translation_.y = 20.0f;
		block->translation_.z = 0.0f;

		block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);

		block->TransferMatrix();
		blockLine1_.push_back(block);
	}
}

// ブロックCの生成
void GameScene::InitializeBlockLine2() {
	const uint32_t kNumBlock = 10;
	const float kBlockHeight = 2.0f;

	for (uint32_t i = 0; i < kNumBlock; i++) {
		WorldTransform* block = new WorldTransform();
		block->Initialize();

		block->translation_.x = 20.0f;
		block->translation_.y = kBlockHeight * i;
		block->translation_.z = 0.0f;

		block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);

		block->TransferMatrix();
		blockLine2_.push_back(block);
	}
}

// ブロックDの生成
void GameScene::InitializeBlockLine3() {
	const uint32_t kNumBlock = 10;
	const float kBlockHeight = 2.0f;

	for (uint32_t i = 0; i < kNumBlock; i++) {
		WorldTransform* block = new WorldTransform();
		block->Initialize();

		block->translation_.x = -2.0f;
		block->translation_.y = kBlockHeight * i;
		block->translation_.z = 0.0f;

		block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);

		block->TransferMatrix();
		blockLine3_.push_back(block);
	}
}

// 初期化
void GameScene::Initialize() {
	// 再入場時は前回シーンのリソースを解放して、初期状態から作り直す。
	Release();
	animationTimer_ = 0.0f;
	skyTexture_ = 0;
	blockAnimation_ = BlockAnimation::Wave;
	stageRotation_ = 0.0f;
	targetStageRotation_ = 0.0f;
	isPlayerGrounded_ = false;
	isDebugCameraActive_ = false;
	modelBlock_ = Model::Create();
	modelPrediction_ = Model::Create();
	hazardBlockColor_.Initialize();
	hazardBlockColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});
	playerDamageTimer_ = 0.0f;
	deathTimer_ = -1.0f;
	isGameOver_ = false;
	gameOverTimer_ = 0.0f;
	isReturnToTitleRequested_ = false;
	for (BurstParticle& particle : burstParticles_) {
		particle.transform.Initialize();
		particle.life = 0.0f;
	}
	if (modelPrediction_) {
		modelPrediction_->SetAlpha(0.3f);
	}
	for (WorldTransform& predictionTransform : predictionTransforms_) {
		predictionTransform.Initialize();
	}
	predictionCount_ = 0;
	startCountdownTimer_ = 0.0f;
	isStartCountdownActive_ = true;
	isStartCountdownReady_ = false;
	const char* countdownTextures[] = {"countdown3.png", "countdown2.png", "countdown1.png", "countdownStart.png"};
	for (size_t i = 0; i < startCountdownSprites_.size(); ++i) {
		const uint32_t texture = TextureManager::Load(countdownTextures[i]);
		Sprite*& sprite = startCountdownSprites_[i];
		sprite = Sprite::Create(texture, {0.0f, 0.0f});
		if (sprite) {
			sprite->SetSize(i == 3 ? Vector2{800.0f, 245.0f} : Vector2{280.0f, 280.0f});
		}
	}
	startCountdownGlowSprite_ = Sprite::Create(TextureManager::Load("countdownStart.png"), {0.0f, 0.0f});
	if (startCountdownGlowSprite_) {
		startCountdownGlowSprite_->SetSize({860.0f, 265.0f});
	}
	gameOverSprite_ = Sprite::Create(TextureManager::Load("gameOver.png"), {349.5f, 266.5f});
	if (gameOverSprite_) {
		gameOverSprite_->SetSize({581.0f, 187.0f});
		gameOverSprite_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
	}
	gameOverDimSprite_ = Sprite::Create(TextureManager::Load("black1x1.png"), {0.0f, 0.0f});
	if (gameOverDimSprite_) {
		gameOverDimSprite_->SetSize({1280.0f, 720.0f});
		gameOverDimSprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
	}

	// プレイヤーもブロックと同じ3Dモデルで描画する。
	worldTransform_.Initialize();
	playerStagePosition_ = {8.0f, 9.0f, 0.0f};
	worldTransform_.translation_ = playerStagePosition_;
	worldTransform_.scale_ = {0.75f, 0.75f, 0.75f};
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
	playerStageVelocity_ = {};
	playerJumpCount_ = 0;

	camera_.Initialize();
	// ステージ外枠（X/Y: -2〜20）の中心を見るように配置する。
	camera_.translation_ = {9.0f, 9.0f, -31.0f};
	camera_.UpdateMatrix();

	// タイトルと同じ、照明の影響を受けない内向き天球を設定する。
	skySphere_ = Model::CreateSphere(64, 128);
	if (skySphere_) {
		for (const auto& mesh : skySphere_->GetMeshes()) {
			if (mesh && mesh->GetMaterial()) {
				Material* material = mesh->GetMaterial();
				material->ambient_ = {1.0f, 1.0f, 1.0f};
				material->diffuse_ = {0.0f, 0.0f, 0.0f};
				material->specular_ = {0.0f, 0.0f, 0.0f};
			}
		}
	}
	skySphereTransform_.Initialize();
	skySphereTransform_.scale_ = {-500.0f, -500.0f, -500.0f};
	skySphereTransform_.translation_ = camera_.translation_;
	skySphereTransform_.matWorld_ = MakeAffineMatrix(skySphereTransform_.scale_, skySphereTransform_.rotation_, skySphereTransform_.translation_);
	skySphereTransform_.TransferMatrix();
	skyTexture_ = TextureManager::Load("caveBrown.png");

	// ブロックAの設定
	const uint32_t kNumBlockVirtcal = 1;
	const uint32_t kNumBlockHorizontal = 10;
	const float kBlockWidth = 2.0f;

	worldTransformBlocks_.resize(kNumBlockVirtcal);

	for (uint32_t i = 0; i < kNumBlockVirtcal; i++) {
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	}

	// ブロックA生成
	for (uint32_t i = 0; i < kNumBlockVirtcal; i++) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; j++) {
			worldTransformBlocks_[i][j] = new WorldTransform();
			worldTransformBlocks_[i][j]->Initialize();

			worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
			worldTransformBlocks_[i][j]->translation_.y = -2.0f;
			worldTransformBlocks_[i][j]->translation_.z = 0.0f;

			worldTransformBlocks_[i][j]->matWorld_ = MakeAffineMatrix(worldTransformBlocks_[i][j]->scale_, worldTransformBlocks_[i][j]->rotation_, worldTransformBlocks_[i][j]->translation_);

			worldTransformBlocks_[i][j]->TransferMatrix();
		}
	}

	// 1番用の初期値
	waveLine_ = rand() % 4;
	isWaveReverse_ = (rand() % 2) != 0;
	waveTimer_ = 0.0f;

	// 2番用の初期値
	oddEvenLine_ = rand() % 4;
	oddEvenTimer_ = 0.0f;

	// 3番用の初期値
	for (int& index : randomIndices_) {
		index = rand() % 10;
	}
	randomTimer_ = 0.0f;

	// ブロックB・C・Dを生成
	InitializeBlockLine1();
	InitializeBlockLine2();
	InitializeBlockLine3();

	// 重力がどの方向を向いても足場に届くよう、中央の上下左右へ対称に配置する。
	for (const Vector3& position : {Vector3{9.0f, 5.0f, 0.0f}, Vector3{9.0f, 13.0f, 0.0f}, Vector3{5.0f, 9.0f, 0.0f}, Vector3{13.0f, 9.0f, 0.0f}}) {
		WorldTransform* block = new WorldTransform();
		block->Initialize();
		block->translation_ = position;
		block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);
		block->TransferMatrix();
		floatingBlocks_.push_back(block);
	}

	// デバッグカメラ
	debugCamera_ = new DebugCamera(1280, 720);
}

void GameScene::BeginStartCountdown() {
	startCountdownTimer_ = 0.0f;
	isStartCountdownReady_ = true;
}

// 更新
void GameScene::Update() {
	// タイトルと同じ速度で天球を回し、常にカメラを中心に置く。
	skySphereTransform_.rotation_.y += 0.02f * 0.025f;
	skySphereTransform_.translation_ = camera_.translation_;
	skySphereTransform_.matWorld_ = MakeAffineMatrix(skySphereTransform_.scale_, skySphereTransform_.rotation_, skySphereTransform_.translation_);
	skySphereTransform_.TransferMatrix();
	if (!isStartCountdownReady_) {
		return;
	}

	// 3、2、1、STARTの演出が終わるまで、物理・入力・ブロックアニメーションを停止する。
	if (isStartCountdownActive_) {
		startCountdownTimer_ += 1.0f / 60.0f;
		if (startCountdownTimer_ >= 3.7f) {
			isStartCountdownActive_ = false;
		}
		return;
	}
	if (deathTimer_ >= 0.0f) {
		deathTimer_ += 1.0f / 60.0f;
		for (BurstParticle& particle : burstParticles_) {
			if (particle.life <= 0.0f) continue;
			particle.life -= 1.0f / 60.0f;
			particle.transform.translation_.x += particle.velocity.x;
			particle.transform.translation_.y += particle.velocity.y;
			particle.velocity.y -= 0.006f;
			const float scale = (std::max)(0.02f, particle.life * 0.42f);
			particle.transform.scale_ = {scale, scale, scale};
			particle.transform.rotation_.z += 0.12f;
			particle.transform.matWorld_ = MakeAffineMatrix(particle.transform.scale_, particle.transform.rotation_, particle.transform.translation_);
			particle.transform.TransferMatrix();
		}
		if (deathTimer_ >= 1.0f) {
			isGameOver_ = true;
			gameOverTimer_ += 1.0f / 60.0f;
			// フェード完了後もしばらく表示を保ち、通常のキーボードキーだけを遷移入力として見る。
			if (gameOverTimer_ >= 3.0f) {
				Input* input = Input::GetInstance();
				for (int key = DIK_ESCAPE; key <= DIK_F12; ++key) {
					if (input->TriggerKey(static_cast<BYTE>(key))) {
						isReturnToTitleRequested_ = true;
						break;
					}
				}
			}
		}
		if (!isGameOver_ || gameOverTimer_ < 1.85f) {
			return;
		}
	}
	playerDamageTimer_ = playerDamageTimer_ > 1.0f / 60.0f ? playerDamageTimer_ - 1.0f / 60.0f : 0.0f;

	// アニメーションタイマーは1フレームごとに0.03進むため、60fps換算の0.8秒分を待つ。
	constexpr float wavePreviewDelay = 1.44f;
	animationTimer_ += 0.03f;

	// Eキーで時計回り、Qキーで逆回転する。
	if (!isGameOver_) {
		if (Input::GetInstance()->TriggerKey(DIK_E)) {
			targetStageRotation_ += 3.141592f * 0.5f;
		}
		if (Input::GetInstance()->TriggerKey(DIK_Q)) {
			targetStageRotation_ -= 3.141592f * 0.5f;
		}
	}

	// 目標角度まで一定速度で回転する。
	const float rotationStep = 0.03f;
	const float rotationDifference = targetStageRotation_ - stageRotation_;
	if (fabsf(rotationDifference) <= rotationStep) {
		stageRotation_ = targetStageRotation_;
	} else {
		stageRotation_ += rotationDifference > 0.0f ? rotationStep : -rotationStep;
	}

	// ステージ回転後の重力方向。ゲームオーバー中は押し戻し判定を止めるが、値は共通で使う。
	const float gravity = 0.02f;
	const float quarterTurn = 3.141592f * 0.5f;
	const float gravityStageRotation = roundf(stageRotation_ / quarterTurn) * quarterTurn;
	const float gravityX = -sinf(gravityStageRotation) * gravity;
	const float gravityY = -cosf(gravityStageRotation) * gravity;
	if (!isGameOver_) {
	// ワールド下向きの重力をステージ座標へ変換して適用する。
	const float playerHalfSize = worldTransform_.scale_.x;
	Input* input = Input::GetInstance();
	float moveDirection = 0.0f;
	if (input->PushKey(DIK_A)) {
		moveDirection -= 1.0f;
	}
	if (input->PushKey(DIK_D)) {
		moveDirection += 1.0f;
	}

	// A/Dの画面左右移動をステージ座標へ変換する。
	const float moveSpeed = 0.12f;
	const float playerStageCosine = cosf(stageRotation_);
	const float playerStageSine = sinf(stageRotation_);
	playerStagePosition_.x += playerStageCosine * moveDirection * moveSpeed;
	playerStagePosition_.y -= playerStageSine * moveDirection * moveSpeed;

	// 接地中の通常ジャンプに加え、空中でもう一度だけジャンプできる。
	if (input->TriggerKey(DIK_SPACE) && (isPlayerGrounded_ || playerJumpCount_ < 2)) {
		const float jumpSpeed = 0.45f;
		playerStageVelocity_.x = sinf(gravityStageRotation) * jumpSpeed;
		playerStageVelocity_.y = cosf(gravityStageRotation) * jumpSpeed;
		isPlayerGrounded_ = false;
		++playerJumpCount_;
	}
	playerStageVelocity_.x += gravityX;
	playerStageVelocity_.y += gravityY;
	playerStagePosition_.x += playerStageVelocity_.x;
	playerStagePosition_.y += playerStageVelocity_.y;

	// 周囲のブロックの内側でプレイヤーを止める。
	const float minStageX = -1.0f + playerHalfSize;
	const float maxStageX = 19.0f - playerHalfSize;
	const float minStageY = -1.0f + playerHalfSize;
	const float maxStageY = 19.0f - playerHalfSize;
	isPlayerGrounded_ = false;
	if (playerStagePosition_.x < minStageX) {
		playerStagePosition_.x = minStageX;
		isPlayerGrounded_ = playerStageVelocity_.x < 0.0f;
		playerStageVelocity_.x = 0.0f;
	} else if (playerStagePosition_.x > maxStageX) {
		playerStagePosition_.x = maxStageX;
		isPlayerGrounded_ = playerStageVelocity_.x > 0.0f;
		playerStageVelocity_.x = 0.0f;
	}
	if (playerStagePosition_.y < minStageY) {
		playerStagePosition_.y = minStageY;
		isPlayerGrounded_ = isPlayerGrounded_ || playerStageVelocity_.y < 0.0f;
		playerStageVelocity_.y = 0.0f;
	} else if (playerStagePosition_.y > maxStageY) {
		playerStagePosition_.y = maxStageY;
		isPlayerGrounded_ = isPlayerGrounded_ || playerStageVelocity_.y > 0.0f;
		playerStageVelocity_.y = 0.0f;
	}

	// プレイヤーの位置と向きをステージの回転に追従させる。
	const float playerStageCenterX = 9.0f;
	const float playerStageCenterY = 9.0f;
	const float offsetX = playerStagePosition_.x - playerStageCenterX;
	const float offsetY = playerStagePosition_.y - playerStageCenterY;
	worldTransform_.translation_ = {
		playerStageCenterX + offsetX * playerStageCosine - offsetY * playerStageSine,
		playerStageCenterY + offsetX * playerStageSine + offsetY * playerStageCosine,
		playerStagePosition_.z,
	};
	worldTransform_.rotation_.z = stageRotation_;
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
	}

	// ============================================================
	// 1番 Wave
	// ============================================================

	if (blockAnimation_ == BlockAnimation::Wave) {
		// 波の位置を進める
		waveTimer_ += 0.03f;

		// ブロック間の間隔
		const float waveLength = 0.35f;

		// 10個目のブロックが伸びて縮み終わる位置
		const float waveEnd = wavePreviewDelay + 9.0f * waveLength + 1.0f;

		// 最後のブロックまでWaveが完全に終了したら次の列へ
		if (waveTimer_ >= waveEnd) {
			waveTimer_ = 0.0f;

			int newLine;

			do {
				newLine = rand() % 4;
			} while (newLine == waveLine_);

			waveLine_ = newLine;
			isWaveReverse_ = (rand() % 2) != 0;
		}
	}

	// ============================================================
	// 2番 OddEven
	// ============================================================

	if (blockAnimation_ == BlockAnimation::OddEven) {
		oddEvenTimer_ += 0.03f;

		if (oddEvenTimer_ >= 2.0f * (wavePreviewDelay + 1.0f)) {
			oddEvenTimer_ = 0.0f;

			int newLine;

			do {
				newLine = rand() % 4;
			} while (newLine == oddEvenLine_);

			oddEvenLine_ = newLine;
		}
	}

	// ============================================================
	// 3番 Random
	// ============================================================

	if (blockAnimation_ == BlockAnimation::Random) {
		randomTimer_ += 0.03f;

		if (randomTimer_ >= wavePreviewDelay + 1.0f) {
			randomTimer_ = 0.0f;

			for (int& index : randomIndices_) {
				index = rand() % 10;
			}
		}
	}

	const float kBlockSize = 2.0f;

	// ============================================================
	// ブロックA
	// ============================================================

	for (uint32_t i = 0; i < worldTransformBlocks_.size(); i++) {
		for (uint32_t j = 0; j < worldTransformBlocks_[i].size(); j++) {
			WorldTransform* worldTransformBlock = worldTransformBlocks_[i][j];

			if (!worldTransformBlock) {
				continue;
			}

			float scaleY = 1.0f;

			switch (blockAnimation_) {

			case BlockAnimation::Wave: {
				if (waveLine_ == 0) {
					const float waveLength = 0.35f;
					const float wavePosition = isWaveReverse_ ? 9.0f - j : static_cast<float>(j);
					float localTime = waveTimer_ - wavePreviewDelay - wavePosition * waveLength;

					// そのブロックのWaveが始まってから1.0になるまで
					if (localTime >= 0.0f && localTime <= 1.0f) {
						scaleY = 1.0f + sinf(localTime * 3.141592f);
					}
				}
				break;
			}

			case BlockAnimation::OddEven: {
				if (oddEvenLine_ == 0) {
					if (oddEvenTimer_ >= wavePreviewDelay && oddEvenTimer_ < wavePreviewDelay + 1.0f && j % 2 == 0) {
						float localTime = oddEvenTimer_ - wavePreviewDelay;
						scaleY = 1.0f + sinf(localTime * 3.141592f);
					} else if (oddEvenTimer_ >= 2.0f * wavePreviewDelay + 1.0f && oddEvenTimer_ < 2.0f * (wavePreviewDelay + 1.0f) && j % 2 == 1) {
						float localTime = oddEvenTimer_ - (2.0f * wavePreviewDelay + 1.0f);
						scaleY = 1.0f + sinf(localTime * 3.141592f);
					}
				}
				break;
			}

			case BlockAnimation::Random: {
				if (j == static_cast<uint32_t>(randomIndices_[0]) && randomTimer_ >= wavePreviewDelay && randomTimer_ < wavePreviewDelay + 1.0f) {
					scaleY = 1.0f + sinf((randomTimer_ - wavePreviewDelay) * 3.141592f);
				}
				break;
			}

			case BlockAnimation::Center:
				break;
			}

			worldTransformBlock->scale_.x = 1.0f;
			worldTransformBlock->scale_.y = scaleY;
			worldTransformBlock->scale_.z = 1.0f;

			worldTransformBlock->translation_.y = -2.0f + (scaleY - 1.0f) * kBlockSize * 0.5f;

			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

			worldTransformBlock->TransferMatrix();
		}
	}

	// ============================================================
	// ブロックB
	// ============================================================

	for (uint32_t j = 0; j < blockLine1_.size(); j++) {
		WorldTransform* block = blockLine1_[j];

		if (!block) {
			continue;
		}

		float scaleY = 1.0f;

		switch (blockAnimation_) {

		case BlockAnimation::Wave: {
			if (waveLine_ == 1) {
				const float waveLength = 0.35f;
				const float wavePosition = isWaveReverse_ ? 9.0f - j : static_cast<float>(j);
				float localTime = waveTimer_ - wavePreviewDelay - wavePosition * waveLength;

				if (localTime >= 0.0f && localTime <= 1.0f) {
					scaleY = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::OddEven: {
			if (oddEvenLine_ == 1) {
				if (oddEvenTimer_ >= wavePreviewDelay && oddEvenTimer_ < wavePreviewDelay + 1.0f && j % 2 == 0) {
					float localTime = oddEvenTimer_ - wavePreviewDelay;
					scaleY = 1.0f + sinf(localTime * 3.141592f);
				} else if (oddEvenTimer_ >= 2.0f * wavePreviewDelay + 1.0f && oddEvenTimer_ < 2.0f * (wavePreviewDelay + 1.0f) && j % 2 == 1) {
					float localTime = oddEvenTimer_ - (2.0f * wavePreviewDelay + 1.0f);
					scaleY = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::Random: {
			if (j == static_cast<uint32_t>(randomIndices_[1]) && randomTimer_ >= wavePreviewDelay && randomTimer_ < wavePreviewDelay + 1.0f) {
				scaleY = 1.0f + sinf((randomTimer_ - wavePreviewDelay) * 3.141592f);
			}
			break;
		}

		case BlockAnimation::Center:
			break;
		}

		block->scale_.x = 1.0f;
		block->scale_.y = scaleY;
		block->scale_.z = 1.0f;

		block->translation_.y = 20.0f - (scaleY - 1.0f) * kBlockSize * 0.5f;

		block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);

		block->TransferMatrix();
	}

	// ============================================================
	// ブロックC
	// ============================================================

	for (uint32_t i = 0; i < blockLine2_.size(); i++) {
		WorldTransform* block = blockLine2_[i];

		if (!block) {
			continue;
		}

		float scaleX = 1.0f;

		switch (blockAnimation_) {

		case BlockAnimation::Wave: {
			if (waveLine_ == 2) {
				const float waveLength = 0.35f;
				const float wavePosition = isWaveReverse_ ? 9.0f - i : static_cast<float>(i);
				float localTime = waveTimer_ - wavePreviewDelay - wavePosition * waveLength;

				if (localTime >= 0.0f && localTime <= 1.0f) {
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::OddEven: {
			if (oddEvenLine_ == 2) {
				if (oddEvenTimer_ >= wavePreviewDelay && oddEvenTimer_ < wavePreviewDelay + 1.0f && i % 2 == 0) {
					float localTime = oddEvenTimer_ - wavePreviewDelay;
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				} else if (oddEvenTimer_ >= 2.0f * wavePreviewDelay + 1.0f && oddEvenTimer_ < 2.0f * (wavePreviewDelay + 1.0f) && i % 2 == 1) {
					float localTime = oddEvenTimer_ - (2.0f * wavePreviewDelay + 1.0f);
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::Random: {
			if (i == static_cast<uint32_t>(randomIndices_[2]) && randomTimer_ >= wavePreviewDelay && randomTimer_ < wavePreviewDelay + 1.0f) {
				scaleX = 1.0f + sinf((randomTimer_ - wavePreviewDelay) * 3.141592f);
			}
			break;
		}

		case BlockAnimation::Center:
			break;
		}

		block->scale_.x = scaleX;
		block->scale_.y = 1.0f;
		block->scale_.z = 1.0f;

		block->translation_.x = 20.0f - (scaleX - 1.0f) * kBlockSize * 0.5f;

		block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);

		block->TransferMatrix();
	}

	// ============================================================
	// ブロックD
	// ============================================================

	for (uint32_t i = 0; i < blockLine3_.size(); i++) {
		WorldTransform* block = blockLine3_[i];

		if (!block) {
			continue;
		}

		float scaleX = 1.0f;

		switch (blockAnimation_) {

		case BlockAnimation::Wave: {
			if (waveLine_ == 3) {
				const float waveLength = 0.35f;
				const float wavePosition = isWaveReverse_ ? 9.0f - i : static_cast<float>(i);
				float localTime = waveTimer_ - wavePreviewDelay - wavePosition * waveLength;

				if (localTime >= 0.0f && localTime <= 1.0f) {
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::OddEven: {
			if (oddEvenLine_ == 3) {
				if (oddEvenTimer_ >= wavePreviewDelay && oddEvenTimer_ < wavePreviewDelay + 1.0f && i % 2 == 0) {
					float localTime = oddEvenTimer_ - wavePreviewDelay;
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				} else if (oddEvenTimer_ >= 2.0f * wavePreviewDelay + 1.0f && oddEvenTimer_ < 2.0f * (wavePreviewDelay + 1.0f) && i % 2 == 1) {
					float localTime = oddEvenTimer_ - (2.0f * wavePreviewDelay + 1.0f);
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::Random: {
			if (i == static_cast<uint32_t>(randomIndices_[3]) && randomTimer_ >= wavePreviewDelay && randomTimer_ < wavePreviewDelay + 1.0f) {
				scaleX = 1.0f + sinf((randomTimer_ - wavePreviewDelay) * 3.141592f);
			}
			break;
		}

		case BlockAnimation::Center:
			break;
		}

		block->scale_.x = scaleX;
		block->scale_.y = 1.0f;
		block->scale_.z = 1.0f;

		block->translation_.x = -2.0f + (scaleX - 1.0f) * kBlockSize * 0.5f;

		block->matWorld_ = MakeAffineMatrix(block->scale_, block->rotation_, block->translation_);

		block->TransferMatrix();
	}

	// 次に盛り上がる場所へ、最大サイズの予測ブロックを配置する。
	// これは描画専用で collisionBlocks には追加しない。
	predictionCount_ = 0;
	auto AddPrediction = [&](int line, int index) {
		if (predictionCount_ >= predictionTransforms_.size()) {
			return;
		}
		WorldTransform& predictionTransform = predictionTransforms_[predictionCount_++];
		predictionTransform.scale_ = line < 2 ? Vector3{1.0f, 2.0f, 1.0f} : Vector3{2.0f, 1.0f, 1.0f};
		switch (line) {
		case 0:
			predictionTransform.translation_ = {2.0f * index, -1.0f, 0.0f};
			break;
		case 1:
			predictionTransform.translation_ = {2.0f * index, 19.0f, 0.0f};
			break;
		case 2:
			predictionTransform.translation_ = {19.0f, 2.0f * index, 0.0f};
			break;
		case 3:
			predictionTransform.translation_ = {-1.0f, 2.0f * index, 0.0f};
			break;
		}
		predictionTransform.rotation_.z = stageRotation_;
		predictionTransform.matWorld_ = MakeAffineMatrix(predictionTransform.scale_, predictionTransform.rotation_, predictionTransform.translation_);
		predictionTransform.TransferMatrix();
	};
	if (blockAnimation_ == BlockAnimation::Wave && waveLine_ >= 0 && waveLine_ < 4) {
		const float waveLength = 0.35f;
		for (int index = 0; index < 10; ++index) {
			const float wavePosition = isWaveReverse_ ? 9.0f - index : static_cast<float>(index);
			const float previewStart = wavePosition * waveLength;
			const float blockMoveStart = wavePreviewDelay + previewStart;
			if (waveTimer_ >= previewStart && waveTimer_ < blockMoveStart) {
				AddPrediction(waveLine_, index);
			}
		}
	} else if (blockAnimation_ == BlockAnimation::OddEven && oddEvenLine_ >= 0 && oddEvenLine_ < 4) {
		if (oddEvenTimer_ < wavePreviewDelay) {
			for (int index = 0; index < 10; index += 2) {
				AddPrediction(oddEvenLine_, index);
			}
		} else if (oddEvenTimer_ >= wavePreviewDelay + 1.0f && oddEvenTimer_ < 2.0f * wavePreviewDelay + 1.0f) {
			for (int index = 1; index < 10; index += 2) {
				AddPrediction(oddEvenLine_, index);
			}
		}
	} else if (blockAnimation_ == BlockAnimation::Random && randomTimer_ < wavePreviewDelay) {
		for (int line = 0; line < static_cast<int>(randomIndices_.size()); ++line) {
			AddPrediction(line, randomIndices_[line]);
		}
	}

	// ステージ座標でプレイヤーと全ブロックのAABBを判定し、重なりを解消する。
	// ブロックの伸縮後のscaleを使うため、アニメーション中も当たり判定が一致する。
	const float stageCenterX = 9.0f;
	const float stageCenterY = 9.0f;
	const float cosine = cosf(stageRotation_);
	const float sine = sinf(stageRotation_);
	if (!isGameOver_) {
	const float playerHalfWidth = worldTransform_.scale_.x;
	const float playerHalfHeight = worldTransform_.scale_.y;
	std::vector<WorldTransform*> collisionBlocks;
	for (const std::vector<WorldTransform*>& line : worldTransformBlocks_) {
		collisionBlocks.insert(collisionBlocks.end(), line.begin(), line.end());
	}
	collisionBlocks.insert(collisionBlocks.end(), blockLine1_.begin(), blockLine1_.end());
	collisionBlocks.insert(collisionBlocks.end(), blockLine2_.begin(), blockLine2_.end());
	collisionBlocks.insert(collisionBlocks.end(), blockLine3_.begin(), blockLine3_.end());
	collisionBlocks.insert(collisionBlocks.end(), floatingBlocks_.begin(), floatingBlocks_.end());
	// 角で複数ブロックが重なった場合も解消できるよう、全ブロックを数回走査する。
	for (int pass = 0; pass < 3; ++pass) {
		for (WorldTransform* block : collisionBlocks) {
			if (!block) {
				continue;
			}
			const float halfWidth = fabsf(block->scale_.x);
			const float halfHeight = fabsf(block->scale_.y);
			const float deltaX = playerStagePosition_.x - block->translation_.x;
			const float deltaY = playerStagePosition_.y - block->translation_.y;
			const float overlapX = playerHalfWidth + halfWidth - fabsf(deltaX);
			const float overlapY = playerHalfHeight + halfHeight - fabsf(deltaY);
			if (overlapX <= 0.0f || overlapY <= 0.0f) {
				continue;
			}
			const bool isMovingHazard = block->scale_.x > 1.001f || block->scale_.y > 1.001f;
			if (isMovingHazard && playerDamageTimer_ <= 0.0f) {
				deathTimer_ = 0.0f;
				playerStageVelocity_ = {};
				for (size_t i = 0; i < burstParticles_.size(); ++i) {
					BurstParticle& particle = burstParticles_[i];
					const float angle = 6.2831853f * static_cast<float>(i) / static_cast<float>(burstParticles_.size());
					const float speed = 0.16f + 0.025f * static_cast<float>(i % 5);
					particle.transform.Initialize();
					particle.transform.translation_ = worldTransform_.translation_;
					particle.transform.scale_ = {0.42f, 0.42f, 0.42f};
					particle.velocity = {cosf(angle) * speed, sinf(angle) * speed, 0.0f};
					particle.life = 0.75f + 0.25f * static_cast<float>(i % 4) / 3.0f;
				}
				return;
			}

			if (overlapX < overlapY) {
				const float normalX = deltaX >= 0.0f ? 1.0f : -1.0f;
				playerStagePosition_.x += normalX * overlapX;
				if (playerStageVelocity_.x * normalX < 0.0f) {
					playerStageVelocity_.x = 0.0f;
				}
				if (normalX * gravityX < 0.0f) {
					isPlayerGrounded_ = true;
				}
			} else {
				const float normalY = deltaY >= 0.0f ? 1.0f : -1.0f;
				playerStagePosition_.y += normalY * overlapY;
				if (playerStageVelocity_.y * normalY < 0.0f) {
					playerStageVelocity_.y = 0.0f;
				}
				// 重力と反対向きの面に押し戻された場合は接地扱いにする。
				if (normalY * gravityY < 0.0f) {
					isPlayerGrounded_ = true;
				}
			}
		}
	}
	if (isPlayerGrounded_) {
		playerJumpCount_ = 0;
	}

	// 押し戻し後の位置を描画用ワールド変換にも反映する。
	const float correctedOffsetX = playerStagePosition_.x - stageCenterX;
	const float correctedOffsetY = playerStagePosition_.y - stageCenterY;
	worldTransform_.translation_ = {
		stageCenterX + correctedOffsetX * cosine - correctedOffsetY * sine,
		stageCenterY + correctedOffsetX * sine + correctedOffsetY * cosine,
		playerStagePosition_.z,
	};
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
	}

	// 各辺のブロックをステージ中心のまわりに回転させる。
	// 上のアニメーション処理が毎フレーム各ブロックの基準位置を設定するため、
	// 回転は描画用の行列を作る最後に適用する。
	auto ApplyStageRotation = [&](WorldTransform* block) {
		if (!block) {
			return;
		}
		const float offsetX = block->translation_.x - stageCenterX;
		const float offsetY = block->translation_.y - stageCenterY;
		Vector3 rotatedTranslation = {
			stageCenterX + offsetX * cosine - offsetY * sine,
			stageCenterY + offsetX * sine + offsetY * cosine,
			block->translation_.z,
		};
		Vector3 rotatedRotation = block->rotation_;
		rotatedRotation.z += stageRotation_;
		block->matWorld_ = MakeAffineMatrix(block->scale_, rotatedRotation, rotatedTranslation);
		block->TransferMatrix();
	};
	for (const std::vector<WorldTransform*>& line : worldTransformBlocks_) {
		for (WorldTransform* block : line) {
			ApplyStageRotation(block);
		}
	}
	for (WorldTransform* block : blockLine1_) {
		ApplyStageRotation(block);
	}
	for (WorldTransform* block : blockLine2_) {
		ApplyStageRotation(block);
	}
	for (WorldTransform* block : blockLine3_) {
		ApplyStageRotation(block);
	}
	for (WorldTransform* block : floatingBlocks_) {
		ApplyStageRotation(block);
	}
	for (size_t i = 0; i < predictionCount_; ++i) {
		WorldTransform& predictionTransform = predictionTransforms_[i];
		const float predictionOffsetX = predictionTransform.translation_.x - stageCenterX;
		const float predictionOffsetY = predictionTransform.translation_.y - stageCenterY;
		predictionTransform.translation_ = {
			stageCenterX + predictionOffsetX * cosine - predictionOffsetY * sine,
			stageCenterY + predictionOffsetX * sine + predictionOffsetY * cosine,
			predictionTransform.translation_.z,
		};
		predictionTransform.matWorld_ = MakeAffineMatrix(predictionTransform.scale_, predictionTransform.rotation_, predictionTransform.translation_);
		predictionTransform.TransferMatrix();
	}

	// ============================================================
	// アニメーション変更
	// ============================================================

	if (!isGameOver_ && Input::GetInstance()->TriggerKey(DIK_1)) {
		blockAnimation_ = BlockAnimation::Wave;

		waveTimer_ = 0.0f;
		waveLine_ = rand() % 4;
		isWaveReverse_ = (rand() % 2) != 0;
	}

	if (!isGameOver_ && Input::GetInstance()->TriggerKey(DIK_2)) {
		blockAnimation_ = BlockAnimation::OddEven;

		oddEvenTimer_ = 0.0f;
		oddEvenLine_ = rand() % 4;
	}

	if (!isGameOver_ && Input::GetInstance()->TriggerKey(DIK_3)) {
		blockAnimation_ = BlockAnimation::Random;

		randomTimer_ = 0.0f;
		for (int& index : randomIndices_) {
			index = rand() % 10;
		}
	}

	if (!isGameOver_ && Input::GetInstance()->TriggerKey(DIK_4)) {
		blockAnimation_ = BlockAnimation::Center;

		waveTimer_ = 0.0f;
		oddEvenTimer_ = 0.0f;
		randomTimer_ = 0.0f;
	}
}

// 描画
void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());
	if (skySphere_) {
		skySphere_->Draw(skySphereTransform_, camera_, skyTexture_);
	}
	auto DrawBlock = [&](WorldTransform* block) {
		if (!block || !modelBlock_) {
			return;
		}
		const bool isMoving = block->scale_.x > 1.001f || block->scale_.y > 1.001f;
		modelBlock_->Draw(*block, camera_, isMoving ? &hazardBlockColor_ : nullptr);
	};

	// プレイヤー描画
	const bool isPlayerVisible = deathTimer_ < 0.0f && (playerDamageTimer_ <= 0.0f || static_cast<int>(playerDamageTimer_ * 10.0f) % 2 == 0);
	if (modelBlock_ && isPlayerVisible) {
		modelBlock_->Draw(worldTransform_, camera_);
	}
	for (const BurstParticle& particle : burstParticles_) {
		if (particle.life > 0.0f && modelBlock_) modelBlock_->Draw(particle.transform, camera_, &hazardBlockColor_);
	}

	// ブロックA描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			DrawBlock(worldTransformBlock);
		}
	}

	// ブロックB描画
	for (WorldTransform* block : blockLine1_) {
		if (!block) {
			continue;
		}

		DrawBlock(block);
	}

	// ブロックC描画
	for (WorldTransform* block : blockLine2_) {
		if (!block) {
			continue;
		}

		DrawBlock(block);
	}

	// ブロックD描画
	for (WorldTransform* block : blockLine3_) {
		if (!block) {
			continue;
		}

		DrawBlock(block);
	}

	// 空中ブロック描画
	for (WorldTransform* block : floatingBlocks_) {
		DrawBlock(block);
	}
	if (modelPrediction_) {
		for (size_t i = 0; i < predictionCount_; ++i) {
			modelPrediction_->Draw(predictionTransforms_[i], camera_);
		}
	}
	Model::PostDraw();
	if (isGameOver_) {
		Sprite::PreDraw(dxCommon->GetCommandList());
		// 黒い半透明スプライトを文字と同時にフェードインし、ステージが見える暗さにする。
		const float dimAlpha = std::clamp((gameOverTimer_ - 0.45f) / 1.4f, 0.0f, 1.0f) * 0.42f;
		if (gameOverDimSprite_) {
			gameOverDimSprite_->SetColor({0.0f, 0.0f, 0.0f, dimAlpha});
			gameOverDimSprite_->Draw();
		}
		const float textAlpha = std::clamp((gameOverTimer_ - 0.45f) / 1.4f, 0.0f, 1.0f);
		if (gameOverSprite_) {
			// 表示後の待機中だけ、中央位置を保ったまま上下にゆっくり揺らす。
			const float idleTime = gameOverTimer_ > 1.85f ? gameOverTimer_ - 1.85f : 0.0f;
			const float verticalOffset = 8.0f * std::sin(idleTime * 2.0943951f);
			gameOverSprite_->SetSize({581.0f, 187.0f});
			gameOverSprite_->SetPosition({349.5f, 266.5f + verticalOffset});
			gameOverSprite_->SetColor({1.0f, 1.0f, 1.0f, textAlpha});
			gameOverSprite_->Draw();
		}
		Sprite::PostDraw();
		return;
	}
	if (!isStartCountdownReady_ || !isStartCountdownActive_) {
		return;
	}

	// タイトルロゴに合わせた太字フォントの透過画像を表示する。
	const int countdownNumber = 3 - static_cast<int>(startCountdownTimer_);
	const bool isStartText = startCountdownTimer_ >= 3.0f;
	const float phaseTime = isStartText ? startCountdownTimer_ - 3.0f : startCountdownTimer_ - floorf(startCountdownTimer_);
	const float pulse = 1.0f + 0.18f * (1.0f - std::clamp(phaseTime / 0.25f, 0.0f, 1.0f));
	const int spriteIndex = isStartText ? 3 : 3 - countdownNumber;
	Sprite* countdownSprite = startCountdownSprites_[spriteIndex];
	if (!countdownSprite) {
		return;
	}
	const Vector2 baseSize = isStartText ? Vector2{800.0f, 245.0f} : Vector2{280.0f, 280.0f};
	const float startProgress = std::clamp(phaseTime / 0.7f, 0.0f, 1.0f);
	const float startPop = phaseTime < 0.18f ? 1.34f - 0.34f * (phaseTime / 0.18f) : 1.0f + 0.025f * sinf((phaseTime - 0.18f) * 12.56637f);
	const float displayScale = isStartText ? startPop : pulse;
	const Vector2 size = {baseSize.x * displayScale, baseSize.y * displayScale};
	countdownSprite->SetSize(size);
	countdownSprite->SetPosition({(1280.0f - size.x) * 0.5f, (720.0f - size.y) * 0.5f});
	// 背景は透過PNG、文字も少し透かしてプレイ画面を見えるようにする。
	countdownSprite->SetColor({1.0f, 1.0f, 1.0f, isStartText ? 0.9f : 0.72f});
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	if (isStartText && startCountdownGlowSprite_) {
		const Vector2 glowSize = {size.x * 1.08f, size.y * 1.08f};
		startCountdownGlowSprite_->SetSize(glowSize);
		startCountdownGlowSprite_->SetPosition({(1280.0f - glowSize.x) * 0.5f, (720.0f - glowSize.y) * 0.5f});
		startCountdownGlowSprite_->SetColor({1.0f, 0.58f, 0.2f, 0.32f * (1.0f - startProgress)});
		startCountdownGlowSprite_->Draw();
	}
	countdownSprite->Draw();
	Sprite::PostDraw();
}
