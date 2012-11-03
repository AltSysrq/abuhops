#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <map>

#include "client.hxx"

using namespace std;

/* Used to track how many connections there are from each IP address. One is
 * disconnected every time an attempt is made to have more than
 * MAX_CLIENTS_FROM_ONE_IP_ADDRESS clients from the same address.
 */
typedef multimap<asio::ip::address,Client*> connectedAddresses_t;
static connectedAddresses_t connectedAddresses;

/* Maps endpoints to clients. */
typedef map<asio::ip::udp::endpoint,Client*> clientelle_t;
static clientelle_t clientelle;

Client* getClientForEndpoint(const Realm* realm,
                             const asio::ip::udp::endpoint& ep) {
  //See if it already exists
  clientelle_t::iterator it = clientelle.find(ep);
  if (it != clientelle.end())
    return it->second;

  //Not yet existing, create a new one.
  //First, check IP address limit
  if (connectedAddresses.count(ep.address()) >=
      MAX_CLIENTS_FROM_ONE_IP_ADDRESS) {
    //Kill the first one which is still online
    connectedAddresses_t::iterator ca =
      connectedAddresses.lower_bound(ep.address());
    connectedAddresses_t::iterator end =
      connectedAddresses.upper_bound(ep.address());
    while (ca != end && ca->second->isOnline())
      ++ca;

    //If any of the clients for this address is still online, kill it. If none
    //are left, they'll be cleared soon.
    if (ca != end)
      ca->second->kill();
  }

  Client* cl = new Client(realm, ep);
  clientelle.insert(make_pair(ep, cl));
  connectedAddresses.insert(make_pair(ep.address(), cl));
  return cl;
}

void cleanClientMaps() {
  //First, remove dead clients from the address multimap.
  for (connectedAddresses_t::iterator it = connectedAddresses.begin();
       it != connectedAddresses.end(); ++it) {
    if (!it->second->isOnline()) {
      //Save the key so we can get a lower bound to resume searching
      asio::ip::address key(it->first);
      connectedAddresses.erase(it);
      it = connectedAddresses.lower_bound(key);
    }
  }

  //Clean the endpoint map
  for (clientelle_t::iterator it = clientelle.begin();
       it != clientelle.end(); ++it) {
    if (!it->second->isOnline()) {
      Client* c = it->second;
      clientelle.erase(it);
      it = clientelle.lower_bound(c->getEndpoint());
      delete c;
    }
  }
}
