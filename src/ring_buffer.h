#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ring_buffer {
    uint32_t _len;
    uint32_t _maxlen;
    uint8_t *_writer;
    uint8_t *_reader;
    uint8_t *_buffer;
};

#define BUFFER_STATIC_INIT(BUFF,LEN) {0, LEN, BUFF, BUFF, BUFF}

void init_static_ring_buffer(struct ring_buffer *buf, uint8_t *buffer, uint32_t maxlen);
void init_buffer(struct ring_buffer *buf, uint32_t maxlen);
void free_buffer(struct ring_buffer *buf);
int fifo_push(struct ring_buffer *buf, uint8_t data);
int fifo_pop(struct ring_buffer *buf, uint8_t *data);
uint32_t buffer_length(struct ring_buffer *buf);

#ifdef __cplusplus
}
#endif
