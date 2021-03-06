#ifndef CLIENT_HXX_
#define CLIENT_HXX_

#include <asio.hpp>

#include <ctime>
#include <string>

#include "advert.hxx"
#include "common.hxx"

class Realm;

/**
 * Tracks information relating to a single client, and handles messages
 * received from that client.
 */
class Client {
  const Realm* realm;
  asio::ip::udp::endpoint endpoint;
  bool online;
  std::time_t lastContact;
  std::string name;
  unsigned id;

  Advert advert;
  Advert::iterator advertIterator;
  bool isIteratingAdverts;
  asio::deadline_timer advertIterationTimer;

public:
  /// Constructs a Client in the given Realm and with the given endpoint. It is
  /// NOT considered online.
  Client(const Realm*, const asio::ip::udp::endpoint&);
  /// Copy constructor
  Client(const Client&);

  /**
   * Returns whether this Client is considered "online". A client is online if:
   * It has passed authentication; the time of last contact is within the
   * timout interval; it has not sent the BYE message.
   */
  bool isOnline() const;

  /**
   * Immediately marks this client as not online.
   */
  void kill();

  /**
   * Returns the endpoint of this client.
   */
  const asio::ip::udp::endpoint& getEndpoint() const;

  /**
   * Maps message numbers to Client functions.
   *
   * Each function takes an array of data, where byte zero is the first byte
   * after the message type, and unsigned is the number of bytes in the packet
   * (again, excluding the initial type byte).
   *
   * Entries which are NULL indicate invalid packet types.
   */
  static void (Client::*const messages[256])(const byte*, unsigned);
  //Don't you just love Declaration Reflects Usage?

private:
  void connect     (const byte*, unsigned);
  void ping        (const byte*, unsigned);
  void proxy       (const byte*, unsigned);
  void post        (const byte*, unsigned);
  void list        (const byte*, unsigned);
  void sign        (const byte*, unsigned);
  void bye         (const byte*, unsigned);

  void contact();
  void setIterationTimer();
  static void timerHandler_static(Client*, const asio::error_code&);
  void timerHandler(const asio::error_code&);
};

#endif /* CLIENT_HXX_ */
