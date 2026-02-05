# 🧠 Royale Delivery Server Project
The Royale Delivery Server project is a comprehensive mail server application designed to handle email-related operations, including client connections, mail storage, and delivery. This project utilizes the Qt framework and C++ to provide a robust and efficient mail server solution. The core features of this project include a TCP server for handling client connections, an SMTP server for handling email delivery, and a mail handler for storing and retrieving emails.

## 🚀 Features
* **TCP Server**: Handles client connections and manages client data
* **SMTP Server**: Handles email delivery and interacts with the mail handler
* **Mail Handler**: Stores and retrieves emails, and provides functions for client authentication and IP address management
* **Client Handler**: Manages client data, including passwords, IP addresses, and mail storage
* **SSL/TLS Encryption**: Provides secure connections for clients
* **Password Hashing**: Utilizes Qt's QCryptographicHash class to hash passwords for security purposes

## 🛠️ Tech Stack
* **Qt Framework**: Utilized for TCP server, SMTP server, and GUI functionality
* **C++**: Used as the primary programming language
* **QSslSocket**: Used for SSL/TLS encryption
* **QTcpServer**: Used for TCP server functionality
* **QTcpSocket**: Used for TCP socket functionality
* **QCryptographicHash**: Used for password hashing
* **CMake**: Used as the build system

## 📦 Installation
To install the Royale Delivery Server project, follow these steps:
1. Clone the repository using Git
2. Install the required dependencies, including Qt and CMake
3. Configure the build system using CMake
4. Build the project using the configured build system
5. Install the built executable

## 💻 Usage
To use the Royale Delivery Server project, follow these steps:
1. Run the executable to start the TCP server and SMTP server
2. Connect to the TCP server using a client application
3. Send emails using the SMTP server
4. Retrieve emails using the mail handler

## 📂 Project Structure
```markdown
Royale Delivery Server Project
├── source
│   ├── main.cpp
│   ├── net
│   │   ├── Server.cpp
│   │   ├── SMTPServer.cpp
│   ├── storage
│   │   ├── ClientHandler.cpp
│   │   ├── MailHandler.cpp
├── headers
│   ├── Server.h
│   ├── SMTPServer.h
│   ├── SslServer.h
│   ├── Utilities.h
├── CMakeLists.txt
```

## 📸 Screenshots
