Comm
====

This subsystem sends and accepts data to and from the outside.


This system is maximum stupid:

	each connection will contain a string of the type of data that is requested and will be enqueued in a queue

	each _send call will work on the queue



TODO:
- expiresAt
- exit forks
- windows
- close port (?)
- websocket bridge
- documentation
- wait for newest buffer
- unittest
- nooutput-option to not save anything to disk
- make port a cli and config option
