################################################################################
#
# security-app
#
################################################################################

SECURITY_APP_VERSION = 1.0
SECURITY_APP_SITE = $(BR2_EXTERNAL_PROJECT_BASE_PATH)/package/security-app/src
SECURITY_APP_SITE_METHOD = local

define SECURITY_APP_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define SECURITY_APP_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/security_app $(TARGET_DIR)/usr/bin/security_app
endef

$(eval $(generic-package))
