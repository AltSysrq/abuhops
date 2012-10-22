#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <cassert>
#include <iostream>
#include <fstream>
#include <map>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>

#include <gmp.h>

using namespace std;

/* SERVER CONFIGURATION.
 * To reconfigure the Abuhops server, redefine these, or pass equivilant
 * definitions in CXXFLAGS to make.
 */

/** The file in which the shared secret is stored. */
#ifndef SHARED_SECRET_FILE
#define SHARED_SECRET_FILE "/usr/local/etc/abuhops/shared_secret"
#endif

/** The port number the server listens on */
#ifndef PORT
#define PORT 12545
#endif

/** The maximum number of seconds in the past or future a timestamp is allowed
 * to be. Since timestamps are provided by the authentication server, this
 * should be a small value.
 */
#ifndef TIMESTAMP_VARIANCE
#define TIMESTAMP_VARIANCE 15
#endif
/** The time in seconds after which an idle connection is considered gone. */
#ifndet CONNECTION_TIMEOUT
#define CONNECTION_TIMEOUT 300
#endif
/** The maximum length, in bytes, of a client advertisement. */
#ifndef MAX_ADVERTISEMENT_SIZE
#define MAX_ADVERTISEMENT_SIZE 128
#endif
/** Read random data from this file */
#ifndef RANDOM_SOURCE
#define RANDOM_SOURCE "/dev/urandom"
#endif



#define ELGAMAL_BASE "2"
#define ELGAMAL_MODULUS "444291e51b3ea5fd16673e95674b01e7b"
#define SIGNATURE_CHUNK_SIZE 17

typedef unsigned char byte;

static mpz_t elGamalModulus, elGamalModulusMinusOne, elGamalBase;
static FILE* randomSource;

template<typename Address>
Address convertAddress(const asio::ip::address&) {
  return Address::is_not_an_understood_address_type;
}

template<>
asio::ip::address_v4
convertAddress<asio::ip::address_v4>(const asio::ip::address& addr) {
  return addr.to_v4();
}

template<>
asio::ip::address_v6
convertAddress<asio::ip::address_v6>(const asio::ip::address& addr) {
  return addr.to_v6();
}

template<typename Address>
class Abuhops {
  typedef typename Address::bytes_type AddressBytes;
  class Client {
  public:
    Address address;
    unsigned short port;
    unsigned identifier;
    string name;
    time_t lastContact;
    unsigned signatureDomain;
    byte signature[SIGNATURE_CHUNK_SIZE*2];

    Client(const asio::ip::udp::endpoint& endpoint,
           unsigned id, const string& name_)
    : address(convertAddress<Address>(endpoint.address())),
      port(endpoint.port()),
      identifier(id), name(name_),
      lastContact(time(0)),
      signatureDomain(0)
    { }

    //Default constructor, only used for temporaries (ie, followed by
    //assignment)
    Client() {}
  };

  map<asio::ip::udp::endpoint, Client> clientsByEndpoint;
  map<unsigned, Client> clientsById;

  class SignatureDomain {
  public:
    mpz_t publicKey, privateKey;
    unsigned refcount;

    SignatureDomain()
    : refcount(0)
    {
      //Generate ElGamal keys
      mpz_init(privateKey);
      do {
        //Read substantially more raw data in than the modulus size, so that a
        //simple modulo will still give a "uniform enough" distribution.
        byte rawRandom[8*SIGNATURE_CHUNK_SIZE];
        fread(rawRandom, sizeof(rawRandom), 1, randomSource);
        mpz_import(privateKey, sizeof(rawRandom), -1, 1, -1, 0, rawRandom);

        mpz_mod(privateKey, privateKey, elGamalModulusMinusOne);
      } while (mpz_cmp_ui(privateKey, 1) <= 0);

      mpz_init(publicKey);
      mpz_powm(publicKey, elGamalBase, privateKey, elGamalModulus);
    }

    SignatureDomain(const SignatureDomain& that)
    : refcount(that.refcount)
    {
      mpz_init_set(publicKey, that.publicKey);
      mpz_init_set(privateKey, that.privateKey);
    }

    ~SignatureDomain() {
      mpz_clear(publicKey);
      mpz_clear(privateKey);
    }

    void getPublicKey(byte* dst) const {
      size_t cnt;
      memset(dst, 0, SIGNATURE_CHUNK_SIZE);
      mpz_export(dst, &cnt, -1, -1, 0, public_key);
      assert(cnt <= SIGNATURE_CHUNK_SIZE);
    }
  };
};
