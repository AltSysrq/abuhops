#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <crypto++/hmac.h>
#include <crypto++/sha.h>

#include <algorithm>
#include <map>
#include <vector>
#include <cassert>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include "common.hxx"
#include "auth.hxx"
#include "realm.hxx"

using namespace std;

#ifdef CLIENT_DEBUG
#include <iostream>
#define debug(str) cerr << str << endl
#else
#define debug(str)
#endif

typedef CryptoPP::HMAC<CryptoPP::SHA256> Hmac;

class HmacCkecker {
  vector<byte> key;

public:
  HmacCkecker() {
    FILE* in = fopen(SHARED_SECRET_FILE, "rb");
    if (!in) {
      fprintf(stderr, "Unable to open %s: %s\n",
              SHARED_SECRET_FILE, strerror(errno));
      exit(1);
    }

    byte buffer[4096];
    size_t read;
    do {
      read = fread(buffer, 1, sizeof(buffer), in);
      key.insert(key.end(), buffer, buffer+read);
    } while (read == sizeof(buffer));

    if (ferror(in)) {
      fprintf(stderr, "Error reading %s: %s\n",
              SHARED_SECRET_FILE, strerror(errno));
      exit(1);
    }
    fclose(in);
  }

  bool operator()(const byte* data, size_t datasz, const byte* digest) {
    Hmac hmac;
    hmac.SetKey(&key[0], key.size());
    return hmac.VerifyDigest(digest, data, datasz);
  }
} static checkHmac;

//"map" timestamps to ids (this allows us to easily track the pairs, and to
//quickly remove out-of-date ranges)
static realmed<multimap<unsigned,unsigned> > recentTimestamps;

static unsigned truncatedTime() {
  return (unsigned)(time(NULL) & 0xFFFFFFFF);
}

bool authenticate(const Realm* realm,
                  unsigned id, unsigned timestamp,
                  const char* name, const byte* hmac) {
  unsigned now = truncatedTime();
  if (timestamp > now || timestamp < now-TIMESTAMP_VARIANCE)
    return false;
  debug("Auth timestamp OK");

  //Ensure the timestamp/id pair does not exist
  pair<multimap<unsigned,unsigned>::iterator,
       multimap<unsigned,unsigned>::iterator>
    bounds = recentTimestamps[realm].equal_range(timestamp);
  if (bounds.second != find(bounds.first, bounds.second,
                            pair<const unsigned,unsigned>(timestamp,id)))
    //Already seen
    return false;

  debug("Auth timestamp non-dupe");

  vector<byte> data(2*4 + strlen(name));
  memcpy(&data[0], &id, 4);
  memcpy(&data[4], &timestamp, 4);
  memcpy(&data[8], name, strlen(name));
  //Ensure little-endian
  assert(data[0] == (id & 0xFF));

  if (checkHmac(&data[0], data.size(), hmac)) {
    //OK
    recentTimestamps[realm].insert(make_pair(timestamp,id));
    debug("HMAC OK");
    return true;
  } else {
    //Invalid HMAC
    debug("HMAC Invalid");
    return false;
  }
}

void updateAuthentication() {
  for (realmed<multimap<unsigned,unsigned> >::iterator it =
         recentTimestamps.begin();
       it != recentTimestamps.end(); ++it)
  it->second.erase(it->second.begin(),
                   it->second.upper_bound(
                     truncatedTime()-TIMESTAMP_VARIANCE));
}
