#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include "clientelle.hxx"
#include "client.hxx"
#include "realm.hxx"
#include "dispatch.hxx"
#include "packets.hxx"

void dispatchPacket(const asio::ip::udp::endpoint& from,
                    const byte* data, unsigned len) {
  if (!len) return;

  //Check for a handler first, since it's cheaper
  void (Client::*handler)(const byte*, unsigned) =
    Client::messages[data[0]];
  if (!handler)
    return;

  //Get the client associated. Only create if this is a CONNECT packet.
  const Realm* realm = from.address().is_v4()? &Realm::v4 : &Realm::v6;
  Client* client = getClientForEndpoint(realm, from, data[0] == PAK_CONNECT);
  if (!client) return;

  (client->*handler)(data+1, len-1);
}
