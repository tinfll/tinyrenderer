#pragma once // <--- 关键！没有这个，只要被include两次就会报错
#include <cmath>
#include <cassert>
#include <iostream>

// =========================================================================
// 向量定义 (Vector)
// =========================================================================

template <size_t Dim, typename T> struct vec {
    T data[Dim];
    vec() { for (size_t i = 0; i < Dim; i++) data[i] = T(); }
    T& operator[](const size_t i) { assert(i < Dim); return data[i]; }
    const T& operator[](const size_t i) const { assert(i < Dim); return data[i]; }
};

template <typename T> struct vec<2, T> {
    T x, y;
    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }
};

template <typename T> struct vec<3, T> {
    T x, y, z;
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    T& operator[](const size_t i) { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    vec<3, T>& normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }
};

template <typename T> struct vec<4, T> {
    T x, y, z, w;
    vec() : x(T()), y(T()), z(T()), w(T()) {}
    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
    T& operator[](const size_t i) { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
    const T& operator[](const size_t i) const { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
};

// =========================================================================
// 向量运算 (Vector Operations)
// =========================================================================

template <size_t Dim, typename T> vec<Dim, T> operator+(vec<Dim, T> lhs, const vec<Dim, T>& rhs) {
    for (size_t i = 0; i < Dim; i++) lhs[i] = lhs[i] + rhs[i];
    return lhs;
}



template <size_t Dim, typename T> vec<Dim, T> operator-(vec<Dim, T> lhs, const vec<Dim, T>& rhs) {
    for (size_t i = 0; i < Dim; i++) lhs[i] = lhs[i] - rhs[i];
    return lhs;
}

// 点积
template <size_t Dim, typename T> T operator*(const vec<Dim, T>& lhs, const vec<Dim, T>& rhs) {
    T ret = T();
    for (size_t i = 0; i < Dim; i++) ret += lhs[i] * rhs[i];
    return ret;
}

template <size_t Dim, typename T> T dot(const vec<Dim, T>& lhs, const vec<Dim, T>& rhs) {
    return lhs * rhs;
}

// 标量乘法
template <size_t Dim, typename T, typename U> vec<Dim, T> operator*(vec<Dim, T> lhs, const U& rhs) {
    for (size_t i = 0; i < Dim; i++) lhs[i] = lhs[i] * rhs;
    return lhs;
}

template <size_t Dim, typename T, typename U> vec<Dim, T> operator*(const U& lhs, vec<Dim, T> rhs) {
    for (size_t i = 0; i < Dim; i++) rhs[i] = rhs[i] * lhs;
    return rhs;
}

template <size_t Dim, typename T, typename U> vec<Dim, T> operator/(vec<Dim, T> lhs, const U& rhs) {
    for (size_t i = 0; i < Dim; i++) lhs[i] = lhs[i] / rhs;
    return lhs;
}

template <size_t Dim, typename T> std::ostream& operator<<(std::ostream& out, const vec<Dim, T>& v) {
    for (unsigned int i = 0; i < Dim; i++) out << v[i] << (i < Dim - 1 ? " " : "");
    return out;
}

// 辅助函数

template <typename T> T cross(vec<2, T> v1, vec<2, T> v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2) {
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

// =========================================================================
// 矩阵定义 (Matrix)
// =========================================================================

template<size_t DimCols, size_t DimRows, typename T> class mat;

template<size_t Dim, typename T> struct dt {
    static T det(const mat<Dim, Dim, T>& src) {
        T ret = 0;
        for (size_t i = 0; i < Dim; i++) ret += src[0][i] * src.cofactor(0, i);
        return ret;
    }
};

template<typename T> struct dt<1, T> {
    static T det(const mat<1, 1, T>& src) { return src[0][0]; }
};

template<size_t DimRows, size_t DimCols, typename T> class mat {
    vec<DimCols, T> rows[DimRows];
public:
    mat() {}
    mat(const T* vals) { // 初始化
        for (size_t i = 0; i < DimRows; i++) for (size_t j = 0; j < DimCols; j++) rows[i][j] = vals[i + j * DimCols];
    }

    vec<DimCols, T>& operator[](const size_t i) { assert(i < DimRows); return rows[i]; }
    const vec<DimCols, T>& operator[](const size_t i) const { assert(i < DimRows); return rows[i]; }

    vec<DimRows, T> col(const size_t i) const {
        assert(i < DimCols);
        vec<DimRows, T> ret;
        for (size_t j = 0; j < DimRows; j++) ret[j] = rows[j][i];
        return ret;
    }

    void set_col(size_t i, vec<DimRows, T> v) {
        assert(i < DimCols);
        for (size_t j = 0; j < DimRows; j++) rows[j][i] = v[j];
    }

    static mat<DimRows, DimCols, T> identity() {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = 0; i < DimRows; i++) for (size_t j = 0; j < DimCols; j++) ret[i][j] = (i == j);
        return ret;
    }

    T det() const { return dt<DimCols, T>::det(*this); }

    mat<DimRows - 1, DimCols - 1, T> get_minor(size_t row, size_t col) const {
        mat<DimRows - 1, DimCols - 1, T> ret;
        for (size_t i = 0; i < DimRows - 1; i++)
            for (size_t j = 0; j < DimCols - 1; j++)
                ret[i][j] = rows[i < row ? i : i + 1][j < col ? j : j + 1];
        return ret;
    }

    T cofactor(size_t row, size_t col) const {
        return get_minor(row, col).det() * ((row + col) % 2 ? -1 : 1);
    }

    mat<DimRows, DimCols, T> adjugate() const {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = 0; i < DimRows; i++) for (size_t j = 0; j < DimCols; j++) ret[i][j] = cofactor(i, j);
        return ret;
    }

    mat<DimCols, DimRows, T> transpose() {
        mat<DimCols, DimRows, T> ret;
        for (size_t i = 0; i < DimCols; i++) for (size_t j = 0; j < DimRows; j++) ret[i][j] = rows[j][i];
        return ret;
    }

    mat<DimRows, DimCols, T> invert_transpose() {
        mat<DimRows, DimCols, T> ret = adjugate();
        T tmp = ret[0] * rows[0];
        return ret / tmp;
    }

    mat<DimRows, DimCols, T> invert() {
        return invert_transpose().transpose();
    }
};

// =========================================================================
// 矩阵运算 (Matrix Operations)
// =========================================================================

template<size_t DimRows, size_t DimCols, typename T>
vec<DimRows, T> operator*(const mat<DimRows, DimCols, T>& lhs, const vec<DimCols, T>& rhs) {
    vec<DimRows, T> ret;
    for (size_t i = 0; i < DimRows; i++) ret[i] = lhs[i] * rhs;
    return ret;
}

template<size_t R1, size_t C1, size_t C2, typename T>
mat<R1, C2, T> operator*(const mat<R1, C1, T>& lhs, const mat<C1, C2, T>& rhs) {
    mat<R1, C2, T> result;
    for (size_t i = 0; i < R1; i++) {
        for (size_t j = 0; j < C2; j++) {
            result[i][j] = lhs[i] * rhs.col(j);
        }
    }
    return result;
}

template<size_t DimRows, size_t DimCols, typename T>
mat<DimRows, DimCols, T> operator/(mat<DimRows, DimCols, T> lhs, const T& rhs) {
    for (size_t i = 0; i < DimRows; i++) lhs[i] = lhs[i] / rhs;
    return lhs;
}

// 类型别名
typedef vec<2, float> Vec2f;
typedef vec<3, float> Vec3f;
typedef vec<4, float> Vec4f;
typedef vec<2, int>   Vec2i;
typedef vec<3, int>   Vec3i;
typedef mat<4, 4, float> Matrix4;
typedef mat<3, 3, float> Matrix3;