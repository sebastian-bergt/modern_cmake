#ifndef LOCKFREE_RINGBUFFER_H
#define LOCKFREE_RINGBUFFER_H

#include <memory>
#include <atomic>
#include <cstring>
#include <bitset>
#include <type_traits>

#include "lockfree_ringbuffer/i_lockfree_ringbuffer.h"

namespace lockfree_ringbuffer
{
    namespace internal
    {
        constexpr bool is_power_of_2(std::size_t n)
        {
            if (n == 0)
            {
                return false;
            }
            else
            {
                return ((n & (n - 1)) == 0); // https://stackoverflow.com/questions/108318/whats-the-simplest-way-to-test-whether-a-number-is-a-power-of-2-in-c
            }
        }
    } // namespace internal

    template <typename T, std::size_t S>
    class RingBuffer : IRingBuffer<T>
    {
    public:
        RingBuffer();

        ID addReader() override;
        ID addWriter() override;

        STATUS tryWrite(ID id, T t) override;
        STATUS tryReadNext(ID id, T &t) override;
        STATUS tryReadNewest(ID id, T &t) override;
        void clean() override;

        static_assert(internal::is_power_of_2(S), "RingBuffer only supports powers of two, e.g. 2, 4, 8, ...");
        static_assert(std::is_copy_assignable<T>::value,
                      "RingBuffer requires copy assigning");

    private:
        template <typename CONTENT>
        struct Element
        {
            std::atomic<bool> fully_written;
            CONTENT content;
        };

        std::unique_ptr<Element<T>[]> buffer_;
        std::size_t buffer_size_;
        std::atomic<std::size_t> writer_position_; // is expected to overflow
        std::atomic<std::size_t> reader_position_; // is expected to overflow
    };

} // namespace lockfree_ringbuffer

#include "lockfree_ringbuffer/lockfree_ringbuffer.ipp"

#endif // LOCKFREE_RINGBUFFER_H
