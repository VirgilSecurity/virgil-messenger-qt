# Qt/QML Demo for Virgil IoTKIT Usage
<a href="https://developer.virgilsecurity.com/docs"><img width="230px" src="https://cdn.virgilsecurity.com/assets/images/github/logos/iotkit/IoTKit.png" align="left" hspace="1" vspace="3"></a>

## Introduction
[Virgil Security](https://virgilsecurity.com) provides a set of APIs for adding security to any application or IoT device.

[Virgil IoTKit](https://github.com/virgilsecurity/virgil-iotkit/) is a C library for connecting IoT devices to the Virgil IoT Security Platform. IoTKit helps you easily add security to your IoT devices at any lifecycle stage for securely provisioning and authenticating devices, updating firmware and TrustLists, and exchanging messages using any transport protocols.

Demo for Virgil IoTKIT is the Virgil IoTKIT usage example based on C++ and Qt/QML library.

## Prerequisites
To compile Demo for Virgil IoTKIT Qt  the following components are required:
- C99 for Virgil IoTKIT.
- C++14 for this demo.
- CMake v3.11 or higher for Virgil IoTKIT building
- GCC or another toolchain for C/C++ compile
- QtCreator for demo building
- Android SDK, Android NDK for Android build. Be sure that `ANDROID_SDK` and `ANDROID_NDK` environment variables with correct paths
are setup.

## Usage
- Clone Demo IoTKIT for QT repository:
```bash
git clone --recurse-submodules https://github.com/VirgilSecurity/demo-iotkit-qt
```
- Build needed platform. Use ext/virgil-iotkit/scripts/build-for-qt.sh script for this goal. You can run it without parameters
to see supported platforms and options. For example: `build-for-qt.sh linux`, `build-for-qt.sh android armeabi-v7a` etc.
- Open demo-iotkit-qt.pro project in QtCreator.

This application show active Virgil IoT devices. You can clone [demo-iotkit-nix](https://github.com/VirgilSecurity/demo-iotkit-nix/)
project and start some of them to test this software. To see them start those devices and your application in the same local network.

## License

This demo and Virgil IoTKIT is released under the [3-clause BSD License](LICENSE).

<div id='support'/>

## Support
Our developer support team is here to help you. Find more information on our [Help Center](https://help.virgilsecurity.com/).

You can find us on [Twitter](https://twitter.com/VirgilSecurity) or send us an email at support@VirgilSecurity.com.

Also, get extra help from our support team on [Slack](https://virgilsecurity.com/join-community).
