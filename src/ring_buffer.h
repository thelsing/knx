#pragma once

#ifdef __cplusplus
extern "C" {
#endif



void init_static_ring_buffer(struct ring_buffer *buf, uint8_t *buffer, uint32_t maxlen);
void init_buffer(struct ring_buffer *buf, uint32_t maxlen);
void free_buffer(struct ring_buffer *buf);
int fifo_push(struct ring_buffer *buf, uint8_t data);
int fifo_pop(struct ring_buffer *buf, uint8_t *data);
uint32_t buffer_length(struct ring_buffer *buf);

#ifdef __cplusplus
}
#endif
