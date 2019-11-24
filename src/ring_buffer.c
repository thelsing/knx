#include <stdlib.h>
#include <stdint.h>
#include "ring_buffer.h"

void init_static_ring_buffer(struct ring_buffer *buf, uint8_t *buffer, uint32_t maxlen)
{
        buf->_buffer = buffer;
        buf->_writer = buf->_buffer;
        buf->_reader = buf->_buffer;
        buf->_len = 0;
        buf->_maxlen = maxlen;
}

int fifo_push(struct ring_buffer *buf, uint8_t data)
{
        /* FIFO FULL */
        if(buf->_len >= buf->_maxlen)
                return -1;

        *buf->_writer = data;
        ++buf->_len;
        ++buf->_writer;

        /* If end of buffer, restart to begin */
        if(buf->_writer > (buf->_buffer + buf->_maxlen))
        {
                buf->_writer = buf->_buffer;
        }
        return 1;
}

int fifo_pop(struct ring_buffer *buf, uint8_t *data)
{
        /* FIFO EMPTY */
        if(buf->_len == 0)
                return -1;

        *data = *buf->_reader;
        ++buf->_reader;
        --buf->_len;

        /* If end of buffer, restart to begin */
        if(buf->_reader > (buf->_buffer + buf->_maxlen))
        {
                buf->_reader = buf->_buffer;
        }
        return 1;
}

uint32_t buffer_length(struct ring_buffer *buf)
{
        return buf->_len;
}


