#include "GameScene.h"
#include "TitleScene.h"
#include "MyMath.h"
#include <KamataEngine.h>
#include <Windows.h>
#include <algorithm>
#include <vector>

using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE3C_26_ヤリタ_コウヤ");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	{
		TitleScene titleScene;
		GameScene gameScene;
		titleScene.Initialize();
		bool isGameSceneActive = false;

		// 右下から対角線の波で現れ、画面を覆った後に同じ順序で消える3Dタイル遷移。
		struct TransitionTile {
			WorldTransform* transform = nullptr;
			Vector2 screenPosition{};
			int diagonalOrder = 0;
			bool isVisible = false;
		};
		constexpr int kTileColumns = 8;
		constexpr int kTileRows = 5;
		constexpr float kTileWidth = 160.0f;
		constexpr float kTileHeight = 144.0f;
		constexpr float kTileDelay = 0.06f;
		constexpr float kTileDuration = 0.18f;
		constexpr int kMaxDiagonalOrder = kTileColumns + kTileRows - 2;
		constexpr float kTransitionPhaseDuration = kMaxDiagonalOrder * kTileDelay + kTileDuration;
		constexpr float kFlipAngle = 1.5707963f;
		constexpr float kWorldUnitsPerPixel = 0.046024f; // 720p viewport at 40 units from the default 45-degree camera.
		constexpr float kTransitionTileDepth = -30.0f;
		constexpr float kTransitionTileScale = 0.5f; // Depth is halved, so halve the scale to preserve its screen size.
		const uint32_t transitionTexture = TextureManager::Load("blockPanel.png");
		Camera transitionCamera;
		transitionCamera.Initialize();
		Model* transitionTileModel = Model::Create();
		if (transitionTileModel) {
			for (const auto& mesh : transitionTileModel->GetMeshes()) {
				if (mesh && mesh->GetMaterial()) {
					Material* material = mesh->GetMaterial();
					material->ambient_ = {1.0f, 1.0f, 1.0f};
					material->diffuse_ = {0.0f, 0.0f, 0.0f};
					material->specular_ = {0.0f, 0.0f, 0.0f};
				}
			}
		}
		std::vector<TransitionTile> transitionTiles;
		transitionTiles.reserve(kTileColumns * kTileRows);
		for (int row = 0; row < kTileRows; ++row) {
			for (int column = 0; column < kTileColumns; ++column) {
				TransitionTile tile;
				tile.screenPosition = {column * kTileWidth + kTileWidth * 0.5f, row * kTileHeight + kTileHeight * 0.5f};
				tile.diagonalOrder = (kTileColumns - 1 - column) + (kTileRows - 1 - row);
				tile.transform = new WorldTransform();
				tile.transform->Initialize();
			tile.transform->scale_ = {(kTileWidth + 3.0f) * kWorldUnitsPerPixel * 0.5f * kTransitionTileScale, (kTileHeight + 3.0f) * kWorldUnitsPerPixel * 0.5f * kTransitionTileScale, 0.01f};
			tile.transform->translation_ = {(tile.screenPosition.x - 640.0f) * kWorldUnitsPerPixel * kTransitionTileScale, (360.0f - tile.screenPosition.y) * kWorldUnitsPerPixel * kTransitionTileScale, kTransitionTileDepth};
				tile.transform->matWorld_ = MakeAffineMatrix(tile.transform->scale_, tile.transform->rotation_, tile.transform->translation_);
				tile.transform->TransferMatrix();
				if (!transitionTileModel) {
					delete tile.transform;
					tile.transform = nullptr;
				}
				transitionTiles.push_back(tile);
			}
		}
		bool isTransitioning = false;
		bool isTransitioningOut = false;
		float transitionTimer = 0.0f;

		// メインループ
		while (true) {
			if (KamataEngine::Update()) {
				break;
			}

			if (isTransitioning) {
				if (isGameSceneActive) {
					gameScene.Update();
				} else {
					titleScene.Update();
				}
				transitionTimer += 1.0f / 60.0f;
				for (TransitionTile& tile : transitionTiles) {
					if (!tile.transform) {
						continue;
					}
					const int order = tile.diagonalOrder;
					const float localProgress = std::clamp((transitionTimer - order * kTileDelay) / kTileDuration, 0.0f, 1.0f);
					const float eased = localProgress * localProgress * (3.0f - 2.0f * localProgress);
					tile.isVisible = isTransitioningOut ? localProgress < 1.0f : localProgress > 0.0f;
					tile.transform->translation_.x = (tile.screenPosition.x - 640.0f) * kWorldUnitsPerPixel * kTransitionTileScale;
					tile.transform->translation_.y = (360.0f - tile.screenPosition.y) * kWorldUnitsPerPixel * kTransitionTileScale;
					tile.transform->translation_.z = kTransitionTileDepth;
					tile.transform->rotation_.y = isTransitioningOut ? eased * kFlipAngle : (1.0f - eased) * kFlipAngle;
					tile.transform->matWorld_ = MakeAffineMatrix(tile.transform->scale_, tile.transform->rotation_, tile.transform->translation_);
					tile.transform->TransferMatrix();
				}

				if (!isTransitioningOut && transitionTimer >= kTransitionPhaseDuration) {
					gameScene.Initialize();
					isGameSceneActive = true;
					isTransitioningOut = true;
					transitionTimer = 0.0f;
				}
				if (isTransitioningOut && transitionTimer >= kTransitionPhaseDuration) {
					isTransitioning = false;
					gameScene.BeginStartCountdown();
				}
			} else if (isGameSceneActive) {
				gameScene.Update();
			} else if (titleScene.Update()) {
				isTransitioning = true;
				transitionTimer = 0.0f;
				isTransitioningOut = false;
			}

			dxCommon->PreDraw();
			if (isGameSceneActive) {
				gameScene.Draw();
			} else {
				titleScene.Draw();
			}
			if (isTransitioning) {
				Model::PreDraw(dxCommon->GetCommandList());
				for (TransitionTile& tile : transitionTiles) {
					if (transitionTileModel && tile.transform && tile.isVisible) {
						transitionTileModel->Draw(*tile.transform, transitionCamera, transitionTexture);
					}
				}
				Model::PostDraw();
			}
			dxCommon->PostDraw();
		}
		for (TransitionTile& tile : transitionTiles) {
			delete tile.transform;
		}
		delete transitionTileModel;
	}

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
