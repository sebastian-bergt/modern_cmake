#ifndef LOCKFREE_RINGBUFFER_H
#define LOCKFREE_RINGBUFFER_H

#include <memory>
#include <vector>
#include <cstring>
#include "lockfree_ringbuffer/i_lockfree_ringbuffer.h"

namespace lockfree_ringbuffer
{
    template <typename T, std::size_t S>
    class RingBuffer : IRingBuffer<T>
    {
    public:
        RingBuffer();

        ID addConsumer() override;
        ID addProducer() override;

        STATUS write(ID id, T t) override;
        STATUS readNext(ID id, T &t) override;
        STATUS readNewest(ID id, T &t) override;
        void clean() override;

    private:
        std::unique_ptr<T[]> buffer_;
        std::size_t buffer_size_;
        std::vector<std::size_t> consumer_positions_;
        std::vector<std::size_t> producer_positions_;
        std::size_t last_producer_position_;
    };

} // namespace lockfree_ringbuffer

#include "lockfree_ringbuffer/lockfree_ringbuffer.ipp"

#endif // LOCKFREE_RINGBUFFER_H
