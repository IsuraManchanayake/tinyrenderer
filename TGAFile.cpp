#include "TGAFile.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <array>

void TGAFile::show() const {
#if defined(_WIN32)
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess("view.bat", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Image viewer open failed (%d).\n", GetLastError());
		exit(-1);
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#endif
}

void TGAFile::line(const std::pair<int, int>& v1, const std::pair<int, int>& v2, const TGAColor& col) {
	line(v1.first, v1.second, v2.first, v2.second, col);
}

#if defined(FLOAT_LINE_CALC)
void TGAFile::line(int x1, int y1, int x2, int y2, const TGAColor& col) {
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
			set(x, y, col);
		}
		else {
			set(y, x, col);
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

void TGAFile::drawObj(std::string objPath, const TGAColor& col, DrawMode mode) {
	std::ifstream ifs(objPath, std::ifstream::in);
	if (!ifs.good()) { return; }
	auto xytransform = [this](std::pair<float, float> xy) -> std::pair<int, int> { return {width * ((xy.first + 1) / 2), height * ((xy.second + 1) / 2)}; };
	std::vector<std::pair<int, int>> vecs;
	std::vector<std::array<size_t, 3>> faces;
	for (std::string line; std::getline(ifs, line); ) {
		std::stringstream ss(line);
		std::string typeStr;
		ss >> typeStr;
		if (typeStr == "v") {
			std::pair<float, float> coordf;
			float ignore;
			ss >> coordf.first >> coordf.second >> ignore;
			vecs.emplace_back(xytransform(coordf));
		} else if (typeStr == "f") {
			std::array<size_t, 3> face;
			for (size_t i = 0; i < 3; i++) {
				std::string vtf;
				ss >> vtf;
				std::string vidx = vtf.substr(0, vtf.find('/'));
				std::stringstream vidxss(vidx);
				vidxss >> face[i];
			}
			faces.emplace_back(face);
		}
	}
	if (mode == DrawMode::WireFrameIgnoreZ) {
		for (const auto& face : faces) {
			const auto& v1 = vecs[face[0] - 1];
			const auto& v2 = vecs[face[1] - 1];
			const auto& v3 = vecs[face[2] - 1];
			line(v1, v2, col);
			line(v2, v3, col);
			line(v3, v1, col);
		}
	}
}
