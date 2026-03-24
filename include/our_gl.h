#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <algorithm>

extern Matrix4f modelv;
extern Matrix4f perspo;
extern Matrix4f viewp;
extern Matrix4f Lmodelv;
extern Matrix4f Lperspo;
extern qmhsV<float> zbuffer;
extern qmhsV<float> zbuffer2;

struct IShader {
	virtual ~IShader() {}
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	virtual std::pair<bool, TGAColor> fragment(const Vec3f bar) const = 0;
};

typedef Vec4f triangle[3];
void line(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color);

class tinfgl {
public:
	void lookat(const Vec3f eye, const Vec3f center, const Vec3f up);
	void lookatL(const Vec3f eye, const Vec3f center, const Vec3f up);
	void init_Lperspo(const float f);
	void init_perspective(const float f);
	void init_viewport(const int x, const int y, const int width, const int height);
	void initZ(int width, int height);
	void rasterization(Vec4f a0, Vec4f a1, Vec4f a2,
		TGAImage& image, TGAImage& z, int width, int height
	, IShader &shader);
	void rasterizationL(Vec4f a0, Vec4f a1, Vec4f a2, int width, int height);
	void drawZ(qmhsV<float>& zbuffer, int width, int height, TGAImage z);
};