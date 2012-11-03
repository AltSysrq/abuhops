#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <ctime>
#include <cstring>

#include "common.hxx"
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

void Client::connect(const byte* dat, unsigned len) {
  contact();
  //TODO
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
