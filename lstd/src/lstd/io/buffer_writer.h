#pragma once

#include "../memory/stack_dynamic_buffer.h"
#include "writer.h"

LSTD_BEGIN_NAMESPACE

template <s64 N>
struct buffer_writer : writer {
    stack_dynamic_buffer<N> *StackDynamicBuffer;

    byte *Buffer, *Current;
    s64 BufferSize, Available;

    buffer_writer(stack_dynamic_buffer<N> *buffer)
        : StackDynamicBuffer(buffer) {
        Buffer     = Current   = buffer->Data;
        BufferSize = Available = buffer->Allocated ? buffer->Allocated : sizeof buffer->StackData;
    }

    void write(const byte *data, s64 size) override {
        if (size > Available) {
            write(data, Available);
            data += Available;
            size -= Available;

            flush();
        }

        copy_memory(Current, data, size);
        Current += size;
        Available -= size;
    }

    void flush() override {
        s64 count = BufferSize - Available;
        reserve(*StackDynamicBuffer, StackDynamicBuffer->Count + count);
        StackDynamicBuffer->Count += count;

        Buffer = Current = StackDynamicBuffer->Data + StackDynamicBuffer->Count;

        if (StackDynamicBuffer->Allocated) {
            Available = StackDynamicBuffer->Reserved - StackDynamicBuffer->Count;
        } else {
            Available = sizeof StackDynamicBuffer->StackData - StackDynamicBuffer->Count;
        }
        BufferSize = Available;
    }
};

LSTD_END_NAMESPACE
