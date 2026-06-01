# Software Report
## Code Dependencies

To understand the structural coupling of the Wireshark source code, a static analysis of file and module dependencies was conducted using the "Understand" and "Sourcetrail" tools. 

### Import Map and Subsystem Dependencies
An Architecture Dependency Graph was generated to evaluate the interactions between the three main layers of the application: 
- `UI` (Presentation)
- `EPAN` (Core dissection engine)
- `WSUTIL` (Base utilities)

The quantitative analysis partially confirms a top-down layered architecture, but also exposes architectural violations:
* **Expected Top-Down Flow:** the system exhibits a strong reliance on the foundational layers. 

        `EPAN` heavily depends on `WSUTIL` (869,607 dependencies), confirming `WSUTIL`'s role as the fundamental utility module. 
        Furthermore, the `UI` properly relies on `EPAN` (8,437 dependencies) to trigger dissections and retrieve packet data.

* **Architectural Violations (Design smells):** a strict layered architecture dictates that lower layers never depend on upper ones. 
However, the dependency graph reveals bidirectional coupling: 

        `EPAN` has 61 outgoing dependencies directed toward the `UI` layer
        `WSUTIL` has 2 dependencies pointing back to `EPAN`.
        Altough numerically small compared to the main flow, these represent architectural leaks where the core engine and base utilities 
        are unnecessarily coupled with higher-level abstractions.

<p align="center">
  <img src="./dependency%20graphs/subsystem_butterfly_coupling.png" alt="Understand Dependency Graph showing bidirectional coupling" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: Dependency graph generated via Understand, illustrating the massive expected top-down flow alongside the minor architectural violations (bidirectional red arrows).</em>
</p>

