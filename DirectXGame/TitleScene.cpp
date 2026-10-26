#include "TitleScene.h"
#include "MyMath.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kUiWorldUnitsPerPixel = 0.051777f;

void ConfigureUnlitModel(KamataEngine::Model* model) {
	if (!model) {
		return;
	}
	for (const auto& mesh : model->GetMeshes()) {
		if (mesh && mesh->GetMaterial()) {
			KamataEngine::Material* material = mesh->GetMaterial();
			material->ambient_ = {2.0f, 2.0f, 2.0f};
			material->diffuse_ = {0.0f, 0.0f, 0.0f};
			material->specular_ = {0.0f, 0.0f, 0.0f};
		}
	}
}

void SetCardTransform(KamataEngine::WorldTransform& transform, const KamataEngine::Vector2& screenPosition, const KamataEngine::Vector2& pixelSize, float rotationZ, float depth) {
	transform.scale_ = {pixelSize.x * kUiWorldUnitsPerPixel * 0.5f, pixelSize.y * kUiWorldUnitsPerPixel * 0.5f, 0.001f};
	transform.rotation_ = {0.0f, 0.0f, rotationZ};
	transform.translation_ = {(screenPosition.x - 640.0f) * kUiWorldUnitsPerPixel, (360.0f - screenPosition.y) * kUiWorldUnitsPerPixel, depth};
	transform.matWorld_ = MakeAffineMatrix(transform.scale_, transform.rotation_, transform.translation_);
	transform.TransferMatrix();
}
} // namespace

TitleScene::~TitleScene() {
	delete skySphere_;
	delete titleLogoModel_;
	delete startButtonModel_;
	delete titleUnderlineModel_;
}

void TitleScene::Initialize() {
	skySphere_ = Model::CreateSphere(64, 128);
	if (skySphere_) {
		// 天球は照明の影響を受けないようにし、UV境界での法線差による線を防ぐ。
		for (const auto& mesh : skySphere_->GetMeshes()) {
			if (mesh && mesh->GetMaterial()) {
				Material* material = mesh->GetMaterial();
				material->ambient_ = {1.0f, 1.0f, 1.0f};
				material->diffuse_ = {0.0f, 0.0f, 0.0f};
				material->specular_ = {0.0f, 0.0f, 0.0f};
			}
		}
	}
	camera_.Initialize();
	skySphereTransform_.Initialize();
	skySphereTransform_.scale_ = {-500.0f, -500.0f, -500.0f};
	skySphereTransform_.translation_ = camera_.translation_;
	skySphereTransform_.matWorld_ = MakeAffineMatrix(skySphereTransform_.scale_, skySphereTransform_.rotation_, skySphereTransform_.translation_);
	skySphereTransform_.TransferMatrix();

	skyTexture_ = TextureManager::Load("caveBrown.png");

	titleLogoTexture_ = TextureManager::Load("titleLogo.png");
	titleLogoModel_ = Model::Create();
	ConfigureUnlitModel(titleLogoModel_);
	titleLogoTransform_.Initialize();
	SetCardTransform(titleLogoTransform_, {640.0f, 253.5f}, {581.0f, 187.0f}, 0.0f, -5.0f);
	if (titleLogoModel_) {
		titleLogoModel_->SetAlpha(0.0f);
	}
	startButtonTexture_ = TextureManager::Load("startButton.png");
	startButtonModel_ = Model::Create();
	ConfigureUnlitModel(startButtonModel_);
	startButtonTransform_.Initialize();
	SetCardTransform(startButtonTransform_, {640.0f, 490.0f}, {251.0f, 82.0f}, 0.0f, -5.0f);
	if (startButtonModel_) {
		startButtonModel_->SetAlpha(0.0f);
	}
	titleUnderlineTexture_ = TextureManager::Load("white1x1.png");
	titleUnderlineModel_ = Model::Create();
	ConfigureUnlitModel(titleUnderlineModel_);
	titleUnderlineTransform_.Initialize();
	SetCardTransform(titleUnderlineTransform_, {640.0f, 270.0f}, {221.0f, 4.0f}, 0.0f, -5.2f);
	if (titleUnderlineModel_) {
		titleUnderlineModel_->SetAlpha(0.0f);
	}
}

