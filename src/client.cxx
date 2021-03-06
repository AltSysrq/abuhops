#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <ctime>
#include <cstring>
#include <vector>
#include <functional>

#include "common.hxx"
#include "auth.hxx"
#include "service.hxx"
#include "realm.hxx"
#include "clientelle.hxx"
#include "client.hxx"
#include "packets.hxx"

using namespace std;

#ifdef CLIENT_DEBUG
#include <iostream>
#define debug(str) cerr << str << endl
#else
#define debug(str)
#endif

Client::Client(const Realm* realm_,
               const asio::ip::udp::endpoint& ep)
: realm(realm_),
  endpoint(ep),
  online(false),
  lastContact(time(NULL)),
  advert(realm, endpoint),
  advertIterator(realm),
  isIteratingAdverts(false),
  advertIterationTimer(service)
{ }

Client::Client(const Client& that)
: realm(that.realm),
  endpoint(that.endpoint),
  online(that.online),
  lastContact(that.lastContact),
  advert(that.advert),
  advertIterator(that.advertIterator),
  isIteratingAdverts(that.isIteratingAdverts),
  advertIterationTimer(service)
{ }

bool Client::isOnline() const {
  return online && time(NULL) < lastContact + CONNECTION_TIMEOUT;
}

void Client::kill() {
  online = false;
}

const asio::ip::udp::endpoint& Client::getEndpoint() const {
  return endpoint;
}

void (Client::*const Client::messages[256])(const byte*, unsigned) = {
  &Client::connect,
  &Client::ping,
  &Client::proxy,
  &Client::post,
  &Client::list,
  &Client::sign,
  &Client::bye,
  //NULLs to end of array are implicit
};

#define READ(type,from) (*(reinterpret_cast<const type*>(from)))
#define WRITE(type,to) (*(reinterpret_cast<type*>(to)))

void Client::connect(const byte* dat, unsigned len) {
  contact();

  debug(">> CONNECT");

  //Check length sanity
  if (len < 2*4 + HMAC_SIZE + 2 ||
      len > 2*4 + HMAC_SIZE + MAX_CLIENT_NAME_LENGTH + 1)
    return;

  debug("Length OK");

  //Ensure NUL-terminated
  if (dat[len-1]) return;

  debug("NUL OK");

  unsigned id = READ(unsigned, dat);
  unsigned timestamp = READ(unsigned, dat+4);
  byte hmac[HMAC_SIZE];
  memcpy(hmac, dat+8, HMAC_SIZE);

  const char* name = (const char*)(dat+8+HMAC_SIZE);
  if (!*name) return; //Empty name

  debug("Name OK");

  if (authenticate(realm, id, timestamp, name, hmac)) {
    debug("Auth OK");
    online = true;

    this->id = id;
    this->name = name;

    //Send response
    byte dontKnowWhoIAm = 1;
    ping(&dontKnowWhoIAm, 1);
  }
}

void Client::ping(const byte* dat, unsigned len) {
  contact();

  if (len == 1 && dat[0]) {
    //Client needs to know its external address/port
    vector<byte> response(1 + realm->addressSize + 2);
    response[0] = PAK_YOUARE;

    realm->encodeAddress(&response[1], endpoint.address());
    WRITE(unsigned short, &response[response.size()-2]) = endpoint.port();

    sendPacket(endpoint, &response[0], response.size());
  } else {
    //Just respond for the sake of two-way traffic
    byte response = PAK_PONG;
    sendPacket(endpoint, &response, 1);
  }
}

void Client::proxy(const byte* dat, unsigned len) {
  contact();

  if (len > MAX_PROXY_SIZE + realm->addressSize + 2) return;
  if (len < 1 + realm->addressSize + 2) return;

  asio::ip::address addr;
  unsigned short port;
  realm->decodeAddress(addr, dat);
  port = READ(unsigned short, dat+realm->addressSize);

  //OK, send the packet if there is a client on that endpoint
  asio::ip::udp::endpoint ep(addr, port);
  if (getClientForEndpoint(realm, ep, false)) {
    byte pack[1+MAX_PROXY_SIZE];
    pack[0] = PAK_FROMOTHER;
    memcpy(&pack[1], dat + realm->addressSize + 2, len-realm->addressSize-2);
    sendPacket(ep, pack, 1 + len-realm->addressSize-2);
  }
}

void Client::post(const byte* dat, unsigned len) {
  contact();

  if (len > MAX_ADVERTISEMENT_SIZE)
    return;

  advert.setData(dat, len);
}

void Client::list(const byte* dat, unsigned len) {
  contact();

  if (isIteratingAdverts) return;

  isIteratingAdverts = true;
  advertIterator.reset();
  setIterationTimer();
}
void Client::sign(const byte* dat, unsigned len) {
  contact();
  //TODO
}

void Client::bye(const byte* dat, unsigned len) {
  contact();

  online = false;
}

void Client::contact() {
  lastContact = time(NULL);
}

//G++'s bind1st seems to be broken, as it tries to overload its operator() in a
//forbidden manner.
struct ClientTHBind1st {
  Client* that;
  void (*f)(Client*, const asio::error_code&);

  void operator()(const asio::error_code& code) {
    f(that, code);
  }
};

void Client::setIterationTimer() {
  advertIterationTimer.expires_from_now(boost::posix_time::milliseconds(512));
  ClientTHBind1st f;
  f.that = this;
  f.f = &Client::timerHandler_static;
  advertIterationTimer.async_wait(f);
}

void Client::timerHandler_static(Client* c, const asio::error_code& code) {
  c->timerHandler(code);
}

void Client::timerHandler(const asio::error_code&) {
  asio::ip::udp::endpoint from;
  vector<byte> pack(1);
  if (advertIterator.next(from, pack)) {
    pack[0] = PAK_ADVERT;
    sendPacket(endpoint, &pack[0], pack.size());
    setIterationTimer();
  } else {
    isIteratingAdverts = false;
  }
}
