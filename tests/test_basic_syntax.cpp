//
// Created by Ramon Smits on 26/07/2025.
//
#include <gtest/gtest.h>
#include <cstdlib>


TEST(CBASICS, Malloc0) {
    size_t intSize = 4;
    int* addressPtr = static_cast<int*>(malloc(intSize));
    *addressPtr = 3;
    std::cout << "Found value: " << std::to_string(*addressPtr) << std::endl;
    free(addressPtr);
}


TEST(CBASICS, Calloc0) {
    size_t intSize = 4;
    size_t count = 10;
    void* memPtr =  calloc(count, intSize);

    int* numbers = static_cast<int*>(memPtr);
    std::cout << "Starting ptr: " << numbers << std::endl;
    int* startPtr = numbers;

    for (int curr_count = 0; curr_count < count; curr_count++) {
        std::cout << "Inserting number: " << std::to_string(curr_count) << std::endl;
        *numbers = curr_count;
        std::cout << std::to_string(*numbers) << std::endl;
        numbers++;
    }

    std::cout << "Loop: " << startPtr << std::endl;
    for (int curr_count = 0; curr_count < count; curr_count++) {
        std::cout << "Number found: " << std::to_string(*static_cast<int *>(startPtr)) << std::endl;
        startPtr++;
    }

    free(memPtr);
}


TEST(CBASICS, Malloc1) {
    size_t intSize = 4;
    size_t count = 10;
    void* memPtr =  malloc(intSize * 10);
    int* numbers = static_cast<int*>(memPtr);

    int* startPtr = numbers;
    int* vecPtr = numbers;

    for (int curr_count = 0; curr_count < count; curr_count++) {
        std::cout << "Inserting number: " << std::to_string(curr_count) << std::endl;
        *numbers = curr_count;
        numbers++;
    }

    std::cout << "Loop: " << startPtr << std::endl;
    for (int curr_count = 0; curr_count < count; curr_count++) {
        std::cout << "Number found: " << std::to_string(*startPtr) << ", at: " << startPtr << std::endl;
        startPtr++;
    }

    for (int i = 0; i < count; i++) {
        std::cout << "Number found: " << std::to_string(vecPtr[i]) << std::endl;
    }

    free(memPtr);
}


TEST(CPPBASICS, Vector0) {
    std::vector<int> someVector;
    someVector.push_back(1);
    someVector.emplace_back(1);
}

TEST(CPPBASICS, ConstExpr0) {
    constexpr int someExpr = 2;
    std::cout << std::to_string(someExpr) << std::endl;
}


int someFunctionLRValues(const int& myNumber) {
    std::cout << "My number: " << std::to_string(myNumber) << std::endl;
    return myNumber;
}


int someFunctionLValue(int& myNumber) {
    std::cout << "My number: " << std::to_string(myNumber) << std::endl;
    return myNumber;
}

int someFunctionRValue(int&& myNumber) {
    std::cout << "My number: " << std::to_string(myNumber) << std::endl;
    return myNumber;
}


TEST(CBASICS, RLValue0) {
    int lValue = 2;
    someFunctionLRValues(lValue);
    someFunctionLRValues(3);
}

TEST(CBASICS, RLValue1) {
    int lValue = 2;
    someFunctionLValue(lValue);
}

TEST(CBASICS, RLValue2) {
    someFunctionRValue(2);
}

TEST(CBASICS, STDForward0) {
    int lValue = 2;
    int&& outcome = std::move(lValue);
    std::cout << outcome << std::endl;
}


template<class T>
void someOtherFunction(T arg) {

}

template<class T>
void someFunction(T&& arg) {
    someOtherFunction(std::forward<T>(arg));
}


template<class T>
    struct TestStruct2 {

    T some;
    T data;
    T inserts;

    TestStruct2(std::initializer_list<T> args) {
        size_t stride = sizeof(T);
        const T* beginPtr = args.begin();
        some = *beginPtr;
        ++beginPtr;
        data = *beginPtr;
        ++beginPtr;
        inserts = *beginPtr;
    }
};




TEST(CPPBASICS, STDInitializerList0) {

    struct TestStruct {
        std::vector<int> data;
        TestStruct(std::initializer_list<int> args) {
            data.insert(data.end(), args.begin(), args.end());
        }
    };

    TestStruct t {1, 2, 3, 4};
    ASSERT_EQ(t.data.size(), 4);

    TestStruct2 t2 {1, 2, 3};

    ASSERT_EQ(t2.some, 1);
    ASSERT_EQ(t2.data, 2);
    ASSERT_EQ(t2.inserts, 3);

}


template<class ...T>
struct TestStruct3 {

    // T some;
    // T data;
    // T here;

    explicit TestStruct3(T... args) {
        // some = args...[0];
        // data = args...[1];
        // here = args...[2];
    }

};


TEST(CPPBASICS, PackExpansion0) {

    // TestStruct3<int, int, int> t3 {1, 2, 3};
    // ASSERT_EQ(t3.some, 1);
    // ASSERT_EQ(t3.data, 2);
    // ASSERT_EQ(t3.here, 3);

}


TEST(CPPBASICS, unique_ptr) {

    struct Data {
        int someData;
        Data() : someData(1) {};
    };

    std::unique_ptr<Data> data = std::make_unique<Data>();
}



TEST(CPPBASICS, allocator_test) {

    std::allocator<int> int_alloc;

    std::allocator_traits<std::allocator<int>> int_allocator_traits;

    int_allocator_traits.max_size(int_alloc);

    std::vector<int> someVec;

    int x = sizeof(someVec);








    // std::cout << size << std::endl;
}


template <typename T>
void generic_c_out(T&& printable) {
    std::cout << std::forward<T>(printable) << std::endl;
}


TEST(CPPBASICS, SharedPtrTest) {
    int y = 5;
    std::shared_ptr<int> y_ptr = std::make_shared<int>(y);
    std::shared_ptr<int> y_ptr2 = y_ptr;
    generic_c_out(y_ptr.use_count());
    generic_c_out(y_ptr2.use_count());
}


