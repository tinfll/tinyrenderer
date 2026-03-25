# CLAUDE.md — tinyrenderer

## Project Overview

**tinyrenderer** is a from-scratch software 3D renderer written in C++20. It implements a complete GPU-style rendering pipeline entirely in software — no OpenGL, DirectX, or Vulkan. This is a learning/educational project demonstrating computer graphics fundamentals including line drawing, rasterization, depth buffering, texture mapping, Phong shading, normal mapping, and shadow mapping.

The renderer outputs TGA image files (`image.tga`, `z.tga`).

---

## Building and Running

### Requirements
- CMake >= 3.12
- C++20-capable compiler (GCC, Clang, or Intel)
- OpenMP (optional, enables parallel rendering)

### Build
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Run
```bash
./tinyrenderer
# Outputs: image.tga (rendered scene), z.tga (depth buffer visualization)
```

### Build Notes
- Default build type is **Release** when not using a multi-config generator
- `-Wall` warnings enabled for Clang/GCC/Intel compilers
- `include-what-you-use` static analysis is optionally supported
- Source files are auto-collected: all `.cpp` in `src/` plus `main.cpp`

---

## Directory Structure

```
tinyrenderer/
├── CMakeLists.txt          # CMake build configuration (C++20, OpenMP optional)
├── README.md               # Project overview with render stage screenshots
├── Catchhaa.md             # Developer troubleshooting log (12 documented bugs)
├── main.cpp                # Entry point; PhongShader and shadowmap shader implementations
├── src/
│   ├── model.cpp           # OBJ file parser (vertices, normals, texture coords, faces)
│   ├── our_gl.cpp          # Core rendering pipeline (lookat, projection, rasterization)
│   └── tgaimage.cpp        # TGA image format read/write with RLE support
├── include/
│   ├── geometry.h          # Math library: Vec2f/Vec3f/Vec4f, Matrix3f/Matrix4f
│   ├── myVector.h          # Custom dynamic array template (qmhsV<T>)
│   ├── model.h             # Model class declaration
│   ├── our_gl.h            # IShader interface, tinfgl renderer class, extern globals
│   ├── rasterization.h     # (Minimal placeholder)
│   └── tgaimage.h          # TGAColor, TGAImage structs
├── obj/                    # 3D model assets (OBJ + textures)
│   ├── african_head/       # Head model with diffuse, normal, specular maps
│   ├── boggie/             # Multi-part character (body, head, eyes)
│   ├── diablo3_pose/       # Diablo 3 character model
│   └── qmhs/               # Additional model
├── renderProcess/          # Screenshot progression of renderer development
└── troubleshootings/       # Debug render images documenting fixed bugs
```

---

## Architecture

### Rendering Pipeline

```
Model (OBJ) → Vertex Shader → Perspective Divide → Viewport → Rasterization → Fragment Shader → TGA Output
```

Two-pass rendering for shadow mapping:
1. **Shadow pass** (`shadowmap` shader): Render depth from light's POV into `zbuffer2`
2. **Color pass** (`PhongShader`): Render final scene, sampling `zbuffer2` to determine shadow

### Global State (`our_gl.h`)

```cpp
extern Matrix4f modelv;      // View matrix (camera transform)
extern Matrix4f perspo;      // Perspective projection matrix
extern Matrix4f viewp;       // Viewport transform
extern Matrix4f Lmodelv;     // Light view matrix (shadow pass)
extern Matrix4f Lperspo;     // Light perspective matrix (shadow pass)
extern qmhsV<float> zbuffer;  // Main depth buffer
extern qmhsV<float> zbuffer2; // Shadow map depth buffer
```

### Shader Interface (`include/our_gl.h`)

```cpp
struct IShader {
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual pair<bool, TGAColor> fragment(const Vec3f bar) const = 0;
};
```

- `vertex()`: Receives face index and vertex index (0–2); returns clip-space position
- `fragment()`: Receives barycentric coordinates; returns `{discard, color}`

---

## Key Classes and Files

### `include/geometry.h` — Math Library

```cpp
// Vector types
Vec2f, Vec2i          // 2D vectors
Vec3f, Vec3i          // 3D vectors (most commonly used)
Vec4f                 // Homogeneous coordinates

// Matrix types
Matrix3f              // 3x3 matrix
Matrix4f              // 4x4 matrix (transformations)

// Key operations
cross(a, b)           // Cross product
dot(a, b)             // Dot product (operator*)
norm(v)               // Vector magnitude
normalize(v)          // Unit vector
mat.invert()          // Matrix inverse
mat.transpose()
mat.invert_transpose()
mat.det()             // Determinant (3x3 only via cofactor expansion)
```

Vectors use template specialization: `vec<N, T>` with specializations for N=2,3,4. Matrices are row-major.

### `include/myVector.h` — Custom Container

`qmhsV<T>` is a custom dynamic array (like `std::vector`). Used throughout instead of `std::vector` for project consistency. Supports `push_back()`, `resize()`, `clear()`, `operator[]`, and range-based for loops.

