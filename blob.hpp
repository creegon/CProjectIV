#pragma once
#ifndef BLOB_HPP
#define BLOB_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <sstream>
#include <type_traits>


using namespace std;

template<typename T, typename Enable = typename enable_if<
        is_same<T, char>::value ||
        is_same<T, int>::value ||
        is_same<T, long>::value ||
        is_same<T, unsigned>::value ||
        is_same<T, float>::value ||
        is_same<T, double>::value>::type>
class MemoryPool{
public:
    using value_type = T;

    MemoryPool() : pool_(), max_memory_block_size(1024 * 1024) {}  // 初始化列表

    shared_ptr<vector<T>> request_memory(size_t size) {
        // 如果请求的内存大小小于阈值，则尝试从内存池中分配内存
        if (size <= max_memory_block_size) {
            for (auto& entry : pool_) {
                if (entry.second) {
                    entry.second = false;
                    return entry.first;
                }
            }
        }

        // 否则，直接分配新内存
        void* raw_memory = std::aligned_alloc(alignof(T), size * sizeof(T));
        if (raw_memory == nullptr) {
            throw std::bad_alloc();
        }

        auto new_memory = std::shared_ptr<std::vector<T>>(
            new(raw_memory) std::vector<T>(size),
            [this](std::vector<T>* ptr) {
                ptr->clear();
                if (ptr->capacity() <= max_memory_block_size) {
                    this->release_memory(ptr); // 如果内存块大小小于阈值，则归还给内存池
                } else {
                    // 如果内存块大小大于阈值，则直接释放内存
                    ptr->~vector<T>(); // 显示调用vector的析构函数
                    std::free(ptr);
                }
            }
        );

        if (size <= max_memory_block_size) {
            pool_.push_back({new_memory, false});
        }
        return new_memory;
    }

    void release_memory(std::vector<T>* memory) {
        for (auto& entry : pool_) {
            if (entry.first.get() == memory) {
                entry.second = true;
                break;
            }
        }
    }

private:
    vector<pair<shared_ptr<vector<T>>, bool>> pool_;
    const int max_memory_block_size;
};



template<typename T, typename Enable = typename enable_if<
        is_same<T, char>::value ||
        is_same<T, int>::value ||
        is_same<T, long>::value ||
        is_same<T, unsigned>::value ||
        is_same<T, float>::value ||
        is_same<T, double>::value>::type>
class Blob {
public:
    using value_type = T;
    // 构造函数和析构函数
    Blob() : rows_(0), cols_(0), channels_(0), data_(nullptr) {}
    Blob(size_t rows, size_t cols, size_t channels) : rows_(rows), cols_(cols), channels_(channels) {}
    Blob(const Blob &other) : rows_(other.rows_), cols_(other.cols_), channels_(other.channels_), data_(other.data_) {init_steps();} //拷贝了指针地址值

    Blob clone() const{
        Blob<T> new_blob(rows_, cols_, channels_);
        new_blob.data_ = memory_pool_.request_memory(rows_ * cols_ * channels_);
        copy(data_->begin(), data_->end(), new_blob.data_->begin());
        return new_blob;
    }
    Blob moveFrom(Blob&& other){
        if (this != &other) {
            rows_ = other.rows_;
            cols_ = other.cols_;
            channels_ = other.channels_;
            data_ = move(other.data_);
            init_steps();
            other.rows_ = 0;
            other.cols_ = 0;
            other.channels_ = 0;
        }
        return *this;
    }

    //重载
    Blob& operator=(const Blob &other){
        if (this != &other) {
            rows_ = other.rows_;
            cols_ = other.cols_;
            channels_ = other.channels_;
            data_ = other.data_;
            init_steps();
        }
        return *this;
    }

    Blob operator+(const Blob &other) const{
        check(other);
        Blob<T> result(rows_, cols_, channels_);
        result.allocate_if_needed();
        T* data_ptr = data_->data();
        T* other_data_ptr = other.data_->data();
        T* result_data_ptr = result.data_->data();
        for (size_t i = 0; i < data_->size(); ++i) {
            *result_data_ptr = *data_ptr + *other_data_ptr;
            ++data_ptr;
            ++other_data_ptr;
            ++result_data_ptr;
        }
        return result;
    }

