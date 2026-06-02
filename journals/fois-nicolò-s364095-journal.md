# Activity Journal - Nicolò Fois (s364095)

## May 3rd
Activity done since last commit:
- Analyzed external libraries (npcap/libpcap)
- Investigated context diagram stakeholders
- Contributed to information gathering for the definition of the Context Diagram (L1)

## May 4th
Activity done since last commit:
- Conducted an in-depth architectural analysis of libpcap/npcap
- Started the Reverse Engineering of dumpcap as a Level 2 Container, specifically mapping the Inter-Process Communication (IPC) via pipes and the asynchronous data flow between libpcap, dumpcap, and the Wireshark GUI
- Initiated the formal Software Architecture Report, documenting the Context Level (L1) and the dumpcap interaction with the system (L2)

## May 14th
Activity done since last commit:
- Conducted a detailed analysis of the Capture and Wiretap modules
- Investigated the internal data flow and interfacing mechanisms between these modules, the Core, and the File System (Hard Disk)

## May 17th
Activity done since last commit:
- Refined the Context Diagram (L1) by treating Wireshark as a pure black box, properly abstracting internal deployable components like Dumpcap
- Drafted the Container Diagram (L2) using PlantUML and the formal C4 Model to clarify the separation of privileges and the roles of the components
- Integrated both automatic and detailed custom legends into the PlantUML scripts to ensure clear architectural documentation

## May 25th
Activity done since last commit:
- Investigated the actual nature of the Core module, clarifying its role as "glue code", rather than a well-defined standalone component
- Performed a SOLID principles analysis for each L3 component
- Developed a Python script to compute quantitative coupling metrics (Ca, Ce, Instability) by statically analyzing #include directives across 4109 source files (99.8% repository coverage), producing a ranked metrics table and a full cross-module dependency matrix

## June 2nd
Activity done since last commit:
- Produced a corrected diagram removing the two non-modular components, fixing the relationship directions, and adding the missing Packet Queue (GAsyncQueue) for multi-thread capture mode
- Drafted architectural overviews for the L1, L2, and L3 (dumpcap) diagrams with embedded PlantUML scripts 
- reviewed the document to eliminate redundancies and fix conceptual imprecisions