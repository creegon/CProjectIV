#include "blob.hpp"


int main() {
    // 测试构造函数
    Blob<int> b1(2, 2, 1);
    b1.allocate_if_needed();
    std::cout << "Rows: " << b1.rows() << ", Cols: " << b1.cols() << ", Channels: " << b1.channels() << std::endl;

    // 测试元素访问和修改
    b1.at(0, 0, 0) = 1;
    b1.at(0, 1, 0) = 2;
    b1.at(1, 0, 0) = 3;
    b1.at(1, 1, 0) = 4;
    std::cout << "Elements of b1: " << std::endl;
    for (size_t i = 0; i < b1.rows(); ++i) {
        for (size_t j = 0; j < b1.cols(); ++j) {
            std::cout << b1.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    // 测试拷贝构造函数（软拷贝）
    Blob<int> b2(b1);
    b2.allocate_if_needed();
    std::cout << "Elements of b2 (copy of b1): " << std::endl;
    for (size_t i = 0; i < b2.rows(); ++i) {
        for (size_t j = 0; j < b2.cols(); ++j) {
            std::cout << b2.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    // 测试硬拷贝
    Blob<int> b3 = b1.clone();
    b3.allocate_if_needed();
    std::cout << "Elements of b3 (clone of b1): " << std::endl;
    for (size_t i = 0; i < b3.rows(); ++i) {
        for (size_t j = 0; j < b3.cols(); ++j) {
            std::cout << b3.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    //测试两个拷贝是否真的是硬拷贝和软拷贝（修改b1的值）
    b1.at(0, 0, 0) = 5;
    b1.at(1, 1, 0) = 5;
    std::cout << "Elements of b2 (copy of b1 after changing): " << std::endl;
    for (size_t i = 0; i < b2.rows(); ++i) {
        for (size_t j = 0; j < b2.cols(); ++j) {
            std::cout << b2.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Elements of b3 (clone of b1 after changing): " << std::endl;
    for (size_t i = 0; i < b3.rows(); ++i) {
        for (size_t j = 0; j < b3.cols(); ++j) {
            std::cout << b3.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    // 测试迭代器访问
    std::cout << "Elements of b1 (using iterators): " << std::endl;
    for (auto it = b1.begin(); it != b1.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // 测试ptr访问方式
    std::cout << "Elements of b1 (using ptr): " << std::endl;
    for (size_t i = 0; i < b1.rows(); ++i) {
        for (size_t j = 0; j < b1.cols(); ++j) {
            std::cout << *b1.ptr(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    // 测试运算符重载
    Blob<int> b4 = b1 + b2;
    b4.allocate_if_needed();
    std::cout << "Elements of b4 (b1 + b2): " << std::endl;
    for (size_t i = 0; i < b4.rows(); ++i) {
        for (size_t j = 0; j < b4.cols(); ++j) {
            std::cout << b4.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    //测试乘法（非标量）
    Blob<int> bMul = b1 * b2;
    bMul.allocate_if_needed();
    std::cout << "Elements of bMul (b1 * b2): " << std::endl;
    for (size_t i = 0; i < bMul.rows(); ++i) {
        for (size_t j = 0; j < bMul.cols(); ++j) {
            std::cout << bMul.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    //测试乘法（标量）
    Blob<int> bMulScalar = b1 * 2;
    bMulScalar.allocate_if_needed();
    std::cout << "Elements of bMulScalar (b1 * 2): " << std::endl;
    for (size_t i = 0; i < bMulScalar.rows(); ++i) {
        for (size_t j = 0; j < bMulScalar.cols(); ++j) {
            std::cout << bMulScalar.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    // 测试翻转变换
    Blob<int> b5 = b1.flipWithReturn(Blob<int>::HORIZONTAL);
    b5.allocate_if_needed();
    std::cout << "Elements of b5 (horizontal flip of b1): " << std::endl;
    for (size_t i = 0; i < b5.rows(); ++i) {
        for (size_t j = 0; j < b5.cols(); ++j) {
            std::cout << b5.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    Blob<int> b6 = b1.flipWithReturn(Blob<int>::VERTICAL);
    b6.allocate_if_needed();
    std::cout << "Elements of b6 (vertical flip of b1): " << std::endl;
    for (size_t i = 0; i < b6.rows(); ++i) {
        for (size_t j = 0; j < b6.cols(); ++j) {
            std::cout << b6.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    Blob<int> b7 = b1.flipWithReturn(Blob<int>::BOTH);
    b7.allocate_if_needed();
    std::cout << "Elements of b7 (both flips of b1): " << std::endl;
    for (size_t i = 0; i < b7.rows(); ++i) {
        for (size_t j = 0; j < b7.cols(); ++j) {
            std::cout << b7.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    // 测试ROI裁剪
    Blob<int> b8 = b1.roi(0, 0, 1, 1, 0);
    b8.allocate_if_needed();
    std::cout << "Elements of b9 (ROI of b1): " << std::endl;
    for (size_t i = 0; i < b8.rows(); ++i) {
        for (size_t j = 0; j < b8.cols(); ++j) {
            std::cout << b8.at(i, j, 0) << " ";
        }
        std::cout << std::endl;
    }

    
    //测试是否正常释放
    Blob<int> b11(2, 2, 1);

    {
        Blob<int> b12(2, 2, 1);
        b12.allocate_if_needed();
        //给b12赋值
        for (size_t i = 0; i < b12.rows(); ++i) {
            for (size_t j = 0; j < b12.cols(); ++j) {
                b12.at(i, j, 0) = i * b12.cols() + j;
            }
        }
        // 将 b1 的指针指向 b2 
        b11 = b12;
    } // 小作用域结束，b2 被销毁，但是 b1 的指针仍然指向 b2 的 data

    // 测试 b1 的指针是否正常工作
    std::cout << "Elements of b11 (testing releasing):" << std::endl;
    for (size_t i = 0; i < b11.rows(); ++i) {
        for (size_t j = 0; j < b11.cols(); ++j) {
            std::cout << *(b11.ptr(i, j, 0)) << " ";
        }
        std::cout << std::endl;
    }
    
    //测试报错（比如除以0）
    Blob<int> bDiv(2, 2, 1);
    bDiv.allocate_if_needed();
    //给bDiv赋值
    for (size_t i = 0; i < bDiv.rows(); ++i) {
        for (size_t j = 0; j < bDiv.cols(); ++j) {
            bDiv.at(i, j, 0) = i * bDiv.cols() + j;
        }
    }
    //测试除法
    Blob<int> bDivResult = bDiv / 0;
    
    

    return 0;
}
