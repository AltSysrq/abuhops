#ifndef COMMON_HXX_
#define COMMON_HXX_

#include <asio.hpp>

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
#ifndef CONNECTION_TIMEOUT
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

#endif /* COMMON_HXX_ */
