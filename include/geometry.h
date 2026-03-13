#pragma once
#include <cmath>
#include <cassert>
#include <iostream>
#include <type_traits>


// 向量定义
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
    vec<2, T> xy() { return {x, y}; }
};

template <typename T> struct vec<4, T> {
    T x, y, z, w;
    vec() : x(T()), y(T()), z(T()), w(T()) {}
    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
    T& operator[](const size_t i) { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
    const T& operator[](const size_t i) const { assert(i < 4); return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w)); }
    vec<2, T> xy() { return { x, y }; }
};


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



template <size_t Dim, typename T, typename U,
    typename = std::enable_if_t<std::is_arithmetic<U>::value>>
    vec<Dim, T> operator*(const U& lhs, vec<Dim, T> rhs) {
    for (size_t i = 0; i < Dim; i++) rhs[i] = rhs[i] * lhs;
    return rhs;
}

template <size_t Dim, typename T, typename U,
    typename = std::enable_if_t<std::is_arithmetic<U>::value>>
    vec<Dim, T> operator*(vec<Dim, T> lhs, const U& rhs) {
    for (size_t i = 0; i < Dim; i++) lhs[i] = lhs[i] * rhs;
    return lhs;
}

template <size_t Dim, typename T, typename U,
    typename = std::enable_if_t<std::is_arithmetic<U>::value>>
    vec<Dim, T> operator/(vec<Dim, T> lhs, const U& rhs) {
    for (size_t i = 0; i < Dim; i++) lhs[i] = lhs[i] / rhs;
    return lhs;
}


template <size_t Dim, typename T> std::ostream& operator<<(std::ostream& out, const vec<Dim, T>& v) {
    for (unsigned int i = 0; i < Dim; i++) out << v[i] << (i < Dim - 1 ? " " : "");
    return out;
}



template <typename T> T cross(vec<2, T> v1, vec<2, T> v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2) {
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}





template <size_t row, size_t col, typename T> class mat;

template <size_t row, size_t col, typename T> class mat {
    vec<col, T> rows[row];

public:
    mat() {}
    mat(std::initializer_list<T> list) {
        assert(list.size() == row * col);
        auto it = list.begin();
        for (size_t i = 0; i < row; i++) {
            for (size_t j = 0; j < col; j++) {
                rows[i][j] = *it;
                it++;
            }
        }
    }
    vec<col, T>& operator[](const size_t i) { assert(i <  row); return rows[i]; }
    const vec<col, T>& operator[](const size_t i) const { assert(i < row); return rows[i]; }
    vec<row, T> Ccol(const size_t i) const {
        assert(i < col);
        vec<row, T> ret;
        for (size_t j = 0; j < row; j++)
            ret[j] = rows[j][i];
        return ret;
    }
    float det() const{
        return (*this)[0][0] * ((*this)[1][1] * (*this)[2][2] - (*this)[1][2] * (*this)[2][1]) -
            (*this)[0][1] * ((*this)[1][0] * (*this)[2][2] - (*this)[1][2] * (*this)[2][0]) +
            (*this)[0][2] * ((*this)[1][0] * (*this)[2][1] - (*this)[1][1] * (*this)[2][0]);
    }

    static mat<row, col, T> identity() {
        mat<row, col, T> I;
        for (size_t i = 0; i < row; i++)
            for (size_t j = 0; j < col; j++)
                I[i][j] = (i == j) ? 1 : 0;
        return I;
    }
    mat<col, row, T> transpose() const {
        mat<col, row, T> res;
        for (size_t i = 0; i < row; i++)
            for (size_t j = 0; j < col; j++)
                res[j][i] = (*this)[i][j];
        return res;
    }

    mat<row, col, T> invert() const {
        mat<row, col, T> res;
        float d = det();
        if (std::abs(d) < 1e-6) return res;

        res[0][0] = (*this)[1][1] * (*this)[2][2] - (*this)[1][2] * (*this)[2][1];
        res[0][1] = (*this)[0][2] * (*this)[2][1] - (*this)[0][1] * (*this)[2][2];
        res[0][2] = (*this)[0][1] * (*this)[1][2] - (*this)[0][2] * (*this)[1][1];

        res[1][0] = (*this)[1][2] * (*this)[2][0] - (*this)[1][0] * (*this)[2][2];
        res[1][1] = (*this)[0][0] * (*this)[2][2] - (*this)[0][2] * (*this)[2][0];
        res[1][2] = (*this)[0][2] * (*this)[1][0] - (*this)[0][0] * (*this)[1][2];

        res[2][0] = (*this)[1][0] * (*this)[2][1] - (*this)[1][1] * (*this)[2][0];
        res[2][1] = (*this)[0][1] * (*this)[2][0] - (*this)[0][0] * (*this)[2][1];
        res[2][2] = (*this)[0][0] * (*this)[1][1] - (*this)[0][1] * (*this)[1][0];

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                res[i][j] /= d;

        return res;
    }

    mat<row, col, T> invert_transpose() const {
        return invert().transpose();
    }

};

template <size_t row, size_t col, typename T> vec<row, T> operator*(const mat<row, col, T>& lhs, const vec<col, T>& rhs) {
    vec<row, T> ret;
    for (size_t i = 0; i < row; i++) 
        ret[i] = rhs * lhs[i];
    return ret;
}

template<size_t row1, size_t col1, size_t col2, typename T> mat<row1, col2, T> operator*(const mat<row1, col1, T>& lhs, const mat<col1, col2, T>& rhs) {
    mat<row1, col2, T> res;

    for (size_t i = 0; i < row1; i++)
        for(size_t j = 0; j < col2; j++)
            res[i][j] = rhs.Ccol(j) * lhs[i];
    
    return res;
}


typedef vec<2, float> Vec2f;
typedef vec<3, float> Vec3f;
typedef vec<4, float> Vec4f;
typedef vec<2, int>   Vec2i;
typedef vec<3, int>   Vec3i;
typedef mat<3, 3, float> Matrix3f;
typedef mat<4, 4, float> Matrix4f;