To support the visual analysis, the architectural coupling metrics (Fan-in $Ca$, Fan-out $Ce$, and Martin's Instability $I = \frac{Ce}{Ca + Ce}$) were computed on the macro-module.
The instability index ranges from 0 (maximum stability) to 1 (maximum instability).

| Module / Subsystem | Architectural Role       | Fan-in (Ca) | Fan-out (Ce) | Instability (I) |
| :---               | :---                     | :---        | :---         | :---            |
| `wsutil`           | Base utilities           | 22          | 0            | **0.000**       |
| `epan`             | Core engine & dissectors | 16          | 3            | **0.158**       |
| `ui_qt`            | Presentation layer       | 2           | 9            | **0.818**       |
| `tshark`           | CLI orchestrator         | 0           | 9            | **1.000**       |

**Architectural Validation:** as expected in a healthy top-down architecture, instability metrics polarize the system. Base modules such as `wsutil` and `epan` score near zero (0.000 and 0.158 respectively): their high Fan-in makes them structural pillars whose modification would trigger extensive recompilation
Conversely, the presentation and orchestration layers (`ui_qt`, `tshark`) carry the majority of outgoing dependencies and are therefore "unstable" ($I between 0.818 and 1.000$), confirming their role as high-level orchestrators that can be modified without cascading effects through the rest of the application.

### Top and Bottom Files by Dependency

A static analysis of `#include` directives across the codebase was performed to identify structural pillars (high Fan-in) and the main orchestrators (high Fan-out).

#### Bottom Files (High Fan-out)
These files represent the modules with the highest number of outgoing dependencies. They import the largest number of header files.

| Rank | File Path | Outgoing Dependencies | Layer / Subsystem |
| :--- | :--- | :--- | :--- |
| 1 | `./ui/qt/wireshark_main_window_slots.cpp` | 136 | UI (Presentation) |
| 2 | `./ui/stratoshark/stratoshark_main_window_slots.cpp` | 98 | UI (Presentation) |
| 3 | `./wiretap/file_access.c` | 90 | Wiretap (Capture) |
| 4 | `./ui/qt/lua_debugger/lua_debugger_dialog.cpp` | 88 | UI (Presentation) |
| 5 | `./tshark.c` | 83 | CLI Entry Point |
| 6 | `./strato.c` | 77 | CLI Entry Point |
| 7 | `./ui/qt/main.cpp` | 74 | UI (Presentation) |
| 8 | `./ui/stratoshark/stratoshark_main.cpp` | 69 | UI (Presentation) |
| 9 | `./ui/qt/lua_debugger/lua_debugger_breakpoints.cpp` | 68 | UI (Presentation) |
| 10 | `./ui/qt/wireshark_main_window.cpp` | 66 | UI (Presentation) |
| 11 | `./ui/qt/packet_list.cpp` | 63 | UI (Presentation) |
| 12 | `./ui/qt/main_application.cpp` | 62 | UI (Presentation) |
| 13 | `./epan/epan.c` | 61 | EPAN (Core) |
| 14 | `./ui/stratoshark/stratoshark_main_window.cpp` | 58 | UI (Presentation) |
| 15 | `./sharkd_session.c` | 55 | Daemon Entry Point |

**Architectural Motivation:**  files at the top of  this list are entry points (`tshark.c`, `strato.c`) and UI orchestrators (`wireshark_main_window_slots.cpp`). Sitting at the apex of the execution pyramid, they must interact with the capture engine (`wiretap`), the dissection core (`epan`), configuration subsystems and numerous Qt widgets; a structural requirement that forces extreme Fan-out and classifies them as god modules.


<p align="center">
  <img src="./dependency%20graphs/tshark_dependency_graph.png" alt="Dependency Graph of tshark.c showing high fan-out" width="60%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: SourceTrail dependency graph of <code>tshark.c</code>, illustrating a massive amount of outgoing dependencies typical of an orchestrator file.</em>
</p>

<p align="center">
  <img src="./dependency%20graphs/main_window_dependency_graph.png" alt="Dependency Graph of wireshark_main_window_slots showing high fan-out" width="50%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: SourceTrail dependency graph of <code>wireshark_main_window_slots.cpp</code>.</em>
</p>


#### Top Files (High Fan-in)
These files represent the most imported headers in the entire project. They are the foundational building blocks upon which the rest of the application relies.

| Rank | Header File | Incoming Dependencies | Layer / Subsystem |
| :--- | :--- | :--- | :--- |
| 1 | `"config.h"` | 2373 | Global Configuration |
| 2 | `<epan/packet.h>` | 2022 | EPAN (Core) |
| 3 | `<epan/expert.h>` | 969 | EPAN (Core) |
| 4 | `<wsutil/array.h>` | 757 | WSUTIL (Utilities) |
| 5 | `<epan/prefs.h>` | 744 | EPAN (Core) |
| 6 | `<epan/tfs.h>` | 617 | EPAN (Core) |
| 7 | `<string.h>` | 354 | C Standard Library |
| 8 | `<epan/asn1.h>` | 353 | EPAN (Core) |
| 9 | `<epan/proto_data.h>` | 328 | EPAN (Core) |
| 10 | `<epan/conversation.h>` | 312 | EPAN (Core) |
| 11 | `<glib.h>` | 285 | External Utility (GLib) |
| 12 | `<epan/to_str.h>` | 253 | EPAN (Core) |
| 13 | `<stdlib.h>` | 251 | C Standard Library |
| 14 | `<config.h>` | 251 | Global Configuration |
| 15 | `"packet-tcp.h"` | 247 | Dissector Header |

**Architectural Motivation:** the Fan-in ranking highlights the rigid modularity of Wireshark's dissection engine. `epan/packet.h` is the most critical non-configuration header: it defines the fundamental C structures defining what a "network packet" is and every one of the thousands of independent dissector modules must include it.
A single change to this file would invalidate the compilation cache of over 2,000 files, forcing a near-total rebuild of the project.

<p align="center">
  <img src="./dependency%20graphs/packet_dependency_graph.png" alt="Dependency Graph of packet.h showing high fan-in" width="30%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: SourceTrail dependency graph of <code>packet.h</code> showing massive high fan-in.</em>
</p>

### Logical Coupling and Architectural Anomalies
Beyond static dependencies, an evolutionary analysis was conducted by mining the Git version control history to identify **change coupling** (pairs of modules that frequently change together in the same commits, regardless of explicit structural dependencies). 

To ensure the validity and mathematical soundness of the findings, a **cross-validation approach** was adopted by cross-checking two independent methodologies:
1. An automated behavioral analysis platform (**CodeScene**).
2. A custom Python script (`cochange_analyzer.py`) mining the raw transaction history.

Both tools identified severe coupling, though their different heuristics (raw mathematical frequency vs. time-decayed behavioral analysis) highlighted distinct hotspots:


**1. Cross-layer knowledge leak (Python script)**
The script uncovered a high-risk `shotgun surgery` smell across system boundaries. In approximately 90% of commits that modify `epan/prefs.c`, developers are forced to update the layout implementation in the Qt presentation layer (`ui/qt/layout_preferences_frame.h`). This reveals an implicit knowledge leak: the core engine carries implicit awareness of GUI layout decisions, violating the principle of layer independence.


| File A (Core / Architecture module) | File B (Presentation / Context layer) | Co-change % (Python) | Risk category   |
| :------------------------------     | :------------------------------       | :-----------------   | :-------------- |
| `epan/prefs.c`                      | `ui/qt/layout_preferences_frame.h`    | 85.71%               | shotgun surgery |
| `epan/prefs.h`                      | `ui/qt/layout_preferences_frame.cpp`  | 72.73%               | shotgun surgery |


**2. High-density module coupling (CodeScene)**
CodeScene behavioral engine prioritized modules with a **100% Degree of coupling** and high revision rates. 
As shown in the extracted data, CodeScene highlights an extreme internal dependency within the UI subsystem. 

| Entity (File A)                 | Coupled entity (File B)         | Degree of coupling | Average revisions |
| :------------------------------ | :------------------------------ | :----------------- | :---------------- |
| `ui/qt/packet_list.cpp`         | `ui/qt/packet_list_record.cpp`  | 100%               | 6                 |
| `epan/dissectors/packet-wlan.c` | `epan/dissectors/packet-wlan.h` | 100%               | 8                 |
| `ui/qt/packet_list_model.cpp`   | `ui/qt/packet_list_record.cpp`  | 100%               | 5                 |

This behavioral data proves that changes cascade rapidly through `packet_list` components due to tight logical binding. This architectural bottleneck forces continuous synchronization across UI files, driving up maintenance costs.

### Cross-validation of Static and Evolutionary metrics: The `tshark.c` case
To bridge teh gap between static structural dependencies and evolutionary technical debt, CodeScene REST APIs were utilized to programmatically extract the project's top "hotspots" (`codescene_hotspots.json`). 

This extraction yielded a crucial architectural validation: `tshark.c`, previously identified in the static analysis as a "god module" with extreme Fan-out (83 outgoing dependencies), is mathematically ranked as one of the top 10 worst hotspots in the entire Wireshark codebase. 

According to the API data, `tshark.c` exhibits:
* **Code health score:** 1.68 / 10.0 (Critical Red Zone)
* **Recent revisions:** 87

**Architectural motivation:** This provides empirical proof of how structural design choices directly impact software evolvability. 
The orchestration responsibilities centralized within `tshark.c` force it to be touched continuously during development (87 revisions).
Because of its massive Fan-out, every modification is highly complex and error-prone, plummeting its `code health` to 1.68. 
This perfectly demonstrates how static structural smells (the god module anti-pattern) directly manifest as severe evolutionary technical debt over time.

<p align="center">
  <img src="./dependency%20graphs/codescene_hotspots.png" alt="CodeScene Hotspot Map" width="50%" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.15); margin-bottom: 10px;">
  <br>
  <em>Figure: CodeScene hotspot virtual map displaying code health degradation driven by high change frequency and overlapping logical dependencies.</em>