    Blob operator-(const Blob &other) const{
        check(other);
        Blob<T> result(rows_, cols_, channels_);
        T* data_ptr = data_->data();
        T* other_data_ptr = other.data_->data();
        T* result_data_ptr = result.data_->data();
        for (size_t i = 0; i < data_->size(); ++i) {
            *result_data_ptr = *data_ptr - *other_data_ptr;
            ++data_ptr;
            ++other_data_ptr;
            ++result_data_ptr;
        }
        return result;
    }

    Blob operator*(const Blob &other) const{
        check(other);
        Blob<T> result(rows_, cols_, channels_);
        result.allocate_if_needed();
        T* data_ptr = data_->data();
        T* other_data_ptr = other.data_->data();
        T* result_data_ptr = result.data_->data();
        for (size_t i = 0; i < data_->size(); ++i) {
            *result_data_ptr = *data_ptr * *other_data_ptr;
            ++data_ptr;
            ++other_data_ptr;
            ++result_data_ptr;
        }
        return result;
    }
    
    Blob operator*(const T &scalar) const{
        check();
        Blob<T> result(rows_, cols_, channels_);
        result.allocate_if_needed();
        T* data_ptr = data_->data();
        T* result_data_ptr = result.data_->data();
        for (size_t i = 0; i < data_->size(); ++i) {
            *result_data_ptr = *data_ptr * scalar;
            ++data_ptr;
            ++result_data_ptr;
        }
        return result;
    }

    Blob operator/(const Blob &other) const{
        check(other);
        Blob<T> result(rows_, cols_, channels_);
        result.allocate_if_needed();
        T* data_ptr = data_->data();
        T* other_data_ptr = other.data_->data();
        T* result_data_ptr = result.data_->data();
        for (size_t i = 0; i < data_->size(); ++i) {
            if (*other_data_ptr == 0) {
                ostringstream os;
                os << "Error at " << __FILE__ << ":" << __LINE__ << "\n";
                os << "Division by zero";
                throw runtime_error(os.str());
            }
            *result_data_ptr = *data_ptr / *other_data_ptr;
            ++data_ptr;
            ++other_data_ptr;
            ++result_data_ptr;
        }
        return result;
    }

    Blob operator/(const T &scalar) const{
        check();
        if (scalar == 0) {
            ostringstream os;
            os << "Error at " << __FILE__ << ":" << __LINE__ << "\n";
            os << "Division by zero";
            throw runtime_error(os.str());
        }
        Blob<T> result(rows_, cols_, channels_);
        result.allocate_if_needed();
        T* data_ptr = data_->data();
        T* result_data_ptr = result.data_->data();
        for (size_t i = 0; i < data_->size(); ++i) {
            *result_data_ptr = *data_ptr / scalar;
            ++data_ptr;
            ++result_data_ptr;
        }
        return result;
    }

