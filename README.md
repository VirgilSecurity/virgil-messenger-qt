# Qt/QML Demo for Virgil IoTKIT Usage
<a href="https://developer.virgilsecurity.com/docs"><img width="230px" src="https://cdn.virgilsecurity.com/assets/images/github/logos/iotkit/IoTKit.png" align="left" hspace="1" vspace="3"></a>

## Introduction
[Virgil Security](https://virgilsecurity.com) provides a set of APIs for adding security to any application or IoT device.

[Virgil IoTKit](https://github.com/virgilsecurity/virgil-iotkit/) is a C library for connecting IoT devices to the Virgil IoT Security Platform. IoTKit helps you easily add security to your IoT devices at any lifecycle stage for securely provisioning and authenticating devices, updating firmware and TrustLists, and exchanging messages using any transport protocols.

Demo for Virgil IoTKIT is the Virgil IoTKIT usage example based on C++ and Qt/QML library.

## Prerequisites
To compile Demo for Virgil IoTKIT the following components are required:
- C99 for Virgil IoTKIT.
- C++14 for this demo.
- CMake v3.11 or higher, for project building
- GCC or another toolchain for C/C++ compile
- Qt for QML usage.

Virgil IoTKIT provides `cmake/android/qt-android-mk-apk.cmake` CMake file. It allows you to deploy application for Android
by using `androiddeployqt` tool provided by Qt library. To use it you need to install Android SDK, NDK and Qt with Android components.

## Android Build
`deploy_android.sh` script builds, deploys and starts demo application on Android device. There are several constants that are described
in this script.

Connect your Android device and follow steps listed below :
```bash
git clone --recurse-submodules -b feature/android-avn https://github.com/VirgilSecurity/demo-iotkit-qt
cd demo-iotkit-qt
./deploy_android.sh
```

## License

This demo and Virgil IoTKIT is released under the [3-clause BSD License](LICENSE).

<div id='support'/>

## Support
Our developer support team is here to help you. Find more information on our [Help Center](https://help.virgilsecurity.com/).

You can find us on [Twitter](https://twitter.com/VirgilSecurity) or send us an email at support@VirgilSecurity.com.

Also, get extra help from our support team on [Slack](https://virgilsecurity.com/join-community).
