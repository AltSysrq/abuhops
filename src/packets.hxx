#ifndef PACKETS_HXX_
#define PACKETS_HXX_

#define PAK_CONNECT    ((unsigned char)0x00)
#define PAK_PING       ((unsigned char)0x01)
#define PAK_PROXY      ((unsigned char)0x02)
#define PAK_POST       ((unsigned char)0x03)
#define PAK_LIST       ((unsigned char)0x04)
#define PAK_SIGN       ((unsigned char)0x05)
#define PAK_BYE        ((unsigned char)0x06)

#define PAK_YOUARE     ((unsigned char)0x00)
#define PAK_PONG       ((unsigned char)0x01)
#define PAK_FROMOTHER  ((unsigned char)0x02)
#define PAK_ADVERT     ((unsigned char)0x03)
#define PAK_SIGNATURE  ((unsigned char)0x05)

#endif /* PACKETS_HXX_ */
