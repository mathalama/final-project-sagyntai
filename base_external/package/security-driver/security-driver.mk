SECURITY_DRIVER_VERSION = 1.0
SECURITY_DRIVER_SITE = $(BR2_EXTERNAL_PROJECT_BASE_PATH)/package/security-driver/src
SECURITY_DRIVER_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))
