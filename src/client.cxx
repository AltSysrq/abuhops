#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <ctime>
#include <cstring>
#include <vector>

#include "common.hxx"
#include "auth.hxx"
#include "client.hxx"

using namespace std;

Client::Client()
: online(false)
{ }

Client::Client(const Realm* realm_,
               const asio::ip::udp::endpoint& ep)
: realm(realm_),
  endpoint(ep),
  online(false),
  lastContact(time(NULL))
{ }

Client::Client(const Client& that)
: realm(that.realm),
  endpoint(that.endpoint),
  online(that.online),
  lastContact(that.lastContact)
{ }

Client& Client::operator=(const Client& that) {
  realm = that.realm;
  endpoint = that.endpoint;
  online = that.online;
  lastContact = that.lastContact;
  return *this;
}

bool Client::operator<(const Client& that) const {
  return (this->realm < that.realm) ||
    (this->realm == that.realm && this->endpoint < that.endpoint);
}

bool Client::operator==(const Client& that) const {
  return this->realm == that.realm &&
         this->endpoint == that.endpoint;
}

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

void Client::connect(const byte* dat, unsigned len) {
  contact();

  //Check length sanity
  if (len < 2*4 + HMAC_SIZE + 2 ||
      len > 2*4 + HMAC_SIZE + MAX_CLIENT_NAME_LENGTH + 1)
    return;

  //Ensure NUL-terminated
  if (dat[len]) return;

  unsigned id = READ(unsigned, dat);
  unsigned timestamp = READ(unsigned, dat+4);
  byte hmac[HMAC_SIZE];
  memcpy(hmac, dat+8, HMAC_SIZE);

  const char* name = (const char*)(dat+8+HMAC_SIZE);
  if (!*name) return; //Empty name

  if (authenticate(realm, id, timestamp, name, hmac)) {
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
  //TODO
}
void Client::proxy(const byte* dat, unsigned len) {
  contact();
  //TODO
}
void Client::post(const byte* dat, unsigned len) {
  contact();
  //TODO
}
void Client::list(const byte* dat, unsigned len) {
  contact();
  //TODO
}
void Client::sign(const byte* dat, unsigned len) {
  contact();
  //TODO
}
void Client::bye(const byte* dat, unsigned len) {
  contact();
  //TODO
}

void Client::contact() {
  lastContact = time(NULL);
}
