# Copyright (C) 2016 Virgil Security Inc.
#
# Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     (1) Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#     (2) Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#
#     (3) Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from
#     this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=virgil-example
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/virgil-example
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Virgil simple client/server application
  DEPENDS:=+libcurl +libstdcpp +libpthread +ca-certificates
  URL:=https://virgilsecurity.com/
  MAINTAINER:=Virgil Security Support <support@virgilsecurity.com>
endef


define Package/virgil-example/description
 This package contains the command line interface to virgil security library.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR);								\
	cd $(PKG_BUILD_DIR); 									\
	rm -rf virgil-cli; 									\
	git clone "https://github.com/VirgilSecurity/virgil-nix-client-server-example.git"; 	\
	cd virgil-nix-client-server-example;							\
	rm -rf .git;										\
	mv -f ./* ../
endef

define Build/Configure
	cd $(PKG_BUILD_DIR);								\
	mkdir build; cd build;								\
	echo "PREPARE TOOLCHAIN FILE ...";						\
	echo "set(TOOLCHAIN_DIR $(TOOLCHAIN_DIR))" > "$(PKG_BUILD_DIR)/build/toolchain.cmake";							\
	echo "set(TARGET_CROSS $(TARGET_CROSS))" >> "$(PKG_BUILD_DIR)/build/toolchain.cmake";							\
	echo "set(VIRGIL_PATCH_DIR $(PKG_BUILD_DIR)/patches)" >> "$(PKG_BUILD_DIR)/build/toolchain.cmake";					\
	echo "set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $(CURDIR)/../../../package/utils/virgil-example/libs)" >> "$(PKG_BUILD_DIR)/build/toolchain.cmake";\
	echo "set(STAGING_DIR $(STAGING_DIR))" >> "$(PKG_BUILD_DIR)/build/toolchain.cmake";							\
	echo "" >> "$(PKG_BUILD_DIR)/build/toolchain.cmake";											\
	cat  "$(CURDIR)/../../../package/utils/virgil-example/virgil-openwrt.toolchain.cmake.in" >> "$(PKG_BUILD_DIR)/build/toolchain.cmake";	\
	cmake .. -DCMAKE_TOOLCHAIN_FILE="$(PKG_BUILD_DIR)/build/toolchain.cmake"
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/build
	$(STRIP) $(PKG_BUILD_DIR)/build/virgil_client
	$(STRIP) $(PKG_BUILD_DIR)/build/virgil_server
endef

define Package/virgil-example/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/virgil_client $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/virgil_server $(1)/usr/bin/
endef

$(eval $(call BuildPackage,virgil-example))