bool TitleScene::Update() {
	animationTimer_ += 0.02f;
	titleElapsedSeconds_ += 1.0f / 60.0f;
	const float entranceProgress = std::clamp(titleElapsedSeconds_ / 0.6f, 0.0f, 1.0f);
	const float easedEntrance = entranceProgress * entranceProgress * (3.0f - 2.0f * entranceProgress);
	titleScale_ = 0.94f + 0.06f * easedEntrance;
	titleOffsetY_ = 12.0f * (1.0f - easedEntrance);
	if (entranceProgress >= 1.0f) {
		const float idleTime = titleElapsedSeconds_ - 0.6f;
		titleOffsetY_ += std::sin(idleTime * 2.0f * 3.14159265f / 3.0f) * 3.0f;
	}
	if (titleLogoModel_) {
		SetCardTransform(titleLogoTransform_, {640.0f, 253.5f + titleOffsetY_}, {581.0f * titleScale_, 187.0f * titleScale_}, 0.0f, -5.0f);
		titleLogoModel_->SetAlpha(easedEntrance);
	}
	const float buttonProgress = std::clamp((titleElapsedSeconds_ - 0.15f) / 0.55f, 0.0f, 1.0f);
	const float easedButton = buttonProgress * buttonProgress * (3.0f - 2.0f * buttonProgress);
	buttonScale_ = 0.96f + 0.04f * easedButton;
	buttonOffsetY_ = 10.0f * (1.0f - easedButton);
	const float idleTime = titleElapsedSeconds_ > 0.6f ? titleElapsedSeconds_ - 0.6f : 0.0f;
	const float buttonPulse = 0.775f + 0.225f * std::sin(idleTime * 2.0f * 3.14159265f / 2.4f);
	if (startButtonModel_) {
		SetCardTransform(startButtonTransform_, {640.0f, 490.0f + buttonOffsetY_}, {251.0f * buttonScale_, 82.0f * buttonScale_}, 0.0f, -5.0f);
		startButtonModel_->SetAlpha(easedButton * buttonPulse);
	}
	if (titleUnderlineModel_) {
		SetCardTransform(titleUnderlineTransform_, {640.0f, 253.5f + titleOffsetY_ + 16.5f * titleScale_}, {221.0f * titleScale_, 4.0f * titleScale_}, 0.0f, -5.2f);
		titleUnderlineModel_->SetAlpha(easedEntrance * 0.5f);
	}
	skySphereTransform_.rotation_.y = animationTimer_ * 0.025f;
	skySphereTransform_.translation_ = camera_.translation_;
	skySphereTransform_.matWorld_ = MakeAffineMatrix(skySphereTransform_.scale_, skySphereTransform_.rotation_, skySphereTransform_.translation_);
	skySphereTransform_.TransferMatrix();

	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_RETURN) || input->TriggerKey(DIK_SPACE)) {
		return true;
	}
	if (input->IsTriggerMouse(0)) {
		const Vector2& mousePosition = input->GetMousePosition();
		const float left = 640.0f - 125.5f * buttonScale_;
		const float right = 640.0f + 125.5f * buttonScale_;
		const float top = 490.0f + buttonOffsetY_ - 41.0f * buttonScale_;
		const float bottom = 490.0f + buttonOffsetY_ + 41.0f * buttonScale_;
		return mousePosition.x >= left && mousePosition.x <= right && mousePosition.y >= top && mousePosition.y <= bottom;
	}
	return false;
}

void TitleScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	if (skySphere_) {
		skySphere_->Draw(skySphereTransform_, camera_, skyTexture_);
	}
	if (titleLogoModel_) {
		titleLogoModel_->Draw(titleLogoTransform_, camera_, titleLogoTexture_);
	}
	if (startButtonModel_) {
		startButtonModel_->Draw(startButtonTransform_, camera_, startButtonTexture_);
	}
	if (titleUnderlineModel_) {
		titleUnderlineModel_->Draw(titleUnderlineTransform_, camera_, titleUnderlineTexture_);
	}
	Model::PostDraw();
}
