#pragma once

#include "tgaimage.h"

enum class DrawMode {
	WireFrame,
	WireFrameIgnoreZ
};

#define FLOAT_LINE_CALC

struct TGAFile : public TGAImage {

	using TGAImage::TGAImage;
	using TGAImage::Format;

	void show() const;
	void line(const std::pair<int, int>& v1, const std::pair<int, int>& v2, const TGAColor& col);
	void line(int x1, int y1, int x2, int y2, const TGAColor& col);
	void drawObj(std::string objPath, const TGAColor& col, DrawMode mode = DrawMode::WireFrameIgnoreZ);
};