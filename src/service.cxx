#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <asio.hpp>

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <list>
#include <iostream>

#include "common.hxx"
#include "service.hxx"
#include "dispatch.hxx"

using namespace std;

static list<void (*)(void)> maintFuncs;

static asio::io_service service;
static asio::ip::udp::socket* sock4 = NULL, * sock6 = NULL;

void addMaintFunc(void (*f)(void)) {
  maintFuncs.push_back(f);
}

void sendPacket(const asio::ip::udp::endpoint& endpoint,
                const byte* dat, unsigned len) {
  asio::ip::udp::socket* sock = endpoint.address().is_v4()? sock4 : sock6;
  assert(sock);
  try {
    sock->send_to(asio::buffer(dat, len), endpoint);
  } catch (const asio::system_error& err) {
    cerr << "Error sending to " << endpoint << ": " << err.what() << endl;
  }
}

bool initService() {
  try {
    sock4 = new asio::ip::udp::socket(service,
                                      asio::ip::udp::endpoint(
                                        asio::ip::udp::v4(),
                                        IPV4PORT));
  } catch (const asio::system_error& err) {
    cerr << "Unable to listen for IPv4: " << err.what() << endl;
  }

  try {
    sock6 = new asio::ip::udp::socket(service,
                                      asio::ip::udp::endpoint(
                                        asio::ip::udp::v6(),
                                        IPV6PORT));
  } catch (const asio::system_error& err) {
    cerr << "Unable to listen for IPv6: " << err.what() << endl;
  }

  return sock4 || sock6;
}

static byte readBuffer[65536];
static asio::ip::udp::endpoint readEndpoint;

static void beginRead(asio::ip::udp::socket*);
static void receivePacket(const asio::error_code& error,
                          size_t size) {
  if (error) {
    cerr << "Error receiving packet: " << error.message();
    beginRead(readEndpoint.address().is_v4()? sock4 : sock6);
    return;
  }

  //0 works well enough, since we only need to test for equality, and this
  //avoids a race condition if we ever make things multithreaded.
  static time_t lastMaintRun = 0;
  time_t now = time(0);
  if (now != lastMaintRun) {
    for (list<void (*)(void)>::const_iterator it = maintFuncs.begin();
         it != maintFuncs.end(); ++it)
      (*it)();

    lastMaintRun = now;
  }

  dispatchPacket(readEndpoint, readBuffer, size);

  beginRead(readEndpoint.address().is_v4()? sock4 : sock6);
}

static void beginRead(asio::ip::udp::socket* sock) {
  assert(sock);

  sock->async_receive_from(asio::buffer(readBuffer, sizeof(readBuffer)),
                           readEndpoint, receivePacket);
}

void runListenReadLoop() {
  if (sock4) beginRead(sock4);
  if (sock6) beginRead(sock6);

  while (true) {
    try {
      service.run();
      cerr << "FATAL: service.run() returned normally!" << endl;
      assert(false);
      exit(-1);
    } catch (const asio::system_error& err) {
      cerr << "service.run() returned error: " << err.what() << endl;
    }
  }
}
