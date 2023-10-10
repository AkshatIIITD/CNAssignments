#include "tcp_receiver.hh"
#include <algorithm>
using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader head = seg.header();
    bool currsynReceived = head.syn;                                     // syn received boolean
    bool currfinReceived = head.fin;                                     // fin received boolean
    if (!(currsynReceived || _synReceived)) {                            // if syn is not received
        return;
    }
    if (!_synReceived) {                                                 // update isn if syn is received
        _synReceived = true;
        _isn = head.seqno;
    }
    string duplicate = seg.payload().copy();                             // copy the payload
    if (!duplicate.empty() && (currsynReceived || head.seqno != _isn)) { // if there is payload and it is not the first segment
        uint64_t checkpoint = _reassembler.ack_index();                  // hint usage
        uint64_t abs_seqno = unwrap(head.seqno, _isn, checkpoint);       // hint usage
        uint64_t stream_idx = abs_seqno - _synReceived;                  // hint usage
        _reassembler.push_substring(duplicate, stream_idx, currfinReceived); // push the substring to reassembler
    }
    if (currfinReceived || _finReceived) {                               // if fin is received
        _finReceived = true;                                             // update fin received boolean
        if (_reassembler.unassembled_bytes() == 0) {
            _reassembler.stream_out().end_input();                       // end input if there is no unassembled bytes
        }
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    optional<WrappingInt32> ans = nullopt;                         // initialize ans to be null
    if (_synReceived) {                                            // if syn is received
        uint64_t checkpoint = _reassembler.ack_index() + 1;        // checkpoint is the acknowledgement index + 1
        if (_reassembler.stream_out().input_ended()) {             // if input is ended
            checkpoint++;                                          // increment checkpoint
        }
        ans.emplace(wrap(checkpoint, _isn));                       // wrap checkpoint and isn, and assign it to ans
    }
    return ans;
}

size_t TCPReceiver::window_size() const { 
    size_t reass_size = _reassembler.stream_out().buffer_size();   // size of the reassembler
    return _capacity - reass_size;                                 // return the window size (total capacity - reassembler size)
}