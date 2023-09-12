#include "byte_stream.hh"
#include <algorithm>
// You will need to add private members to the class declaration in `byte_stream.hh`
/* Replace all the dummy definitions inside the methods in this file. */

using namespace std;
ByteStream::ByteStream(const size_t capa) : capacity(capa), readLen(0), writeLen(0), _input_ended(false), _error(false){}

size_t ByteStream::write(const string &data) {
  if (_input_ended || _error) {
    return 0;
  }
  size_t writtenBytes = 0;
  for (const char byte : data) {
    if (_buffer.size() < capacity) {
      _buffer.push_back(byte);
      writtenBytes++;
    } else {
      break; // buffer is full, hence I will break the loop.
    }
  }
  writeLen += writtenBytes; // Updating the number of bytes written
  return writtenBytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
  string ans;
  for (size_t i = 0; i < len && i < _buffer.size(); ++i) {
    ans += _buffer[i];
  }
  return ans;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
  if (_buffer.empty()) {
    return;  // Nothing to pop
  }
  if (len > _buffer.size()) {
    set_error();  // Can't pop more bytes than are in the buffer
    return;  // Can't pop more bytes than are in the buffer
  }
  // Remove the specified number of bytes from the front of the buffer
  _buffer.erase(_buffer.begin(), _buffer.begin() + len);
  readLen += len;  // Update the number of bytes read
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
  string ans = peek_output(len);
  pop_output(len);
  return ans;
}

void ByteStream::end_input() {_input_ended = true;}

bool ByteStream::input_ended() const { return _input_ended;}

size_t ByteStream::buffer_size() const {return _buffer.size();}

bool ByteStream::buffer_empty() const {return _buffer.empty();}

bool ByteStream::eof() const {return (_input_ended && _buffer.empty());}

size_t ByteStream::bytes_written() const {return writeLen;}

size_t ByteStream::bytes_read() const {return readLen;}

size_t ByteStream::remaining_capacity() const {return capacity - _buffer.size();}