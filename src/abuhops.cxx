#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "common.hxx"
#include "service.hxx"
#include "clientelle.hxx"

using namespace std;

int main(void) {
  if (!initService())
    return 1;

  addMaintFunc(cleanClientMaps);
  runListenReadLoop();
}
