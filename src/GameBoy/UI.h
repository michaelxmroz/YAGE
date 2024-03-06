#pragma once
#include "RendererVulkan.h"

class UI
{
public:
	explicit UI(RendererVulkan& renderer);
	void Prepare();
	void Draw(RendererVulkan& renderer);
	~UI();
private:
	UI(const UI& other) = delete;
	UI& operator= (const UI& other) = delete;
};