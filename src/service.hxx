#ifndef SERVICE_HXX_
#define SERVICE_HXX_

#include <asio.hpp>

#include "common.hxx"

/**
 * The io_service used for the server.
 */
extern asio::io_service service;

/**
 * Causes the given function to be called periodically (at most once per
 * second).
 *
 * More specifically, whenever a packet is received after more than a second
 * has passed since the last running, all such functions are called, in the
 * order they were given to this function.
 *
 * There is no way to undo this operation.
 */
void addMaintFunc(void (*)(void));

/**
 * Sends the given data to the given Asio endpoint, synchronously.
 *
 * No exception will be thrown; if an error occurs, a diagnostic is printed,
 * but the caller is not notified.
 */
void sendPacket(const asio::ip::udp::endpoint&, const byte*, unsigned len);

/**
 * Creates the asio service and sockets, if possible.
 * Returns whether any sockets were successfully created.
 */
bool initService();

/**
 * Begins the listen/read loop. This function will not return.
 */
void runListenReadLoop()
#ifdef __GNUC__
__attribute__((__noreturn__))
#endif
;


#endif /* SERVICE_HXX_ */
