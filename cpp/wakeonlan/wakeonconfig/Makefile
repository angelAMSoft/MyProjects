#
# This software is licensed under the Public Domain.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=wakeonconfig
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/wakeonconfig
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=WakeON config utilities.
endef

define Package/wakeonconfig/description
	WakeON config utilities.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/wakeonconfig/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/WakeON_conf $(1)/usr/bin/
	
	mkdir $(1)/etc/wakeon
	chmod 770 -R $(1)/etc/wakeon
endef

$(eval $(call BuildPackage,wakeonconfig))
