#
# This software is licensed under the Public Domain.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=wakeonserver
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk


define Package/wakeonserver
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=WakeON server utilities.
endef

define Package/wakeonserver/description
	WakeON server utilities.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Package/wakeonserver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/WakeON_server $(1)/usr/bin/
endef

$(eval $(call BuildPackage,wakeonserver))
