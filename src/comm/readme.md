Comm
====

This subsystem accepts queries from and sends data to the outside.
It implements a low level tcp communication with every
process that connects to port config->port, which can be set in the
configfile ("port=<port>") or as a command line option ("--port
<port>").

The system allows for an infinite number of socket connection, but each
one will be handled in its own process (fork).

The following protocol is observed:

Each message consists of the message length as an int calculated in chars,
excluding the length itself and the binary payload.

The client requests a buffer by sending the name of that buffer, valid buffer names are
- top - top camera
- bot - bot camera
- cam - webcam
- spc - spectrum
eg. 3top would request the top buffer.

The system then sends the length of the buffer, followed by the buffer.


This subsystem exposes three methods
- int comm_init(sConfigStructconfig);
  set up data structures, open a tcp port etc
- int comm_set_buffer(charcmd, charbuffer, int size);
  tcp requests are fullfilled on demand from a prefilled buffer. That
  buffer need to be updated with this function. The provided buffer
  is memcopied internally and can be freed afterwards.
- int comm_uninit(sConfigStructconfig);
  tear down, stop all child processes and close ports

All other functions are internal.