</p>


## Patterns
In this section is present a list of detected patterns in Wireshark.

### Chain of Responsibility
This pattern gives the possibility to send a request along a chain of potential handlers until one of them handles it. The general idea is to decouple senders and receivers. 

The discussed pattern is used in the `epan` folder of the project to implement the dissectors chain: every dissector analyzes its packet level and pass the payload to the next dissector through dispatch tables without knowing who will handle it. The classic pattern has been adapted from an OOP style to a C architecture based on modules and table.

The **Handler** role is developed by the struct `dissector_handle_t` defined in `epan/packet.c`and by the type function `dissector_t` declared in `epan/packet.h`. The successor link isn’t in the Handler but in the `dissector_table_t`. Every `epan/dissectors/packet-*.c` is a **Concrete Handler** which manages the packet or passes it to the successor calling it by `dissector_try_uint()` or `call_dissector()`. The **Client**, which sends the first request is split into two different moments in Wireshark: chain building with every `proto_reg_handoff_*()` call in `packet-*.c`files and request send with `epan_dissect_run()` in `epan/epan.c` which hands over the execution to the packet processing core in `epan/packet.c`, which bootstraps the entire chain by calling `call_dissector()` on the initial `frame_handle`.

Instead of *Chain of Responsibility*, other solutions could be used:
-	*centralized switch/if-else*: simple to use but only with few protocols;
-	 *Strategy pattern*: simpler conceptually but not capable to manage hierarchical style of protocols;
-	*Observer pattern*: guarantees total decouple but not the processing order adding complexity to debug.
  
