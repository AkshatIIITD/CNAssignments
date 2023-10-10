#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#include "byte_stream.hh"
#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <map>
#include <limits>

//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:
    ByteStream _output;  //!< The reassembled in-order byte stream
    std::map<size_t, std::string> _input_map; //byte streams which require to be assembled
    size_t _capacity; //capacity 
    size_t _wait;     // waiting index 
    size_t _eof;      // eof index
    void update();    // update _input_map according to _wait. If the index of a byte is smaller than _wait_index, it is trimmed.
    void checked_insertions(const std::string &data, uint64_t index); // insert data into _input_map after verfication for any duplicacies.
    
  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receive a substring and write any newly contiguous bytes into the stream.
    //! The StreamReassembler will stay within the memory limits of the `capacity`.
    //! Bytes that would exceed the capacity are silently discarded.
    //! \param data the substring
    //! \param index indicates the index (place in sequence) of the first byte in `data`
    //! \param eof the last byte of `data` will be the last byte in the entire stream
    void push_substring(const std::string &data, const uint64_t index, const bool eof);
    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}
    //! The number of bytes in the substrings stored but not yet reassembled
    //! \note If the byte at a particular index has been pushed more than once, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;
    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;
    //! The acknowledge index of the stream, i.e., the index of the next interested substring
    size_t ack_index() const;
};
#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
