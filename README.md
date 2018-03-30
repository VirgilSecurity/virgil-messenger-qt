# Virgil Security Simple client/server example for *nix platforms

# Quickstart

## Motivation
This example consists of client and server parts which are using Virgil Security
stack functionality:

-   encrypt, decrypt, sign and verify data;
-   interact with Virgil Keys Service;

Both parts can create their own key pair and Virgil Cards.
Virgil Cards are pushed to Virgil Key Service.
And private keys are stored locally in a file system.

## Unix Build

### Toolchain
*   [CMake](http://www.cmake.org/) (accessible in command prompt). Minimum version: 3.2.
*   [Git](http://git-scm.com/) (accessible in command prompt).
*   [libcurl](http://curl.haxx.se/libcurl/).

### Build steps
1.   Open terminal.

1.   Clone the project.

        git clone https://github.com/VirgilSecurity/virgil-nix-client-server-example.git

1.   Go to the project's folder.

        cd virgil-nix-client-server-example

1.   Create a folder for the build purposes and go to it.

        mkdir build && cd build

1.   Configure and build.

        cmake .. && make


## Start server

        virgil_server <port> <email> <private key directory>

- port - any port in the range [2000 .. 65535]
- email - this parameter is used as an identity. It can contain any fictive value (for the current example).
- private key directory - directory where the private key will be stored.

Example:
        ./virgil_server 3000 server@test.com ${HOME}/

## Start client

        virgil_client <host name> <port> <email> <private key directory>

- host name - the name of the host or IP address
- server's port
- email - this parameter is used as an identity. It can contain any fictive value (for the current example).
- private key directory - directory where the private key will be stored.

Example:
        ./virgil_client 192.168.0.101 3000 client@test.com ${HOME}/

## License
BSD 3-Clause. See [LICENSE](https://github.com/VirgilSecurity/virgil-cli/blob/master/LICENSE) for details.


## Contacts

Our developer support team is here to help you. Find out more information on our [Help Center](https://help.virgilsecurity.com/).

You can find us on [Twitter](https://twitter.com/VirgilSecurity) or send us email support@VirgilSecurity.com.

Also, get extra help from our support team on [Slack](https://virgilsecurity.slack.com/join/shared_invite/enQtMjg4MDE4ODM3ODA4LTc2OWQwOTQ3YjNhNTQ0ZjJiZDc2NjkzYjYxNTI0YzhmNTY2ZDliMGJjYWQ5YmZiOGU5ZWEzNmJiMWZhYWVmYTM).
