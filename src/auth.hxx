#ifndef AUTH_HXX_
#define AUTH_HXX_

#include "common.hxx"

class Realm;

#define HMAC_SIZE 32

/**
 * Attempts to authenticate a client with the given information.
 *
 * The authentication will only be accepted if the timestamp is not from the
 * future, is not too far in the past, and the id/timestamp combination has not
 * yet been seen in this realm. (The hmac itself must be valid too, of course.)
 */
bool authenticate(const Realm*,
                  unsigned id, unsigned timestamp,
                  const char* name,
                  const byte* hmac);

/**
 * Performs periodic maintenance on the authentication data structures. Namely,
 * id/timestamp combinations from too far in the past are forgotten.
 */
void updateAuthentication();

#endif /* AUTH_HXX_ */
