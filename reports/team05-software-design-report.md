# Software Report
## Patterns
### Chain of Responsibility
This pattern gives the possibility to send a request along a chain of potential handlers until one of them handles it. The general idea is to decouple senders and receivers. 
If *Chain of Responsibility* was not used in Wireshark where thousands of protocols have to be supported, the code would look like a huge conditional block which would be difficult to manage.

The discussed pattern is used in the `epan` folder of the project to implement the dissectors chain: every dissector analyzes its packet level and pass the payload to the next dissector through dispatch tables without knowing who will handle it. The classic pattern has been adapted from an OOP style to a C architecture based on modules and table.

The **Handler** role is developed by the struct `dissector_handle` defined in `epan/packet.c`and by the type function `dissector_t` declared in `epan/packet.h`. The successor link isn’t in the Handler but in the `dissector_table`. Every `epan/dissectors/packet-*.c` is a **Concrete Handler** which manages the packet or passes it to the successor calling it by `dissector_try_uint_new()` or `call_dissector()`. Some examples are `dissect_frame()` from `packet-frame.c`, `dissect_ip()` from `packet-ip.c` and `dissect_tcp()` from `packet-tcp.c`. The **Client**, which sends the first request is split into two different moments in Wireshark: chain building with every `proto_reg_handoff_*()` call in `packet-*.c`files and request send with `epan_dissect_run()` in `epan/epan.c` which calls `dissect_packet()` in `epan/packet.c` who calls `call_dissector(frame_handle, …)`.

Instead of *Chain of Responsibility*, other solutions could be used:
-	*centralized switch/if-else*, as written above, simple to use only with few protocols;
-	 *strategy pattern*, simpler conceptually but not capable to manage hierarchical style of protocols;
-	*observer pattern*, guarantees total decouple but not the processing order adding complexity to debug.
  
Analyzing pros and cons, the actual pattern stays the better choice because respects hierarchical and sequential nature of network protocols.
