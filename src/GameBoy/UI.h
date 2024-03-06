#pragma once
#include "RendererVulkan.h"
#include "EngineState.h"

class UI
{
public:
	explicit UI(RendererVulkan& renderer);
	void Prepare(EngineData& data);
	void Draw(RendererVulkan& renderer);
	~UI();
private:
	UI(const UI& other) = delete;
	UI& operator= (const UI& other) = delete;
};