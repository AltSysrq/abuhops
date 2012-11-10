#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <map>
#include <vector>

#include "common.hxx"
#include "realm.hxx"
#include "advert.hxx"

using namespace std;

static realmed<map<asio::ip::udp::endpoint, Advert*> > adverts;

Advert::Advert(const Realm* r, const asio::ip::udp::endpoint& ep)
: realm(r), endpoint(ep)
{
  adverts[r][ep] = this;
}

Advert::Advert(const Advert& that)
: realm(that.realm), endpoint(that.endpoint)
{
  adverts[realm][endpoint] = this;
}

Advert::~Advert() {
  map<asio::ip::udp::endpoint,Advert*>::iterator it =
    adverts[realm].find(endpoint);
  if (it != adverts[realm].end() && it->second == this)
    adverts[realm].erase(it);
}

void Advert::setData(const byte* dat, unsigned len) {
  data.assign(dat, dat+len);
}

Advert::iterator::iterator(const Realm* r)
: realm(r), beginning(true)
{ }


Advert::iterator::iterator(const iterator& that)
: realm(that.realm), prev(that.prev), beginning(that.beginning)
{ }

Advert::iterator& Advert::iterator::operator=(const iterator& that) {
  realm = that.realm;
  prev = that.prev;
  beginning = that.beginning;
  return *this;
}

bool Advert::iterator::next(asio::ip::udp::endpoint& ep, vector<byte>& dst) {
  map<asio::ip::udp::endpoint,Advert*>::const_iterator it =
    beginning? adverts[realm].begin() : adverts[realm].lower_bound(prev);
  //Move past the key (we got this last time)
  if (beginning && it != adverts[realm].end())
    ++it;
  beginning = false;

  //Move past any empty ones
  while (it != adverts[realm].end() && it->second->data.empty())
    ++it;

  if (it != adverts[realm].end()) {
    ep = it->second->endpoint;
    dst.insert(dst.end(), it->second->data.begin(), it->second->data.end());
    prev = ep;
    return true;
  } else {
    return false;
  }
}

void Advert::iterator::reset() {
  beginning = true;
}
