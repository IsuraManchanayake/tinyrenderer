#pragma once

#include "tgaimage.h"
#include "geometry.h"

enum class DrawMode {
	Filled,
	WireFrame,
	WireFrameIgnoreZ
};

#define LINE_FLOAT
//#define TRIANGLE_LINE_SWEEP

struct TGAFile : public TGAImage {

	using TGAImage::TGAImage;
	using TGAImage::Format;

	void show() const;
	void line(int x1, int y1, int x2, int y2, const TGAColor& stroke);
	void line(const Vec2i& v1, const Vec2i& v2, const TGAColor& stroke);
	void triangle(const Vec2i& v1, const Vec2i& v2, const Vec2i& v3, const TGAColor& fill, DrawMode mode = DrawMode::WireFrame);
	void triangle(const Vec2i& v1, const Vec2i& v2, const Vec2i& v3, const TGAColor& stroke, const TGAColor& fill, DrawMode mode = DrawMode::WireFrame);
	void drawObj(std::string objPath, const TGAColor& col, DrawMode mode = DrawMode::WireFrameIgnoreZ);
};