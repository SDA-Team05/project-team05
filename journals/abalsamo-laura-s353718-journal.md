# Activity Journal - Laura Abalsamo (s353718)
## May 3rd
- Conducted a comparative evaluation of software design analysis tools tailored for Wireshark;
- Selected the "Understand" static analysis platform, following a comprehensive review of its technical specifications and diagnostic capabilities;
- Initiated source code analysis to map architectural design patterns, leveraging the advanced features of the "Understand" suite.

## May 8th
- Finalized the analysis environment setup by completing the indexing of the Wireshark source code (approx. 5.5 million lines) in the "Understand" suite;
- A preliminary structural overview of the repository was done, identifying the epan and ui/qt directories as priority areas for architectural investigation;
- Initiated the mapping of high-level dependencies between the core engine and the various protocol modules, in order to identify recurring organizational structures.

## May 10th
- Analysis of the epan module and examination of the proto.c file to isolate the initialization and protocol management logic;
- The implementation of the Abstract Factory pattern has been identified;
- The process of defining roles within the identified registration architecture has begun.
