#include "byte_buffer.h"

GU_BEGIN_NAMESPACE

void release(Byte_Buffer &buffer) {
    if (buffer.Data) {
        Delete(buffer.Data, buffer._Reserved, buffer.Allocator);
        buffer.Data = null;
        buffer._Reserved = 0;
        buffer.Used = 0;
    }
}

Byte_Buffer::~Byte_Buffer() { release(*this); }

Byte_Buffer::Byte_Buffer(Byte_Buffer const &other) {
    Used = other.Used;
    _Reserved = other._Reserved;

    Data = New_And_Ensure_Allocator<byte>(_Reserved, Allocator);
    CopyElements(Data, other.Data, Used);
}

Byte_Buffer::Byte_Buffer(Byte_Buffer &&other) { *this = std::move(other); }

Byte_Buffer &Byte_Buffer::operator=(Byte_Buffer const &other) {
    if (Data) release(*this);

    Used = other.Used;
    _Reserved = other._Reserved;

    Data = New_And_Ensure_Allocator<byte>(_Reserved, Allocator);
    CopyElements(Data, other.Data, Used);

    return *this;
}

Byte_Buffer &Byte_Buffer::operator=(Byte_Buffer &&other) {
    if (this != &other) {
        if (Data) release(*this);

        Data = other.Data;
        Used = other.Used;
        Allocator = other.Allocator;
        _Reserved = other._Reserved;

        other.Data = null;
        other.Used = 0;
        other._Reserved = 0;
    }
    return *this;
}

void reserve(Byte_Buffer &buffer, size_t size) {
    if (buffer._Reserved >= size) return;

    byte *newData = New_And_Ensure_Allocator<byte>(size, buffer.Allocator);
    CopyElements(newData, buffer.Data, buffer.Used);
    Delete(buffer.Data, buffer._Reserved, buffer.Allocator);

    buffer._Reserved = size;
    buffer.Data = newData;
}

GU_END_NAMESPACE
