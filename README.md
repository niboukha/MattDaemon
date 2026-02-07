# Matt_daemon

A Unix daemon server that runs in the background and accepts TCP client connections for remote command execution. The project demonstrates advanced system programming concepts including daemonization, signal handling, multi-client networking, authentication, and comprehensive logging with rotation.

## 📋 Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Technical Details](#technical-details)
- [Security](#security)
- [Logging](#logging)

---

## 🎯 Overview

Matt_daemon is a background daemon service that:
- **Daemonizes** itself (detaches from terminal, runs in background)
- **Listens** on TCP port 4242 for client connections
- **Authenticates** clients with username/password
- **Executes** commands remotely and returns results
- **Logs** all activity with automatic log rotation
- **Handles** signals gracefully for clean shutdown
- **Supports** up to 3 concurrent clients
- **Includes** a Qt-based GUI client for easy interaction

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│   Daemon Process (Background)               │
│   PID: /var/lock/matt_daemon.lock           │
│                                             │
│   ┌──────────────────────────────────────┐ │
│   │  Server Object (Port 4242)           │ │
│   │  ┌────────────────────────────────┐  │ │
│   │  │  Client Manager (Max 3)        │  │ │
│   │  │  - Authentication              │  │ │
│   │  │  - Command Execution           │  │ │
│   │  │  - Response Handling           │  │ │
│   │  └────────────────────────────────┘  │ │
│   │                                      │ │
│   │  ┌────────────────────────────────┐  │ │
│   │  │  CommandExecutor               │  │ │
│   │  │  - help, pwd, whoami           │  │ │
│   │  │  - ls, echo                    │  │ │
│   │  │  - rm (monitored/blocked)      │  │ │
│   │  └────────────────────────────────┘  │ │
│   │                                      │ │
│   │  ┌────────────────────────────────┐  │ │
│   │  │  Tintin_reporter               │  │ │
│   │  │  - System logs                 │  │ │
│   │  │  - Audit logs                  │  │ │
│   │  │  - Command logs                │  │ │
│   │  │  - Log rotation                │  │ │
│   │  └────────────────────────────────┘  │ │
│   └──────────────────────────────────────┘ │
│                                             │
└─────────────────────────────────────────────┘
              ▲
              │ TCP Socket Connection
              │
    ┌─────────┴──────────┐
    │                    │
┌───▼────┐      ┌────────▼────┐
│ Client │      │  GUI Client │
│ (CLI)  │      │  (Qt5)      │
└────────┘      └─────────────┘
```

---

## ✨ Features

### Core Functionality
- **Full Daemonization**: Double-fork technique with session leadership
- **Lock File Management**: Prevents multiple daemon instances (`/var/lock/matt_daemon.lock`)
- **TCP Server**: Non-blocking server on port 4242 using `select()`
- **Multi-Client Support**: Handles up to 3 simultaneous authenticated clients
- **Authentication System**: Username/password validation before command execution
- **Signal Handling**: Graceful shutdown on SIGTERM, SIGINT, and 26 other signals

### Command Execution
Available commands:
- `help` - List available commands
- `pwd` - Show current directory
- `whoami` - Display current user
- `ls` - List directory contents
- `echo <text>` - Echo back text
- `rm <file>` - Monitored command (triggers email alert)
- `quit` - Disconnect client

### Security Features
- **Root Requirement**: Must run as root (enforced at startup)
- **Single Instance Lock**: Prevents multiple daemons
- **Authentication**: Required before command execution
- **Failed Login Alerts**: Email notifications for suspicious activity
- **Monitored Commands**: Alerts on dangerous operations (e.g., file deletion)

### Logging System
- **Multi-Category Logs**:
  - Main log: `/var/log/matt_daemon/matt_daemon.log`
  - System events
  - Audit trail (authentication)
  - Command history
- **Automatic Rotation**: Size-based log rotation
- **Configurable Retention**: Keep N rotated logs
- **Optional Compression**: Gzip support for archived logs

### GUI Client
- Qt5-based graphical interface
- Real-time command execution
- Connection status indicators
- Integrated authentication flow

---

## 📦 Requirements

### System Requirements
- **OS**: Linux/Unix (macOS supported)
- **Architecture**: x86_64
- **Privileges**: Root access required

### Build Dependencies
```bash
# Compiler
g++ with C++11 support

# Libraries (for GUI only)
Qt5 (QtWidgets, QtGui, QtCore, QtNetwork)
```

### Runtime Requirements
```bash
# Log directory (created automatically)
/var/log/matt_daemon/

# Lock file directory (must be writable)
/var/lock/
```

---

## 🔧 Installation

### 1. Clone the Repository
```bash
git clone <repository-url>
cd MattDaemon
```

### 2. Build the Daemon
```bash
make
```

This creates the `Matt_daemon` executable.

### 3. Build the GUI Client (Optional)
```bash
make gui
```

This creates the `Matt_daemon_gui` executable.

### 4. Build Everything
```bash
make all gui
```

---

## 🚀 Usage

### Starting the Daemon

**Must be run as root:**
```bash
sudo ./Matt_daemon
```

The daemon will:
1. Create lock file at `/var/lock/matt_daemon.lock`
2. Daemonize itself (detach from terminal)
3. Write its PID to the lock file
4. Start listening on port 4242
5. Log startup to `/var/log/matt_daemon/matt_daemon.log`

### Checking Daemon Status
```bash
# Check if running
ps aux | grep Matt_daemon

# View PID
cat /var/lock/matt_daemon.lock

# View logs
sudo tail -f /var/log/matt_daemon/matt_daemon.log
```

### Connecting with CLI Client

**Using netcat:**
```bash
nc localhost 4242
```

**Using telnet:**
```bash
telnet localhost 4242
```

**Connection flow:**
1. Server prompts: `USERNAME:`
2. Enter: `admin`
3. Server prompts: `PASSWORD:`
4. Enter: `1234`
5. Upon success: `Matt-daemon>` prompt appears
6. Type commands (e.g., `ls`, `pwd`)
7. Type `quit` to disconnect

### Using the GUI Client
```bash
./Matt_daemon_gui
```

1. Click "Connect" button
2. Enter credentials in the GUI
3. Execute commands via input field
4. Click "Disconnect" or close window when done

### Stopping the Daemon

**Manual cleanup:**
```bash
# Kill the daemon
sudo pkill Matt_daemon

# Remove lock file
sudo rm -f /var/lock/matt_daemon.lock
```

**Using Makefile target:**
```bash
make daemon-clean
```

**Graceful shutdown (via signal):**
```bash
sudo kill -SIGTERM $(cat /var/lock/matt_daemon.lock)
```

---

## 📁 Project Structure

```
MattDaemon/
├── main.cpp                      # Entry point (validates root, starts daemon)
├── GuiMain.cpp                   # GUI application entry point
├── Makefile                      # Build configuration
├── README.md                     # This file
│
├── daemon/                       # Daemon core
│   ├── Daemon.hpp               # Daemon class interface
│   └── Daemon.cpp               # Daemonization, signal handling, main loop
│
├── network/                      # Networking layer
│   ├── Server.hpp               # Server class interface
│   ├── Server.cpp               # Socket server, client management
│   ├── Client.h                 # Client class interface
│   └── Client.cpp               # Per-client state and authentication
│
├── CommandExecutor/              # Command execution
│   ├── CommandExecutor.h        # Command executor interface
│   └── CommandExecutor.cpp      # Command parsing and execution
│
├── util/                         # Utilities
│   ├── Tintin_reporter.hpp      # Logging system interface
│   └── Tintin_reporter.cpp      # Multi-category logging with rotation
│
├── mailService/                  # Email notifications
│   ├── mailService.h            # Mail service interface
│   └── mailService.cpp          # Email alert functionality
│
├── Gui/                          # Qt GUI client
│   ├── Gui.h                    # GUI class interface
│   └── Gui.cpp                  # GUI implementation
│
└── obj/                          # Build artifacts (auto-generated)
```

---

## 🔬 Technical Details

### Daemonization Process

The daemon follows the standard Unix double-fork daemonization:

1. **First fork()**: Child continues, parent exits (detach from shell)
2. **setsid()**: Child becomes session leader (detach from controlling terminal)
3. **Second fork()**: Grandchild continues, child exits (prevent terminal reacquisition)
4. **umask(0)**: Reset file creation mask
5. **chdir("/")**: Change to root directory (prevent unmount issues)
6. **Close std file descriptors**: Close STDIN, STDOUT, STDERR
7. **Redirect to /dev/null**: Prevent accidental I/O

### Network Architecture

- **Protocol**: TCP
- **Port**: 4242
- **Concurrency Model**: Single-threaded with `select()` multiplexing
- **Max Clients**: 3 simultaneous connections
- **Timeout**: None (persistent connections)

### Signal Handling

27 signals are handled for graceful shutdown:
```
SIGINT, SIGTERM, SIGHUP, SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
SIGFPE, SIGBUS, SIGSEGV, SIGSYS, SIGPIPE, SIGALRM, SIGURG,
SIGTSTP, SIGCONT, SIGCHLD, SIGTTIN, SIGTTOU, SIGIO, SIGXCPU,
SIGXFSZ, SIGVTALRM, SIGPROF, SIGWINCH, SIGUSR1, SIGUSR2
```

All trigger cleanup and graceful shutdown.

### Authentication

**Default credentials:**
- Username: `admin`
- Password: `1234`

*Note: These are hardcoded for demonstration. In production, use secure credential storage.*

### Command Execution Flow

```
Client sends "ls\n"
    ↓
Server receives data → buffer
    ↓
Parse complete line (newline-delimited)
    ↓
Check client authentication state
    ↓
CommandExecutor::executeCommand()
    ↓
Match command type (ls, pwd, etc.)
    ↓
Execute (popen, getcwd, getlogin, etc.)
    ↓
Return result string
    ↓
Send response back to client socket
    ↓
Append "Matt-daemon> " prompt
    ↓
Log command execution
```

---

## 🔐 Security

### Current Security Measures
- Root requirement enforcement
- Lock file prevents multiple instances
- Authentication before command execution
- Email alerts for suspicious activity
- Limited command set (no arbitrary shell execution)

### Security Limitations (Educational Project)
⚠️ **This is a learning project. Do NOT use in production without addressing:**
- Hardcoded credentials
- No encryption (plaintext communication)
- No TLS/SSL support
- Limited command sandboxing
- Email credentials in source code

---

## 📝 Logging

### Log Files

All logs are stored in `/var/log/matt_daemon/`:

```bash
/var/log/matt_daemon/
├── matt_daemon.log           # Main log file
├── matt_daemon_system.log    # System events (startup, shutdown)
├── matt_daemon_audit.log     # Authentication attempts
└── matt_daemon_commands.log  # Command execution history
```

### Log Rotation

Logs automatically rotate when they exceed configured size:
- **Rotated logs**: Named with timestamp (e.g., `matt_daemon.log.2026-02-07_14-30-00`)
- **Retention**: Configurable number of old logs kept
- **Compression**: Optional gzip compression of archives

### Log Format
```
[TIMESTAMP] [LEVEL] Message
```

Example:
```
[2026-02-07 14:30:15] [INFO] Matt_daemon: Started.
[2026-02-07 14:30:15] [INFO] Matt_daemon: started. PID: 12345
[2026-02-07 14:30:20] [INFO] Matt_daemon: Client #4 authenticated successfully
[2026-02-07 14:30:25] [LOG] Matt_daemon: Client #4 executed command: ls
```

---

## 🛠️ Makefile Targets

```bash
make              # Build daemon only
make gui          # Build GUI client only
make all          # Build daemon (default)
make clean        # Remove object files
make fclean       # Remove objects and executables
make re           # Clean rebuild
make daemon-clean # Stop daemon and remove lock file
```

---

## 📧 Email Alerts

The `MailService` component sends email notifications for:
- Multiple failed authentication attempts
- Attempted file deletion commands (`rm`)

**Configuration**: Edit `mailService.h`:
```cpp
#define EMAIL_SENDER    "your-email@gmail.com"
#define EMAIL_RECEIVER  "alert-email@gmail.com"
#define APP_PASSWORD    "your-app-password"
```

**Note**: Uses Gmail SMTP. Requires an [App Password](https://support.google.com/accounts/answer/185833).

---

## 🐛 Debugging

### Common Issues

**"Can't open /var/lock/matt_daemon.lock"**
- Daemon already running
- Solution: `make daemon-clean`

**"This program must be run as root"**
- Insufficient privileges
- Solution: `sudo ./Matt_daemon`

**Connection refused**
- Daemon not running
- Solution: Check logs, restart daemon

**GUI won't connect**
- Firewall blocking port 4242
- Daemon crashed
- Solution: Check daemon status and logs

### Viewing Logs
```bash
# Tail main log
sudo tail -f /var/log/matt_daemon/matt_daemon.log

# View all logs
sudo ls -la /var/log/matt_daemon/

# Check for errors
sudo grep ERROR /var/log/matt_daemon/matt_daemon.log
```

---

## 🎓 Learning Objectives

This project demonstrates:
- Unix daemon creation and management
- TCP server implementation with `select()`
- Signal handling and graceful shutdown
- Process management (fork, setsid, PID files)
- File locking mechanisms
- Multi-client state management
- Logging systems with rotation
- Qt GUI development
- Client-server architecture

---

## 📄 License

Educational project - use at your own risk.

---

## 👥 Authors

Developed as a system programming exercise.

---

## 🔗 References

- [Linux Daemon Writing HOWTO](https://www.netzmafia.de/skripten/unix/linux-daemon-howto.html)
- [Unix Network Programming](http://www.unpbook.com/)
- [Advanced Programming in the UNIX Environment](https://www.apuebook.com/)

---

**Happy Daemonizing! 🚀**
