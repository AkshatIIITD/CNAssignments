#include "wrapping_integers.hh"
#include <iostream>
using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { 
    return isn + uint32_t(n); 
}
//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t start = uint64_t(UINT32_MAX) + 1; // starting number is max of uint32_t + 1
    uint64_t modulo = checkpoint % start;      // modulo is the remainder of checkpoint / start
    uint64_t diff = checkpoint - modulo;       // difference between checkpoint and modulo for comparison
    if (modulo == 0) { 
        if (diff >= start) {                   // if modulo is 0, then checkpoint is a multiple of start
            modulo += start;                   // increment modulo by start
            diff -= start;                     // decrement diff by start
        }
    }
    uint64_t n_modulo = n.raw_value() - isn.raw_value(); // n_modulo is the difference between n and isn
    uint64_t n_diff = n_modulo - modulo;                 // n_diff is the difference between n_modulo and modulo
    uint64_t unwrapped = diff + n_modulo;                // unwrapped is the sum of diff and n_modulo, will be the main value
    if (modulo < n_modulo) {                             // if modulo is less than n_modulo
        if (start <= diff && (n_diff) >= (start - n_diff)) { 
            return unwrapped - start;                    // substract start from unwrapped, as it is closer to checkpoint
        }
        return unwrapped;
    } 
    if (start + n_diff >= -n_diff) {                     // if twice the difference in modulo and n_modulo is less than start
        return unwrapped;
    }
    return unwrapped + start;                            // add start to unwrapped, as it is closer to checkpoint
}