Analysing pros and cons, the actual pattern stays the best choice because respects hierarchical and sequential nature of network protocols.

### Strategy 
This pattern defines a family of algorihms, putting each of them into a separate class, and making them interchangeable. This pattern is particularly useful when you have multiple algorithms for a specific task and want to be able to switch between them dynamically. 

The discussed pattern is present in the `wiretap` subsystem which reads and writes capture files in different formats. Since the project uses the C language, the pattern is simulated with structs and function pointers.

The **Strategy** role is fulfilled by the struct `file_type_subtype_info` defined in `wiretap/wtap.h`, which defines the functions protoypes that each file format must implement to interact with Wireshark. Each file designed for a specific format such as `wiretap/json.c` or `wiretap/pcapng.c` represents a **Concrete Strategy** since they contain static constant variables of the struct `file_type_subtype_info`. The **Context** role is principally managed by `wiretap/file_access.c` and encapsulated at runtime into Wireshark’s session structures. This file allows the storage of tables or arrays of all strategies then, when a users open a file, Wireshark searches and identifies the right Concrete Strategy saving the structure pointer into the current session.

Instead of the *Strategy* pattern, other solutions could be used:
-	*monolithic switch or if/else chain*: it eliminates the overhead of function pointer calls but violates of the Open/Closed Principle;
-	*Chain of Responsibility pattern*: the file to be opened is passed through a chain of modules until it is recognized but the centralized control given by the Context is lost.

Analysing the different options, the chosen pattern remains the best choice since it enforces strict modularity and offers a centralized orchestration.

### Observer
This pattern defines an object which maintains a list of dependents and automatically notifies them when its state changes. The *Observer* pattern guarantees the abstract coupling and the respect for Open/Closed Principle.

The discussed pattern is used in the `epan` folder of the project to implement the Tap system. A tap allows other items to see what is happening as a protocol is dissected. Since the project uses the C language, the pattern is developed with an event structure based on callbacks.

The **Subject** role, which is the interface of the observable subject, is defined in `epan/tap.h` through the functions `register_tap`and `tap_queue_packet`which notifies all the Observer registered on that tap. The **Observer** interface is expressed by function pointers defined in `epan/tap.h`. Protocol dissectors in files such as `epan/dissectors/packet-ip.c` act as the **Concrete Subject** role. Each of them obtains a tap handle via `register_tap` and then, after packet analysis, invokes `tap_queue_packet`. Every statistic module that calls `register_tap_listener` in `epan/tap.h` is a **Concrete Observer**.

Instead of the *Observer* pattern, other strategies could be used:

-	*Mediator pattern*: it is useful when objects have to cooperate in complex ways, avoiding the creation of a chaotic net of notifications. However, in this context it would introduce an unnecessary level of indirection;
-	*Polling*: it is very simple to implement and relies on each component periodically checking the data source for updates but in this way, packets may be processed with a delay. Furthermore, the Tap system is synchronous: listeners and dissectors process data simultaneously, a guarantee that *Polling* would break.
  
Analysing the different options, the chosen pattern remains the best choice because it’s more flexible than the *Mediator* and more efficient than *Polling*.

### Abstract Factory
The Abstract Factory pattern provides an interface for creating families of related objects without specifying their concrete classes. In Wireshark it decouples the core engine from thousands of specific protocol implementations. Without it, the core would require a massive hardcoded list of all protocols, making maintenance rigid and preventing the addition of external plugins.

The *Abstract Factory* role is implemented by the `proto_register_protocol()` function in epan/proto.c, which defines the contract for system integration. The *Concrete Factory* role is played by individual dissector files (in epan/dissectors/) that "produce" and hand over their protocol definitions to the core. Finally, the *Product* is the `protocol_t` structure, representing the registered protocol within the engine.

Instead of the Abstract Factory, other solutions could include:
- *Builder Pattern*: It would allow step-by-step protocol configuration. However, it adds significant verbosity and memory overhead, making the startup process less efficient than a single function call.
- *Prototype Pattern*: Dissectors would be cloned from a pre-configured `protocol_t` base object and customized only where needed. However, deep copying in C is complex and risky, requiring meticulous pointer management to prevent different protocols from accidentally sharing the same memory area.

