#include "lockfree_ringbuffer/lockfree_ringbuffer.h"
#include <cstring>

namespace lockfree_ringbuffer
{
    template <typename T, std::size_t S>
    RingBuffer<T, S>::RingBuffer()
    {
        buffer_ = std::make_unique<T[]>(S); // heap allocated array
        buffer_size_ = S;
        clean();
    }

    template <typename T, std::size_t S>
    ID RingBuffer<T, S>::addConsumer()
    {
        const ID id = consumer_positions_.size();
        consumer_positions_.push_back(last_producer_position_);
        return id;
    }

    template <typename T, std::size_t S>
    ID RingBuffer<T, S>::addProducer()
    {
        const ID id = producer_positions_.size();
        consumer_positions_.push_back(0);
        return id;
    }

    template <typename T, std::size_t S>
    STATUS RingBuffer<T, S>::write(ID id, T t)
    {
        if (id >= producer_positions_.size())
        {
            return STATUS::ERROR_ID;
        }

        last_producer_position_++;
        std::size_t write_position = last_producer_position_ % buffer_size_; // TODO: could be further optimized with buffer sizes of only 2^N
        producer_positions_.at(id) = write_position;
        buffer_[write_position] = t;
        return STATUS::SUCCESS;
    }

    template <typename T, std::size_t S>
    STATUS RingBuffer<T, S>::readNext(ID id, T &t)
    {
        if (id >= consumer_positions_.size())
        {
            return STATUS::ERROR_ID;
        }

        consumer_positions_.at(id)++;
        std::size_t read_position = consumer_positions_.at(id) % buffer_size_;
        t = buffer_[read_position];
        if (last_producer_position_ == read_position)
        {
            return STATUS::ERROR_READ;
        }
        return STATUS::SUCCESS;
    }

    template <typename T, std::size_t S>
    STATUS RingBuffer<T, S>::readNewest(ID id, T &t)
    {
        if (id >= consumer_positions_.size())
        {
            return STATUS::ERROR_ID;
        }

        std::size_t read_position = last_producer_position_;
        consumer_positions_.at(id) = read_position;
        t = buffer_[read_position];
        return STATUS::SUCCESS;
    }

    template <typename T, std::size_t S>
    void RingBuffer<T, S>::clean()
    {
        std::memset(buffer_.get(), 0, S * sizeof(T));
        consumer_positions_.clear();
        producer_positions_.clear();
    }

} // namespace lockfree_ringbuffer