#include "TGAFile.h"

#include <iostream>
#include <chrono>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);

int main() {
    TGAFile img(800, 800, TGAFile::RGB);
	img.drawObj("obj/african_head/african_head.obj", white);
	//img.line(50, 50, 200, 500, white);
    img.flip_vertically();
    img.write_tga_file("isura1.tga");
	img.show();

    return 0;
}