### `include/model.h` / `src/model.cpp` — 3D Model

```cpp
class Model {
    // Accessors
    int nfaces()                    // Number of triangle faces
    Vec4f vert(int iface, int j)    // Vertex position (j = 0,1,2)
    Vec4f normal(int iface, int j)  // Normal vector
    Vec4f uv(int iface, int j)      // Texture coordinate

    // Texture maps (TGAImage)
    TGAImage diffusemap_
    TGAImage normalmap_
    TGAImage specularmap_
};
```

- Parses OBJ format: `v` (vertices), `vt` (texture), `vn` (normals), `f` (faces)
- Quad faces are split into two triangles automatically
- Vertex positions stored as `Vec4f` with w=1; normals with w=0

### `include/our_gl.h` / `src/our_gl.cpp` — Renderer

```cpp
class tinfgl {
    void lookat(Vec3f eye, Vec3f center, Vec3f up);
    void init_perspective(float fov);
    void init_viewport(int x, int y, int width, int height);
    void rasterization(Vec4f v0, Vec4f v1, Vec4f v2, IShader&, TGAImage&, qmhsV<float>& zbuf);
    void rasterizationL(Vec4f v0, Vec4f v1, Vec4f v2, IShader&, TGAImage&, qmhsV<float>& zbuf);
};
```

Rasterization uses barycentric coordinates to interpolate depth and call `fragment()` per pixel.

### `main.cpp` — Shaders and Entry Point

**`PhongShader`**: Full Phong lighting with textures and shadows
- Samples diffuse, normal, specular maps
- Computes TBN matrix for tangent-space normal mapping
- Applies shadow attenuation (0.3×) when fragment is in shadow

**`shadowmap`**: Depth-only shader for shadow map generation

**`main()`**: Sets up camera, light, loads models, runs two-pass rendering

---

## Code Conventions

### Naming
| Element | Convention | Example |
|---|---|---|
| Classes | PascalCase | `Model`, `TGAImage`, `PhongShader` |
| Functions | camelCase or snake_case | `lookat`, `init_perspective`, `push_back` |
| Variables | camelCase | `eyePos`, `lightDir`, `zbuffer` |
| Type aliases | PascalCase + suffix | `Vec3f`, `Matrix4f` |
| Member variables | trailing underscore | `verts_`, `m_size`, `diffusemap_` |
| Constants | UPPER_CASE or constexpr | `MY_PI`, `blue`, `white` |

### Style
- Brace style: Egyptian (opening brace same line)
- Indentation: 4 spaces
- Templates heavily used for math types (`vec<N,T>`, `mat<R,C,T>`)
- Minimal comments; code is expected to be self-documenting
- Global state (transformation matrices, depth buffers) via `extern` variables

### Patterns
- **Shader polymorphism**: Inherit `IShader`, implement `vertex()` and `fragment()`
- **Homogeneous coordinates**: Always work in 4D; divide by w for NDC
- **BGRA color order**: `TGAColor` stores channels as `bgra[4]`
- **Row-major matrices**: Matrix indexing is `mat[row][col]`

---

## Adding New Shaders

1. Define a struct inheriting `IShader` in `main.cpp` (or a new file)
2. Implement `Vec4f vertex(int iface, int nthvert)`:
   - Use global `modelv`, `perspo`, `viewp` to transform vertices
   - Store per-vertex interpolation data in member arrays
3. Implement `pair<bool, TGAColor> fragment(const Vec3f bar) const`:
   - Use barycentric `bar` to interpolate stored data
   - Return `{true, color}` to render or `{false, {}}` to discard
4. Instantiate and pass to `tinfgl::rasterization()`

---

## Testing / Validation

There are **no automated tests**. Validation is purely visual:
- Run the renderer and inspect `image.tga` (rendered output)
- Inspect `z.tga` (depth buffer visualization)
- Compare against reference images in `renderProcess/`
- Debug artifacts documented in `troubleshootings/`

Use any TGA-capable viewer (e.g., GIMP, Preview on macOS, online TGA viewers).

---

## Known Quirks and Gotchas

- **w-component confusion**: Camera position must use w=1 (point), light direction w=0 (vector). Mixing these causes incorrect transforms — see `Catchhaa.md` bug #4.
- **Normal normalization**: Normals from the normal map must be re-normalized after TBN transform to avoid lighting artifacts.
- **Shadow map bias**: No explicit depth bias is applied; shadow acne may occur with certain light angles.
- **Depth buffer initialization**: `zbuffer` and `zbuffer2` must be resized and initialized to `-std::numeric_limits<float>::max()` before rendering.
- **TGA flip**: `TGAImage::flip_vertically()` is called after rendering to match standard image coordinate orientation (Y-up).

---

## Git Workflow

Development branches follow the pattern:
```
claude/<feature-name>-<id>
```

Always develop on the designated feature branch and push with:
```bash
git push -u origin <branch-name>
```
