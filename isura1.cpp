#include "TGAFile.h"

#include <iostream>
#include <chrono>

#include <random>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);

int main() {
    TGAFile img(800, 800, TGAFile::RGB);
	//img.drawObj("obj/african_head/african_head.obj", white);
	//img.line(50, 50, 200, 500, white);

	//Vec2i v1(100, 100);
	//Vec2i v2(700, 300);
	//Vec2i v3(500, 700);
	//img.triangle(v1, v2, v3, white, red, DrawMode::Filled);

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> distw(0, img.get_width());
	std::uniform_int_distribution<std::mt19937::result_type> disth(0, img.get_height());
	Vec2i v1(distw(rng), disth(rng));
	Vec2i v2(distw(rng), disth(rng));
	Vec2i v3(distw(rng), disth(rng));
	img.triangle(v1, v2, v3, white, red, DrawMode::Filled);

    img.flip_vertically();
    img.write_tga_file("isura1.tga");
	img.show();

    return 0;
}