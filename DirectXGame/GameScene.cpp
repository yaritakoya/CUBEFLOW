#include "GameScene.h"
#include "MyMath.h"
#include <cstdlib>

// デストラクタ
GameScene::~GameScene() {
	delete modelBlock_;
	delete debugCamera_;

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
	modelBlock_ = Model::Create();

	camera_.Initialize();

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
	waveTimer_ = 0.0f;

	// 2番用の初期値
	oddEvenLine_ = rand() % 4;
	oddEvenTimer_ = 0.0f;

	// 3番用の初期値
	randomLine_ = rand() % 4;
	randomIndex_ = rand() % 10;
	randomTimer_ = 0.0f;

	// ブロックB・C・Dを生成
	InitializeBlockLine1();
	InitializeBlockLine2();
	InitializeBlockLine3();

	// デバッグカメラ
	debugCamera_ = new DebugCamera(1280, 720);
}

// 更新
void GameScene::Update() {
	animationTimer_ += 0.03f;

	// ============================================================
	// 1番 Wave
	// ============================================================

	if (blockAnimation_ == BlockAnimation::Wave) {
		// 波の位置を進める
		waveTimer_ += 0.03f;

		// ブロック間の間隔
		const float waveLength = 0.35f;

		// 10個目のブロックが伸びて縮み終わる位置
		const float waveEnd = 9.0f * waveLength + 1.0f;

		// 最後のブロックまでWaveが完全に終了したら次の列へ
		if (waveTimer_ >= waveEnd) {
			waveTimer_ = 0.0f;

			int newLine;

			do {
				newLine = rand() % 4;
			} while (newLine == waveLine_);

			waveLine_ = newLine;
		}
	}

	// ============================================================
	// 2番 OddEven
	// ============================================================

	if (blockAnimation_ == BlockAnimation::OddEven) {
		oddEvenTimer_ += 0.03f;

		if (oddEvenTimer_ >= 2.0f) {
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

		if (randomTimer_ >= 1.0f) {
			randomTimer_ = 0.0f;

			randomLine_ = rand() % 4;
			randomIndex_ = rand() % 10;
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
					float localTime = waveTimer_ - j * waveLength;

					// そのブロックのWaveが始まってから1.0になるまで
					if (localTime >= 0.0f && localTime <= 1.0f) {
						scaleY = 1.0f + sinf(localTime * 3.141592f);
					}
				}
				break;
			}

			case BlockAnimation::OddEven: {
				if (oddEvenLine_ == 0) {
					if (oddEvenTimer_ < 1.0f && j % 2 == 0) {
						float localTime = oddEvenTimer_;
						scaleY = 1.0f + sinf(localTime * 3.141592f);
					} else if (oddEvenTimer_ >= 1.0f && j % 2 == 1) {
						float localTime = oddEvenTimer_ - 1.0f;
						scaleY = 1.0f + sinf(localTime * 3.141592f);
					}
				}
				break;
			}

			case BlockAnimation::Random: {
				if (randomLine_ == 0 && j == static_cast<uint32_t>(randomIndex_)) {
					scaleY = 1.0f + sinf(randomTimer_ * 3.141592f);
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
				float localTime = waveTimer_ - j * waveLength;

				if (localTime >= 0.0f && localTime <= 1.0f) {
					scaleY = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::OddEven: {
			if (oddEvenLine_ == 1) {
				if (oddEvenTimer_ < 1.0f && j % 2 == 0) {
					float localTime = oddEvenTimer_;
					scaleY = 1.0f + sinf(localTime * 3.141592f);
				} else if (oddEvenTimer_ >= 1.0f && j % 2 == 1) {
					float localTime = oddEvenTimer_ - 1.0f;
					scaleY = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::Random: {
			if (randomLine_ == 1 && j == static_cast<uint32_t>(randomIndex_)) {
				scaleY = 1.0f + sinf(randomTimer_ * 3.141592f);
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
				float localTime = waveTimer_ - i * waveLength;

				if (localTime >= 0.0f && localTime <= 1.0f) {
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::OddEven: {
			if (oddEvenLine_ == 2) {
				if (oddEvenTimer_ < 1.0f && i % 2 == 0) {
					float localTime = oddEvenTimer_;
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				} else if (oddEvenTimer_ >= 1.0f && i % 2 == 1) {
					float localTime = oddEvenTimer_ - 1.0f;
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::Random: {
			if (randomLine_ == 2 && i == static_cast<uint32_t>(randomIndex_)) {
				scaleX = 1.0f + sinf(randomTimer_ * 3.141592f);
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
				float localTime = waveTimer_ - i * waveLength;

				if (localTime >= 0.0f && localTime <= 1.0f) {
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::OddEven: {
			if (oddEvenLine_ == 3) {
				if (oddEvenTimer_ < 1.0f && i % 2 == 0) {
					float localTime = oddEvenTimer_;
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				} else if (oddEvenTimer_ >= 1.0f && i % 2 == 1) {
					float localTime = oddEvenTimer_ - 1.0f;
					scaleX = 1.0f + sinf(localTime * 3.141592f);
				}
			}
			break;
		}

		case BlockAnimation::Random: {
			if (randomLine_ == 3 && i == static_cast<uint32_t>(randomIndex_)) {
				scaleX = 1.0f + sinf(randomTimer_ * 3.141592f);
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

	// ============================================================
	// アニメーション変更
	// ============================================================

	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		blockAnimation_ = BlockAnimation::Wave;

		waveTimer_ = 0.0f;
		waveLine_ = rand() % 4;
	}

	if (Input::GetInstance()->TriggerKey(DIK_2)) {
		blockAnimation_ = BlockAnimation::OddEven;

		oddEvenTimer_ = 0.0f;
		oddEvenLine_ = rand() % 4;
	}

	if (Input::GetInstance()->TriggerKey(DIK_3)) {
		blockAnimation_ = BlockAnimation::Random;

		randomTimer_ = 0.0f;
		randomLine_ = rand() % 4;
		randomIndex_ = rand() % 10;
	}

	if (Input::GetInstance()->TriggerKey(DIK_4)) {
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

	// ブロックA描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// ブロックB描画
	for (WorldTransform* block : blockLine1_) {
		if (!block) {
			continue;
		}

		modelBlock_->Draw(*block, camera_);
	}

	// ブロックC描画
	for (WorldTransform* block : blockLine2_) {
		if (!block) {
			continue;
		}

		modelBlock_->Draw(*block, camera_);
	}

	// ブロックD描画
	for (WorldTransform* block : blockLine3_) {
		if (!block) {
			continue;
		}

		modelBlock_->Draw(*block, camera_);
	}

	Model::PostDraw();
}