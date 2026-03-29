# CipherCore 🛡️
**A Decentralized, End-to-End Encrypted Terminal Network**

CipherCore is a high-performance, multithreaded peer-to-peer chat architecture engineered from scratch in **C**. It features hardware-level cryptographic acceleration via **x86 Assembly** and implements raw TCP socket routing.

### ⚙️ Core Engineering Features
* **Zero-Trust Network:** Host-side `/accept` and `/deny` socket handshake validation.
* **Diffie-Hellman Key Exchange:** Secure, mathematical session key generation over public LAN/WAN networks.
* **Persistent Hex Accumulator:** Custom stream-parsing algorithm making the network immune to TCP packet fragmentation over services like Pinggy.
* **LSB Steganography:** Bitwise manipulation engine to covertly embed server data inside `.bmp` image metadata.
* **Asynchronous I/O:** Multithreaded architecture allowing simultaneous file downloads (`/send`), UI rendering, and raw keystroke capture.

*Built for the System Programming curriculum at NIT Delhi.*
