#include "TGAFile.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <random>

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#endif

void TGAFile::show() const {
#if defined(_WIN32)
	ShellExecute(0, 0, "isura1.tga", 0, 0, SW_SHOW);
#endif
}

void TGAFile::line(const Vec2i& v1, const Vec2i& v2, const TGAColor& stroke) {
	line(v1.x, v1.y, v2.x, v2.y, stroke);
}

#if defined(LINE_FLOAT)
void TGAFile::line(int x1, int y1, int x2, int y2, const TGAColor& stroke) {
	if (x1 == x2 && y1 == y2) return;
	int dx = std::abs(x2 - x1);
	int dy = std::abs(y2 - y1);
	bool flat = true;
	if (dy > dx) {
		std::swap(x1, y1);
		std::swap(x2, y2);
		flat = false;
	}
	if (x2 <= x1) {
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	float m = (y2 - y1 + 0.0) / (x2 - x1);
	float c = y1 - m * x1;
	for (int x = x1; x <= x2; x++) {
		int y = m * x + c;
		if (flat) {
			set(x, y, stroke);
		}
		else {
			set(y, x, stroke);
		}
	}
}
#else
void TGAFile::line(int x0, int y0, int x1, int y1, const TGAColor& color) {
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (steep) {
			set(y, x, color);
		}
		else {
			set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx) {
			y += (y1 > y0 ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}
#endif

void TGAFile::drawObj(std::string objPath, const TGAColor& stroke, DrawMode mode) {
	std::ifstream ifs(objPath, std::ifstream::in);
	if (!ifs.good()) { return; }
	auto xytransform = [this](Vec3f xy) -> Vec2i { float factor = std::min(width, height); return { (int)(factor * ((xy.x + 1) / 2)), (int)(factor * ((xy.y + 1) / 2)) }; };
	std::vector<Vec3f> vecs;
	std::vector<Vec3f> normals;
	std::vector<std::array<size_t, 3>> faceIndices;
	std::vector<std::array<size_t, 3>> normalIndices;
	for (std::string line; std::getline(ifs, line); ) {
		std::stringstream ss(line);
		std::string typeStr;
		ss >> typeStr;
		if (typeStr == "v") {
			Vec3f coordf;
			ss >> coordf.x >> coordf.y >> coordf.z;
			vecs.emplace_back(coordf);
		} else if (typeStr == "vn") {
			Vec3f coordf;
			ss >> coordf.x >> coordf.y >> coordf.z;
			normals.emplace_back(coordf);
		} else if (typeStr == "f") {
			std::array<size_t, 3> face;
			std::array<size_t, 3> normal;
			for (size_t i = 0; i < 3; i++) {
				std::string vtf;
				ss >> vtf;
				size_t pos1 = vtf.find('/');
				size_t pos2 = vtf.find('/', pos1 + 1);
				std::string vidx = vtf.substr(0, pos1);
				std::string nidx = vtf.substr(pos2 + 1, vtf.size());
				std::stringstream vidxss(vidx);
				std::stringstream nidxss(nidx);
				vidxss >> face[i];
				nidxss >> normal[i];
			}
			faceIndices.emplace_back(face);
			normalIndices.emplace_back(normal);
		}
	}
	if (mode == DrawMode::WireFrameIgnoreZ) {
		for(size_t i = 0; i < faceIndices.size(); i++) {
			const auto& face = faceIndices[i];
			const auto& normal = normalIndices[i];
			const auto& v0 = vecs[face[0] - 1];
			const auto& v1 = vecs[face[1] - 1];
			const auto& v2 = vecs[face[2] - 1];
			Vec3f n(cross(v1 - v0, v2 - v0));
			n.normalize();
			Vec3f light{ 0, 0, 1 };
			float value = 255 * (light * n);
			TGAColor intensity(value, value, value, 255);
			if (value > 0) {
				triangle(xytransform(v0), xytransform(v1), xytransform(v2), intensity, DrawMode::Filled);
			}
		}
	}
}

void TGAFile::triangle(const Vec2i& v1, const Vec2i& v2, const Vec2i& v3, const TGAColor& fill, DrawMode mode) {
	triangle(v1, v2, v3, fill, fill, mode);
}

void TGAFile::triangle(const Vec2i& v1, const Vec2i& v2, const Vec2i& v3, const TGAColor& stroke, const TGAColor& fill, DrawMode mode) {
	if (mode == DrawMode::WireFrame) {
		line(v1, v2, stroke);
		line(v2, v3, stroke);
		line(v3, v1, stroke);
	}
	else if (mode == DrawMode::Filled) {
		line(v1, v2, stroke);
		line(v2, v3, stroke);
		line(v3, v1, stroke);
		
#if defined(TRIANGLE_LINE_SWEEP)
		std::vector<Vec2i> vs{ v1, v2, v3 };
		std::sort(vs.begin(), vs.end(), [](const auto& vl, const auto& vr) { return vl.y > vr.y; });
		int x0 = vs[0].x;
		int y0 = vs[0].y;
		int x1 = vs[1].x;
		int y1 = vs[1].y;
		int x2 = vs[2].x;
		int y2 = vs[2].y;
		float t_ = (y1 - y2 + 0.0) / (y0 - y2);
		int x_ = x2 + t_ * (x0 - x2);
		// bottom half
		for (int y = y1; y > y2; y--) {
			float t1 = (y - y2 + 0.0) / (y0 - y2);
			float t2 = (y - y2 + 0.0) / (y1 - y2);
			int x1_ = x2 + t1 * (x0 - x2);
			int x2_ = x2 + t2 * (x1 - x2);
			line(x1_ + 1, y, x2_ - 1, y, fill);
		}
		// top half
		for (int y = y1; y < y0; y++) {
			float t1 = (y - y2 + 0.0) / (y0 - y2);
			float t2 = (y - y1 + 0.0) / (y0 - y1);
			int x1_ = x2 + t1 * (x0 - x2);
			int x2_ = x1 + t2 * (x0 - x1);
			line(x1_ + 1, y, x2_ - 1, y, fill);
		}
#else
		std::vector<Vec2i> vs{ v1, v2, v3 };
		Vec2i boundmin{ width - 1, height - 1 };
		Vec2i boundmax{ 0, 0 };
		for (const auto& v : vs) {
			boundmin.x = std::min(boundmin.x, v.x);
			boundmin.y = std::min(boundmin.y, v.y);
			boundmax.x = std::max(boundmax.x, v.x);
			boundmax.y = std::max(boundmax.y, v.y);
		}
		for (int y = boundmin.y; y <= boundmax.y; y++) {
			for (int x = boundmin.x; x <= boundmax.x; x++) {
				Vec2i av = vs[1] - vs[0];
				Vec2i bv = vs[2] - vs[0];
				Vec2i pv = Vec2i{ x, y } - vs[0];
				Vec3i xx{ av.x, bv.x, pv.x };
				Vec3i yy{ av.y, bv.y, pv.y };
				Vec3f ortho(cross(xx, yy));
				ortho /= -ortho[2];
				if (ortho[0] >= 0 && ortho[1] >= 0 && ortho[0] + ortho[1] <= 1) {
					set(x, y, fill);
				}
			}
		}
#endif
	}
}