### Composite Pattern
The Composite pattern allows clients to treat individual objects and compositions uniformly via tree structures. In Wireshark this is essential for representing decoded packet data hierarchically, as network packets are inherently nested. Without it, handling entire protocols versus individual fields would require complex, separate logic, making navigation and UI rendering highly inefficient.

The *Component* role is fulfilled by the base `proto_node` (in epan/proto.h), which establishes the common interface and the pointers necessary for parent-child relationships between nodes. Its typedef aliases define the other roles: `proto_tree` acts as the *Composite* for nested elements, while `proto_item` acts as the *Leaf* for terminal, childless fields. This uniform interface allows dissectors to recursively add elements via functions like `proto_tree_add_item()` without distinguishing between the main tree and deep subtrees.

Instead of the Composite pattern, other solutions could be:
- *Iterator Pattern*: Accessing fields via a sequential iterator object would decouple UI algorithms from the physical data structure. However this is purely an access pattern. On its own, it fails to solve how to physically represent the hierarchy and nesting of protocols in memory.
- *Flyweight Pattern*: Sharing a common "intrinsic state" for repetitive fields would drastically reduce RAM consumption and allocation overhead during prolonged captures. Nevertheless it significantly increases architectural complexity in C and introduces indirect data access, causing computational overhead incompatible with real-time requirements.

### Singleton Pattern
The Singleton pattern ensures that there is only one instance of a given data structure and provides a global access point to it. In Wireshark it crucially manages dissector tables, which act as a “switchboard” to route packets to the correct protocol. Without it, modules might create separate tables, destroying the Single Source of Truth and causing packet routing to fail irreversibly.

Since Wireshark is written in C, the pattern is adapted to work around the lack of classes and private constructors. The *Singleton* role is achieved by hiding the data: a pointer to a global hash table is declared in epan/packet.c and marked with the keyword `static` (e.g., `static GHashTable *dissector_tables = NULL;`), preventing accidental clones. The *Global Access Point* is provided by public functions like `find_dissector_table()`, ensuring all dissectors safely interact with the exact same memory instance.

Instead of the Singleton pattern, other solutions could include:
- *Proxy Pattern*: An intermediary proxy could either defer loading the table into memory until the first request (Virtual Proxy), or act as a security layer restricting write-access to prevent external plugins from corrupting the routing rules (Protection Proxy). However, since routing occurs per captured packet, evaluating a proxy layer on every read operation introduces computational overhead incompatible with real-time dissection.
- *Facade Pattern*: A global Facade could hide the entire complex group of routing interfaces, offering dissectors a unified high-level access point. The problem is that in a layered system, wrapping an entire functional area risks creating a massive bottleneck (a God Object). The current Singleton, on the other hand, is specifically limited to ensuring the uniqueness of the resource, keeping the architecture leaner and more modular.

### Summary
The analyzed design patterns directly explain specific architectural dependencies and structural metrics. 

The **Chain of Responsibility** accounts for the exceptionally high Fan-in of epan/packet.h, as every *Concrete Handler* must include this header to participate in the dissection chain. 
Conversely, the **Strategy** pattern explains the high Fan-out of `wiretap/file_access.c`, which acts as a *Context* that must import all *Concrete Strategies* to orchestrate file format selection.

The near-total stability of the core engine (*I = 0.158*) is actively guaranteed by the **Abstract Factory** pattern: `proto_register_protocol()` inverts dependencies, enabling external dissectors to register autonomously without forcing the core to continuously update its dependencies. Furthermore, the **Singleton** pattern applied to dissector tables establishes a strict Single Source of Truth within the EPAN layer, preventing the chaotic proliferation of routing tables and contributing to the core's structural solidity.

However, the lack of appropriate patterns in other areas exposes severe evolutionary debt. The 100% change coupling detected among `packet_list` files highlights the absence of a proper structural pattern (such as an intermediate Model-View-ViewModel abstraction) to decouple the UI from the **Composite** tree generated by the core. Similarly, the shotgun surgery detected between `epan/prefs.c` and the Qt interface demonstrates a failure to use patterns like the **Observer** to decouple state changes, directly causing the architectural knowledge leaks identified in the evolutionary analysis.

