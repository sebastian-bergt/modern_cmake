#ifndef I_LOCKFREE_RINGBUFFER_H
#define I_LOCKFREE_RINGBUFFER_H

#include <cstdint>

namespace lockfree_ringbuffer
{
    enum class STATUS
    {
        SUCCESS = 0,
        ERROR_READ,
        ERROR_WRITE,
        ERROR_ID
    };

    using ID = std::uint32_t;

    template <typename T>
    class IRingBuffer
    {
    public:
        virtual ~IRingBuffer(){};
        virtual ID addConsumer() = 0;
        virtual ID addProducer() = 0;

        virtual STATUS write(ID id, T t) = 0;
        virtual STATUS readNext(ID id, T &t) = 0;
        virtual STATUS readNewest(ID id, T &t) = 0;
        virtual void clean() = 0;
    };

} // namespace lockfree_ringbuffer

#endif // I_LOCKFREE_RINGBUFFER_H