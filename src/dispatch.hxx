#ifndef DISPATCH_HXX_
#define DISPATCH_HXX_

#include <asio.hpp>

#include "common.hxx"

/**
 * Examines the given packet and decides what to do with it.
 */
void dispatchPacket(const asio::ip::udp::endpoint& from,
                    const byte* data, unsigned len);

#endif /* DISPATCH_HXX_ */
