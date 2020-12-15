#ifndef LOCKFREE_RINGBUFFER_H
#define LOCKFREE_RINGBUFFER_H

#include <memory>
#include <atomic>
#include <cstring>
#include <bitset>
#include <type_traits>

#include "lockfree_ringbuffer/i_lockfree_ringbuffer.h"

namespace lockfree
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

    /**
     * RingBuffer wich allows only a single writer and a single reader
     * 
     * Make sure to pass only sizes which are power of two.
     */
    template <typename T, std::size_t S>
    class SWSRRingBuffer : IRingBuffer<T>
    {
    public:
        SWSRRingBuffer();

        ID addReader() override;
        ID addWriter() override;

        STATUS tryWrite(ID id, T t) override;
        STATUS tryRead(ID id, T &t) override;
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

    /* IMPLEMENTATIONS */

    template <typename T, std::size_t S>
    SWSRRingBuffer<T, S>::SWSRRingBuffer()
    {
        buffer_ = std::make_unique<Element<T>[]>(S); // heap allocated array
        clean();
    }

    template <typename T, std::size_t S>
    ID SWSRRingBuffer<T, S>::addReader()
    {
        return 0;
    }

    template <typename T, std::size_t S>
    ID SWSRRingBuffer<T, S>::addWriter()
    {
        return 0;
    }

    static inline std::size_t modulo_power_of2(const std::size_t &dividend, const std::size_t &divisor)
    {
        return dividend & (divisor - 1);
    }

    template <typename T, std::size_t S>
    STATUS SWSRRingBuffer<T, S>::tryWrite(ID id, T t)
    {
        std::size_t writer_index = modulo_power_of2(writer_position_ + 1, S);
        std::size_t reader_index = modulo_power_of2(reader_position_, S);

        if (writer_index == reader_index)
        {
            return STATUS::ERROR_BUFFER_FULL;
        }

        buffer_[writer_index].fully_written = false;

        writer_position_++;

        buffer_[writer_index].content = t;
        buffer_[writer_index].fully_written = true;
        return STATUS::SUCCESS;
    }

    template <typename T, std::size_t S>
    STATUS SWSRRingBuffer<T, S>::tryRead(ID id, T &t)
    {
        if (reader_position_ == writer_position_)
        {
            return STATUS::ERROR_NOTHING_TO_READ;
        }
        std::size_t writer_index = modulo_power_of2(writer_position_, S);
        std::size_t reader_index = modulo_power_of2(reader_position_ + 1, S);

        if (reader_index == writer_index && !buffer_[reader_index].fully_written)
        {
            return STATUS::ERROR_NOTHING_TO_READ;
        }

        t = buffer_[reader_index].content;
        reader_position_++;
        return STATUS::SUCCESS;
    }

    template <typename T, std::size_t S>
    STATUS SWSRRingBuffer<T, S>::tryReadNewest(ID id, T &t)
    {
    }

    //     if (reader_position_ == writer_position_)
    //     {
    //         return STATUS::ERROR_NOTHING_TO_READ;
    //     }
    //     std::size_t index = modulo_power_of2(writer_position_, S);
    //     if (buffer_[index].fully_written)
    //     {
    //         t = buffer_[index].content;
    //         std::size_t tmp = writer_position_;
    //         reader_position_ = tmp;
    //     }
    //     else
    //     {
    //         index = (writer_position_ - 1) & (S - 1);
    //         t = buffer_[index].content;
    //         std::size_t tmp = writer_position_ - 1;
    //         reader_position_ = tmp;
    //     }
    //     return STATUS::SUCCESS;
    // }

    template <typename T, std::size_t S>
    void SWSRRingBuffer<T, S>::clean()
    {

        for (std::size_t i = 0; i < S; i++)
            buffer_[i].fully_written = false;
        reader_position_ = 0;
        writer_position_ = 0;
    }

} // namespace lockfree

#endif // LOCKFREE_RINGBUFFER_H
