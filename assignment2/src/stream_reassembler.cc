#include "stream_reassembler.hh"
using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _wait(0), _eof(numeric_limits<size_t>::max()) {}


void StreamReassembler::checked_insertions(const string &data, uint64_t index) {
    string curr_data = data;                     // temporarily store incoming data.
    size_t curr_s = index;                       // temporarily store index.
    size_t curr_e = index + curr_data.size() - 1; // temporarily store end index.
    auto a = _input_map.begin();                 // starting index
    while (a != _input_map.end()) {              // loop through _input_map.
        const string &tmp = (*a).second;         // temporarily store string from _input_map.
        size_t i = (*a).first;                   // it's index.
        size_t j = i + tmp.size() - 1;           // it's end index.
        if (curr_s <= j && i <= curr_e) {        // if there is overlap.
            if (curr_s <= i && curr_e >= j) {    // if incoming data is larger than the data in _input_map.
                a = _input_map.erase(a);         // erase the data in _input_map.
            } else if (i <= curr_s && j >= curr_e) { // if incoming data is smaller than the data in _input_map.
                curr_data.clear();               // clear the incoming data.
                ++a;                             // move to next data in _input_map.
            } else {                             // if incoming data is partially overlapping with the data in _input_map.
                if (curr_s > i) { 
                    index = i;
                    curr_data.insert(0, tmp.substr(0, curr_s - i)); // insert the data in _input_map to incoming data.
                } else {
                    curr_data += tmp.substr(curr_e + 1 - i, j - curr_e); // append the data in _input_map to incoming data partially.
                }
                a = _input_map.erase(a);          // erase the data in _input_map.
            }
        } else {
            ++a;                                  // move to next data in _input_map.
        }
    }
    if (!curr_data.empty()) {
        size_t temp = min(min(_output.bytes_read() + _capacity - index, curr_data.size()), _capacity - _output.buffer_size() - unassembled_bytes());
        _input_map.insert(make_pair(index, curr_data.substr(0, temp))); // insert the trimmed incoming data to _input_map.
    }
}

void StreamReassembler::update() {
    auto a = _input_map.begin();
    while (a != _input_map.end()) {
        size_t i = (*a).first;
        if (i < _wait) {
            string tmp_string = (*a).second;
            a = _input_map.erase(a);              // erasing, it will either be discarded or modified
            if (i + tmp_string.size() > _wait) {  // if the end index of incoming byte is larger than the waiting index, check for insertion.
                tmp_string = tmp_string.substr(_wait - i);
                i = _wait;
                checked_insertions(tmp_string, i); // insert the trimmed incoming data to _input_map.
            }
        } else {                                  // if the end index of incoming byte is smaller than the waiting index, discard it.
            ++a;
        }
    }
}

void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    // Determine the starting position for this substring in the output stream.
    if (eof) _eof = index + data.size();
    size_t _start_pos = 0; 
    if (index < _wait) {                          // if index of incoming is smaller than waiting index, shift the starting position.
        _start_pos = _wait - index;
    }
    if (_start_pos >= data.size()) {              // if starting position is larger than the size of incoming data, return.
        if (empty() && _wait >= _eof) {
            _output.end_input();
        }
        return;
    }
    size_t _new_ind = index + _start_pos;
    string _modified_data = data.substr(_start_pos); // Resize the copy of the data string if needed.
    if (_new_ind > _wait) {
        checked_insertions(_modified_data, index);
    } else if (_new_ind == _wait) {
        size_t temp = min(min(_output.bytes_read() + _capacity - index, _modified_data.size()), _capacity - _output.buffer_size() - unassembled_bytes());
        _modified_data = _modified_data.substr(0, temp);
        size_t _written = _output.write(_modified_data);
        _wait += _written;
        while (true) {                            // reassembly
            update();
            auto a = _input_map.find(_wait);
            if (a == _input_map.end()) {
                break;                            // if _input_map is empty, break.
            }
            _written = _output.write((*a).second);
            _wait += _written;
            _input_map.erase(a);
        }
    }
    if (empty() && _wait == _eof) {
        _output.end_input();                      // if _input_map is empty and waiting index is equal to eof index, end the input.
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t c = 0;
    for (auto &a : _input_map) {
        c += a.second.size();                     // add the size of each string in _input_map.
    }
    return c;
}

bool StreamReassembler::empty() const {
    return _input_map.empty();                    // if _input_map is empty, return true.
}

size_t StreamReassembler::ack_index() const {
    return _wait;                                 // return the output's bytes read. 
}