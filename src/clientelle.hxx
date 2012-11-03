#ifndef CLIENTELLE_HXX_
#define CLIENTELLE_HXX_

#include <asio.hpp>

class Client;
class Realm;

/**
 * Returns a/the Client associated with the given asio endpoint in the given
 * realm. One is created if it does not yet exist.
 *
 * This may kill clients if too many come from the same IP address.
 */
Client* getClientForEndpoint(const Realm*, const asio::ip::udp::endpoint&);

/**
 * Cleans all offline clients.
 */
void cleanClientMaps();

#endif /* CLIENTELLE_HXX_ */
