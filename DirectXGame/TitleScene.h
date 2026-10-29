#pragma once

#include "KamataEngine.h"

using namespace KamataEngine;

class TitleScene {
public:
	~TitleScene();

	void Initialize();
	bool Update();
	void Draw();

private:
	Model* skySphere_ = nullptr;
	uint32_t skyTexture_ = 0;
	WorldTransform skySphereTransform_;
	Camera camera_;
	uint32_t titleLogoTexture_ = 0;
	Model* titleLogoModel_ = nullptr;
	WorldTransform titleLogoTransform_;
	uint32_t startButtonTexture_ = 0;
	Model* startButtonModel_ = nullptr;
	WorldTransform startButtonTransform_;
	uint32_t titleUnderlineTexture_ = 0;
	Model* titleUnderlineModel_ = nullptr;
	WorldTransform titleUnderlineTransform_;
	float animationTimer_ = 0.0f;
	float titleElapsedSeconds_ = 0.0f;
	float titleScale_ = 0.94f;
	float titleOffsetY_ = 12.0f;
	float buttonScale_ = 0.96f;
	float buttonOffsetY_ = 10.0f;
};
