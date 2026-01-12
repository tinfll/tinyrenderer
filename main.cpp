#include <cmath>
#include <ctime>
#include <cstdlib>
#include <chrono> // 用于精确计时
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax - bx) < std::abs(ay - by);
    if(steep) {
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax > bx) {
        std::swap(ax, bx);
		std::swap(ay, by);
    }
    float y = ay;
    int ie = 0;
    for (int x = ax; x <= bx; x++) {
	    if(steep)
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ie += 2 * std::abs(by - ay);
        y += (by - ay) / static_cast<float>(bx - ax);
        ie -= 2 * (bx - ax) * (ie > bx - ax);
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    /*int ax = 7, ay = 3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    line(ax, ay, bx, by, framebuffer, red);
    line(bx, by, cx, cy, framebuffer, green);
    line(ax, ay, cx, cy, framebuffer, blue);
    

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    
    framebuffer.write_tga_file("framebuffer.tga");
    */
    
    /*std::srand(std::time({}));
    for (int i = 0; i < (1 << 24); i++) {
        int ax = rand() % width, ay = rand() % height;
        int bx = rand() % width, by = rand() % height;
        line(ax, ay, bx, by, framebuffer, { static_cast<unsigned char>(rand() % 255), 
                                            static_cast<unsigned char>(rand() % 255), 
                                            static_cast<unsigned char>(rand() % 255), 
                                            static_cast<unsigned char>(rand() % 255 )});
    }

    framebuffer.write_tga_file("framebuffer.tga");
    */

    printf("Generating random numbers...\n");
    std::vector<int> rand_cache;
    rand_cache.reserve(20000000);
    std::srand(std::time(nullptr));
    for (int i = 0; i < 20000000; i++) {
        rand_cache.push_back(std::rand());
    }

    printf("Start rendering...\n");

    // 2. 计时开始
    auto start = std::chrono::high_resolution_clock::now();

    int idx = 0;
    for (int i = 0; i < 1000000; i++) { 
        int x0 = rand_cache[idx++] % width;
        int y0 = rand_cache[idx++] % height;
        int x1 = rand_cache[idx++] % width;
        int y1 = rand_cache[idx++] % height;


        line(x0, y0, x1, y1, framebuffer, white);

        // 简单的越界保护
        if (idx >= 19999990) idx = 0;
    }

    // 3. 计时结束
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    printf("Time taken: %f s\n", diff.count());

    return 0;
}