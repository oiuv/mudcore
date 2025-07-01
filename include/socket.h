#ifndef SOCKET_H
#define SOCKET_H

// socket_mode
#define MUD                 0   // for sending LPC data types using TCP protocol.
#define STREAM              1   // for sending raw data using TCP protocol.
#define DATAGRAM            2   // for using UDP protocol.
#define STREAM_BINARY       3
#define DATAGRAM_BINARY     4
#define STREAM_TLS          5
#define STREAM_TLS_BINARY   6

// socket_option
#define SO_INVALID          0
#define SO_TLS_VERIFY_PEER  1
#define SO_TLS_SNI_HOSTNAME 2

#endif          /* SOCKET_H */