    void printBlob(){
        check();
        for (int k = 0; k < channels_; ++k) {
            std::cout << "Channel " << k << ":" << std::endl;
            for (int i = 0; i < rows_; ++i) {
                for (int j = 0; j < cols_; ++j) {
                    std::cout << at(i, j, k) << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    // 元素访问和修改
    T& at(size_t row, size_t col, size_t channel){
        check();
        if(row > rows_ || col > cols_ || channel > channels_){
            std::ostringstream os;
            os << "Error at " << __FILE__ << ":" << __LINE__ << "\n";
            os << "Index out of range: row(" << row << "), col(" << col << "), channel(" << channel << ")\n";
            os << "Valid range: row(0-" << rows_ << "), col(0-" << cols_ << "), channel(0-" << channels_ << ")";
            throw std::runtime_error(os.str());
        }
        return (*data_)[index(row, col, channel)];
    }

    const T& at(size_t row, size_t col, size_t channel) const{
        check();
        if(row > rows_ || col > cols_ || channel > channels_){
            std::ostringstream os;
            os << "Error at " << __FILE__ << ":" << __LINE__ << "\n";
            os << "Index out of range: row(" << row << "), col(" << col << "), channel(" << channel << ")\n";
            os << "Valid range: row(0-" << rows_ << "), col(0-" << cols_ << "), channel(0-" << channels_ << ")";
            throw std::runtime_error(os.str());
        }
        return (*data_)[index(row, col, channel)];
}
    

    // 迭代器访问
    typename std::vector<T>::iterator begin(){
        check();
        return data_->begin();
    }

    typename std::vector<T>::const_iterator begin() const{
        check();
        return data_->cbegin();
    }

    typename std::vector<T>::iterator end(){
        check();
        return data_->end();
    }

    typename std::vector<T>::const_iterator end() const{
        check();
        return data_->cend();
    }

    // ptr访问方式
    T* ptr(size_t row, size_t col, size_t channel){
        check();
        if(row > rows_ || col > cols_ || channel > channels_){
            std::ostringstream os;
            os << "Error at " << __FILE__ << ":" << __LINE__ << "\n";
            os << "Index out of range: row(" << row << "), col(" << col << "), channel(" << channel << ")\n";
            os << "Valid range: row(0-" << rows_ << "), col(0-" << cols_ << "), channel(0-" << channels_ << ")";
            throw std::runtime_error(os.str());
        }
        return reinterpret_cast<T*>(reinterpret_cast<char*>(data_->data()) + row * steps_[0] + col * steps_[1] + channel * steps_[2]);
    }


    const T* ptr(size_t row, size_t col, size_t channel) const{
        check();
        if(row > rows_ || col > cols_ || channel > channels_){
            std::ostringstream os;
            os << "Error at " << __FILE__ << ":" << __LINE__ << "\n";
            os << "Index out of range: row(" << row << "), col(" << col << "), channel(" << channel << ")\n";
            os << "Valid range: row(0-" << rows_ << "), col(0-" << cols_ << "), channel(0-" << channels_ << ")";
            throw std::runtime_error(os.str());
        }
        return reinterpret_cast<const T*>(reinterpret_cast<const char*>(data_->data()) + row * steps_[0] + col * steps_[1] + channel * steps_[2]);
    }

    // ROI裁剪
    Blob<T> roi(size_t start_row, size_t start_col, size_t end_row, size_t end_col, size_t channel) const{
         check();
        if(start_row > end_row || start_col > end_col || start_row > rows_ || start_col > cols_ || end_row > rows_ || end_col > cols_ || channel > channels_ ){
            std::ostringstream os;
            os << "Error at " << __FILE__ << ":" << __LINE__ << "\n";
            os << "Invalid range: row(" << start_row << "-" << end_row << "), col(" << start_col << "-" << end_col << "), channel(" << channel << ")";
            os << "Valid range: row(0-" << rows_ << "), col(0-" << cols_ << "), channel(0-" << channels_ << ")";
            throw std::runtime_error(os.str());
        }
        size_t new_rows = end_row - start_row;
        size_t new_cols = end_col - start_col;
        Blob<T> new_blob(new_rows, new_cols, 1);
        new_blob.allocate_if_needed();
        for (int i = 0; i < new_rows; ++i) {
            for (int j = 0; j < new_cols; ++j) {
                *new_blob.ptr(i, j, 0) = *ptr(start_row + i, start_col + j, channel);
            }
        }

        return new_blob;
    }

    // 矩阵变换
    Blob<T> transposeWithReturn() const{
        check();
        if (rows_ != cols_) {
            throw std::runtime_error("In-place transpose is only possible for square matrices.");
        }
        Blob<T> new_blob(cols_, rows_, channels_);
        new_blob.allocate_if_needed();
        for (int k = 0; k < channels_; ++k) {
            for (int i = 0; i < rows_; ++i) {
                for (int j = 0; j < cols_; ++j) {
                    *new_blob.ptr(i, j, k) = *ptr(j, i, k);
                }
            }
        }

        return new_blob;
    }

    void transpose(){
        check();
        if (rows_ != cols_) {
            throw std::runtime_error("In-place transpose is only possible for square matrices.");
        }
        for (int k = 0; k < channels_; ++k) {
            for (int i = 0; i < rows_; ++i) {
                for (int j = i + 1; j < cols_; ++j) {
                    std::swap(*ptr(i, j, k), *ptr(j, i, k));
                }
            }
        }
    }

    // 翻转
    enum FlipType { HORIZONTAL, VERTICAL, BOTH };

    void flip(FlipType flip_type){
        check();
        for (int k = 0; k < channels_; ++k) {
            if (flip_type == HORIZONTAL || flip_type == BOTH) {
                for (int i = 0; i < rows_; ++i) {
                    for (int j = 0; j < cols_ / 2; ++j) {
                        std::swap(*ptr(i, j, k), *ptr(i, cols_ - 1 - j, k));
                    }
                }
            }
            if (flip_type == VERTICAL || flip_type == BOTH) {
                for (int i = 0; i < rows_ / 2; ++i) {
                    for (int j = 0; j < cols_; ++j) {
                        std::swap(*ptr(i, j, k), *ptr(rows_ - 1 - i, j, k));
                    }
                }
            }
        }
    }

    Blob<T> flipWithReturn(FlipType flip_type) const{
        check();
        Blob<T> new_blob(rows_, cols_, channels_);
        new_blob.allocate_if_needed();
        for (int k = 0; k < channels_; ++k) {
            for (int i = 0; i < rows_; ++i) {
                for (int j = 0; j < cols_; ++j) {
                    int new_i = i, new_j = j;

                    if (flip_type == HORIZONTAL || flip_type == BOTH) {
                        new_j = cols_ - 1 - j;
                    }
                    if (flip_type == VERTICAL || flip_type == BOTH) {
                        new_i = rows_ - 1 - i;
                    }

                    *new_blob.ptr(new_j, new_i, k) = *ptr(j, i, k);
                }
            }
        }

        return new_blob;
    }

    // 访问器
    size_t rows() const{
        return rows_;
    }
    size_t cols() const{
        return cols_;
    }
    size_t channels() const{
        return channels_;
    }

    //获得data_
    T* data(){
        return data_;
    }

    Blob<T> subBlob(int start_row, int start_col, int end_row, int end_col){
        // 计算新的行数和列数
        int new_rows = end_row - start_row;
        int new_cols = end_col - start_col;

        // 计算新子矩阵视图的起始指针位置
        check();
        T* subBlob_data = data_ + (start_row * cols_ + start_col);

        // 创建一个新的Mat对象，它共享原始数据，但具有新的大小和偏移量
        Blob<T> subBlob(new_rows, new_cols, subBlob_data);
        
        return subBlob;
    }

    void allocate_if_needed() {
         if (!data_) {
            try {
                data_ = memory_pool_.request_memory(rows_ * cols_ * channels_);
            } catch (std::bad_alloc& e) {
                std::cerr << "Error: Failed to allocate memory for Blob." << std::endl;
                // 抛出异常或返回错误码等
            }
            init_steps();
        }
    }

private:
    // 成员变量
    size_t rows_;
    size_t cols_;
    size_t channels_;
    vector<size_t> steps_; 
    mutable shared_ptr<vector<T>> data_;

    static MemoryPool<T> memory_pool_;

    // 辅助函数
    size_t index(size_t row, size_t col, size_t channel) const{
        return rows_ * cols_ * channel + rows_ * col + row;
    }

    void check(const Blob &other) const{
        if (rows_ != other.rows_ || cols_ != other.cols_ || channels_ != other.channels_) {
            throw std::runtime_error("Incompatible Blob dimensions for element-wise operation");
        }
        //同时检查Blob的data_是否为空
        if (!data_ || !other.data_) {
            throw std::runtime_error("Blob data is empty");
        }
    }

    void check() const{
         if (!data_) {
            throw std::runtime_error("Blob data is empty");
        }
    }


    void init_steps(){
        steps_.resize(3);
        steps_[0] = cols_ * channels_ * sizeof(T);
        steps_[1] = channels_ * sizeof(T);
        steps_[2] = sizeof(T);
    }
};

template<typename T, typename Enable>
MemoryPool<T> Blob<T, Enable>::memory_pool_;

#endif /* BLOB_HPP */