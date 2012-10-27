#ifndef REALM_HXX_
#define REALM_HXX_

#include <asio.hpp>

#include <map>

#include "common.hxx"

/**
 * A Realm is primarily used as a key to classify clients and data based on
 * which IP world they live in. It also handles the messy work of
 * encoding/decoding addresses belonging to the realm.
 */
class Realm {
  Realm(unsigned, const byte*,
        const byte* (*)(const asio::ip::address&));

  const byte* (*const getBytesFromAddress)(const asio::ip::address&);

public:
  const unsigned addressSize;
  const byte* addressEncoding;

  void encodeAddress(byte*, const asio::ip::address&) const;
  void decodeAddress(asio::ip::address&, const byte*) const;

  static const Realm v4, v6;
};

/** A map keyed to the possible Realms. */
template<typename T>
class realmed: public std::map<const Realm*, T> {
};

#endif /* REALM_HXX_ */
