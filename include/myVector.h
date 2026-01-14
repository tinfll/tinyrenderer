#pragma once
#include<cassert>
#include<algorithm>
#include<iostream>


template <typename T>
class qmhsV {
	T* data = nullptr;
	size_t m_size;
	size_t m_capacity;

public:
	qmhsV() :data(nullptr), m_size(0), m_capacity(0) {}
	~qmhsV() {
		delete[] data;
	}
	qmhsV(const qmhsV<T>& other) {
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		data = new T[other.m_size];
		std::copy(other.data, other.data + m_size, data);
	}

	qmhsV<T>& operator=(const qmhsV<T>& other) {
		if (this == &other) return *this;
		delete[] data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		data = new T[m_capacity];
		std::copy(other.data, other.data + m_size, data);
		return *this;
	}

	
	void reverse(size_t newcap) {
		if (newcap <= m_capacity) return;
		T* nV = new T[newcap];
		if (data != nullptr) {
			std::copy(data, data + m_size , nV);
			delete[] data;
		}
		data = nV;
		m_capacity = newcap;
	}
	void push_back(const T& value) {
		if (m_size >= m_capacity) {
			size_t newcap = (m_capacity == 0) ? 1 : 2 * m_capacity;
			reverse(newcap);
		}
		data[m_size] = value;
		m_size++;
	}


	size_t size() const { return m_size; }
	size_t capacity() const { return m_capacity; }

	T& operator[](size_t index) {
		assert(index < m_size);
		return data[index];
	}

	const T& operator[](size_t index) const {
		assert(index < m_size);
		return data[index];
	}
	T* begin() { return data; }
	T* end() { return data + m_size; }

	const T* begin() const { return data; }
	const T* end() const { return data + m_size; }

};

