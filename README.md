# Linux Chat

A secure, GTK-based chat application for Linux with user-to-user messaging and group chat capabilities.

## Features

- **Secure Communication**: End-to-end encryption using AES-256
- **User Management**: Registration, login, and user discovery
- **Messaging**: Direct messaging between users
- **Group Chats**: Create, join, and manage group conversations
- **Modern UI**: Clean GTK3-based user interface
- **Cross-Platform**: Supports both x64 and ARM64 architectures

## Prerequisites

To build and run Linux Chat, you'll need:

- C compiler (GCC or Clang)
- CMake (3.10 or newer)
- GTK3 development libraries
- OpenSSL development libraries
- pthread support

### Installing Dependencies on Ubuntu/Debian

```bash
sudo apt install build-essential cmake libgtk-3-dev libssl-dev
```

### For Cross-Compiling to ARM64

```bash
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

## Building

### Standard Build

```bash
# Clone the repository
git clone https://github.com/duynguyen/linux-chat.git](https://github.com/nvanquyet/linux-chat
cd linux-chat

# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
make
```

### Debug Build

```bash
cmake -DBUILD_DEBUG=ON ..
make
```

### Cross-Compile for ARM64

```bash
cmake -DBUILD_ARM64=ON ..
make
```

### Build with Tests

```bash
cmake -DBUILD_TESTS=ON ..
make
make test
```

## Running

After building, the executable will be available in the `build/bin` directory:

```bash
./build/bin/chat_app
```

## Architecture

Linux Chat is organized in the following structure:

- **network**: Handles networking, sessions, and message exchange
- **models**: User, group, and message data models
- **ui**: GTK-based user interface components
- **utils**: Encryption, logging, and utility functions
- **manager**: Request handling and management

## Network Protocol

The application uses a custom binary protocol with the following message types:

- Login/Register/Logout messages
- User-to-user messages
- Group management messages
- Chat history requests and responses

Messages are encrypted using AES-256 with a key exchange protocol for secure communication.

## Configuration

The default server address is `localhost`. To use a different server:

```bash
./build/bin/chat_app --server serveraddress --port portnumber
```

## Contributing

We welcome contributions!  
If you'd like to add features, fix bugs, or improve the code:

1. Fork this repository.
2. Create a new branch: `git checkout -b feature-name`.
3. Commit your changes: `git commit -m "Add feature XYZ"`.
4. Push to your fork.
5. Open a Pull Request.

Please follow the project's coding style and guidelines if available.


## Contact

For questions, feedback, or bug reports, feel free to reach out:

- 📧 Email: nguyenducduypc160903@gmail.com
- 🐙 GitHub: [@duynguyen](https://github.com/N-D-Duy)
