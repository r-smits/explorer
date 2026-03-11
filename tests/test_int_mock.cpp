//
// Created by Ramon Smits on 28/07/2025.
//
#include <gtest/gtest.h>
#include <stdexcept>

template<class K, class V>
struct Element {

    Element(const K& key, const V& value, const bool& occupied = false) : K(key), V(value), occupied(occupied){}

    K key;
    V value;
    bool occupied;



};



template<class K, class V>
class OpenAddressingHashTable {

public:
    explicit OpenAddressingHashTable(const size_t& initialCapacity = 10) {
        capacity = initialCapacity;
        currentSize = 0;
        startIndex = malloc(capacity * sizeof(int));
        endIndex = startIndex += 10;
        currIndex = startIndex;
    }

    V get(const K& key) {
        return 0;
    }

    void insert(const K& key, const V& value) {

    }

    size_t get_size() const {
        return currentSize;
    }

    size_t get_capacity() const {
        return capacity;
    }

private:
    std::vector<V> data;
    size_t capacity;
    size_t currentSize;
    void* startIndex;
    void* endIndex;
    void* currIndex;

    size_t hash(const K& key) const {
        return std::hash<K>{}(key) % capacity;
    }

    void resize() {

        if (capacity > 0xffffffff / 2) {
            throw std::logic_error("Too many elements in OpenAddressingHashTable");
        }
        capacity *= 2;
        std::vector<V> oldData = data;
        data.resize(capacity);


    }

    std::unordered_map<K, V> map;


};

TEST(MOCK, Test0) {
    // We need to track a large number of items for insertion via a key
    // We want the performance to be good
    // We want to minimize the performance variance so as to have a more stable performance metric to plan on
    // We use a traditional hash map for the task
    // We've found it is too high in latency has too high a variance
    // A developer mentions: open addressing hash-table
    // Implement one to compare performance characteristics

    // Open-addressing hash-table has an overly sized internal array: same as collision-list based hash-table
    // Instead of using a hash to find a collision list and inserting element into list
    // Item goes directly into hashed location within internal array
    // If an element is already there, we place the new element in first free location by stepping down incrementally
    // Collision lists exist as consecutive filled elements in the array

    std::cout << "Hello world!" << std::endl;
}


