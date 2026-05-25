# Activity Journal - Laura Abalsamo (s353718)
## May 3rd
- Conducted a comparative evaluation of software design analysis tools tailored for Wireshark;
- Selected the "Understand" static analysis platform, following a comprehensive review of its technical specifications and diagnostic capabilities;
- Initiated source code analysis to map architectural design patterns, leveraging the advanced features of the "Understand" suite.

## May 8th
Activities done since last commit:
- Finalized the analysis environment setup by completing the indexing of the Wireshark source code (approx. 5.5 million lines) in the "Understand" suite;
- A preliminary structural overview of the repository was done, identifying the epan and ui/qt directories as priority areas for architectural investigation;
- Initiated the mapping of high-level dependencies between the core engine and the various protocol modules, in order to identify recurring organizational structures.

## May 10th
Activities done since last commit:
- Analysis of the epan module and examination of the proto.c file to isolate the initialization and protocol management logic;
- The implementation of the Abstract Factory pattern has been identified;
- The process of defining roles within the identified registration architecture has begun.

## May 20th
Activities done since last commit:
- Investigation of the role and relationships between the proto_node, proto_item, and proto_tree structures within epan/proto.h and epan/proto.c. Understanding how Wireshark uses these fundamental elements to construct the protocol tree that maps each level of the analyzed packet;
- Study of how dissectors rely on these structures, utilizing functions such as proto_tree_add_item() to recursively nest protocols and fields, maintaining a clear separation between the raw data and its logical representation.

## May 23th
Activities done since last commit:
-Identified the architectural relationship between the proto_node structures as a concrete implementation of the Composite Pattern, noting how it allows the dissection engine to treat both individual packet fields and nested protocol containers uniformly;
-Initiated a comparative analysis of potential alternative design solutions to evaluate their respective trade-offs in terms of memory overhead and processing efficiency.
