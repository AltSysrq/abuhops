#ifndef ADVERT_HXX_
#define ADVERT_HXX_

#include <asio.hpp>

#include <vector>

#include "common.hxx"

class Realm;

/**
 * Represents a posted advertisement associated with an endpoint.
 */
class Advert {
  const Realm*const realm;
  const asio::ip::udp::endpoint endpoint;
  std::vector<byte> data;

  //Not defined
  Advert& operator=(const Advert&);

public:
  /**
   * Constructs a new, empty advert for the given endpoint.
   *
   * If an Advert for the given endpoint already exists, it is disassociated.
   */
  explicit Advert(const Realm*, const asio::ip::udp::endpoint&);
  /// Copy constructor
  Advert(const Advert&);
  ~Advert();

  /**
   * Sets the data assocaited with this advert to the given byte array (which
   * is copied).
   */
  void setData(const byte*, unsigned);

  /**
   * Iterates through each Advert with non-empty data.
   */
  class iterator {
    const Realm* realm;
    asio::ip::udp::endpoint prev;
    bool beginning;

  public:
    explicit iterator(const Realm*);
    iterator(const iterator&);
    iterator& operator=(const iterator&);

    /**
     * Returns the data and endpoint of the next Advert. Data, if any, is
     * appended to the vector given (though the endpoint is replaced).
     * Subsequent calls will return subsequent adverts.
     *
     * Returns whether anything was retrieved.
     */
    bool next(asio::ip::udp::endpoint&, std::vector<byte>&);

    /**
     * Resets the iterator to start from the beginning again.
     */
    void reset();
  };
};

#endif /* ADVERT_HXX_ */
