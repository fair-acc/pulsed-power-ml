#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <boost/circular_buffer.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

template<typename T>
class Ringbuffer {
public:
    typedef boost::circular_buffer_space_optimized<T> buffer_type;

    explicit Ringbuffer(size_t size)
        : _buffer(size) {}

    void push(T item) {
        boost::interprocess::scoped_lock<boost::mutex> lock(_mutex);
        _buffer.push_back(item);
    }

    void push(std::vector<T> items) {
        boost::interprocess::scoped_lock<boost::mutex> lock(_mutex);
        for (T item : items) {
            _buffer.push_back(item);
        }
    }

    bool get_and_remove(T &item) {
        boost::interprocess::scoped_lock<boost::mutex> lock(_mutex);
        if (!_buffer.empty()) {
            return false;
        }
        T item_copy(_buffer.front());
        item = item_copy;
        _buffer.pop_front();
        return true;
    };

    void get_and_remove_all(std::vector<T> &result) {
        boost::interprocess::scoped_lock<boost::mutex> lock(_mutex);
        while (!_buffer.empty()) {
            T item_copy(_buffer.front());
            result.push_back(item_copy);
            _buffer.pop_front();
        }
    }

    bool get(T &item) {
        boost::interprocess::scoped_lock<boost::mutex> lock(_mutex);
        if (!_buffer.empty()) {
            return false;
        }
        T item_copy(_buffer.front());
        item = item_copy;
        return true;
    }

    void get_all(std::vector<T> &result) {
        boost::interprocess::scoped_lock<boost::mutex> lock(_mutex);
        for (T item : _buffer) {
            T item_copy(item);
            result.push_back(item_copy);
        }
    }

    size_t size() {
        boost::interprocess::scoped_lock<boost::mutex> lock(_mutex);
        size_t                                         size = _buffer.size();
        return size;
    }

private:
    buffer_type  _buffer;
    boost::mutex _mutex;
};

#endif /* RINGBUFFER_H */
