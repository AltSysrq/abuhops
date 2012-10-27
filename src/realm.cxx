#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <cstring>
#include <cassert>

#include "realm.hxx"

using namespace std;

static const asio::ip::address_v4 nativeIpv4Address(
  asio::ip::address_v4::from_string("00.01.02.03"));
static const asio::ip::address_v6 nativeIpv6Address(
  asio::ip::address_v6::from_string(
    "0100:0302:0504:0706:0908:0B0A:0D0C:0F0E"));

template<unsigned Len, typename T>
const byte* getAddressEncoding(const T& t) {
  static byte data[Len];
  const byte* raw = reinterpret_cast<const byte*>(&t[0]);
  bool isNativeCorrect = true;
  for (unsigned i = 0; i < Len; ++i) {
    data[raw[i]] = i;
    isNativeCorrect &= (raw[i] == i);
  }

  return isNativeCorrect? NULL : data;
}

//This is not reentrant, but this won't be an issue since this program is not
//multi-threaded.
template<typename T,
         T (asio::ip::address::* F)() const>
const byte* implGetBytesFromAddress(const asio::ip::address& addr) {
  static typename T::bytes_type t;
  t = (addr.*F)().to_bytes();
  return reinterpret_cast<const byte*>(&t[0]);
}

const Realm Realm::v4(4,
                      getAddressEncoding<4>(
                        nativeIpv4Address.to_bytes()),
                      implGetBytesFromAddress<asio::ip::address_v4,
                                              &asio::ip::address::to_v4>);
const Realm Realm::v6(16,
                      getAddressEncoding<16>(
                        nativeIpv6Address.to_bytes()),
                      implGetBytesFromAddress<asio::ip::address_v6,
                                              &asio::ip::address::to_v6>);

Realm::Realm(unsigned l, const byte* enc,
             const byte* (*gbfa)(const asio::ip::address&))
: getBytesFromAddress(gbfa),
  addressSize(l), addressEncoding(enc)
{ }

void Realm::encodeAddress(byte* dst, const asio::ip::address& addr) const {
  const byte* src = getBytesFromAddress(addr);
  if (addressEncoding)
    for (unsigned i = 0; i < addressSize; ++i)
      dst[addressEncoding[i]] = src[i];
  else
    memcpy(dst, src, addressSize);
}

void Realm::decodeAddress(asio::ip::address& addr, const byte* src) const {
  byte* dst = const_cast<byte*>(getBytesFromAddress(addr));
  if (addressEncoding)
    for (unsigned i = 0; i < addressSize; ++i)
      dst[i] = src[addressEncoding[i]];
  else
    memcpy(dst, src, addressSize);
}

