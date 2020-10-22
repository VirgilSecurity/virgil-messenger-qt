#   Copyright (C) 2015-2020 Virgil Security Inc.
#
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are
#   met:
#
#       (1) Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#       (2) Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#       (3) Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived from
#       this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.
#
#   Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

if(VS_PLATFORM STREQUAL "android")

  # Create "apk" and "aab" targets

  find_program(ANDROID_DEPLOY_QT androiddeployqt)
  
  if(DEFINED ENV{JAVA_HOME})
    set(JAVA_HOME $ENV{JAVA_HOME} CACHE INTERNAL "Saved JAVA_HOME variable")
  elseif(EXISTS "/etc/alternatives/java_sdk")
    set(JAVA_HOME "/etc/alternatives/java_sdk" CACHE INTERNAL "Saved JAVA_HOME variable")
  endif()
  
  if(JAVA_HOME)
    set(android_deploy_qt_jdk "--jdk ${JAVA_HOME}")
    message(STATUS "Java home: [${android_deploy_qt_jdk}]")
  endif()

  if (ANDROID_SDK_PLATFORM)
    set(android_deploy_qt_platform "--android-platform ${ANDROID_SDK_PLATFORM}")
    message(STATUS "Android deploy QT platform: [${ANDROID_SDK_PLATFORM}]")
  endif()
    
  set(ANDROID_SDK_BUILD_TOOLS_VERSION "29.0.2")

  if(VS_KEYCHAIN_PASSWORD)
    set(ANDROID_DEPLOY_QT_PARAMS "--storepass '${VS_KEYCHAIN_PASSWORD}'")
  endif()
  
  if(VS_KEY_PASSWORD)
    set(ANDROID_DEPLOY_QT_PARAMS "${ANDROID_DEPLOY_QT_PARAMS} --keypass '${VS_KEY_PASSWORD}'")  
  endif()
  
  add_custom_target(apk_release
    COMMAND ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME} ${ANDROID_DEPLOY_QT}
       --input "${CMAKE_BINARY_DIR}/android_deployment_settings.json"
       --output "${CMAKE_BINARY_DIR}/android-build"
       --apk "${CMAKE_BINARY_DIR}/android-build/${PROJECT_NAME}.apk"
       ${android_deploy_qt_platform}
       ${android_deploy_qt_jdk}
       --gradle
       --sign "${VS_KEYCHAIN}"
       ${ANDROID_DEPLOY_QT_PARAMS}      
       --no-gdbserver      
    VERBATIM)

  add_custom_target(aab_release
    COMMAND ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME} ${ANDROID_DEPLOY_QT}
      --input "${CMAKE_BINARY_DIR}/android_deployment_settings.json"
      --output "${CMAKE_BINARY_DIR}/android-build"
      --apk "${CMAKE_BINARY_DIR}/android-build/${PROJECT_NAME}.apk"
      --aab
      ${android_deploy_qt_platform}
      ${android_deploy_qt_jdk}
      --gradle
      --sign "${VS_KEYCHAIN}"
      ${ANDROID_DEPLOY_QT_PARAMS}      
      --no-gdbserver      
   VERBATIM)

endif()