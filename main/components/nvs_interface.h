
#ifndef NVS_INTERFACE_H_
#define NVS_INTERFACE_H_

#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "nvs_flash.h"
#include "nvs.h"

#define NVS_TAG "nvs interface"
esp_err_t read_nvs(nvs_handle_t nvs_handle
		, const char * space_name
		, const char * key
		, uint32_t *value);
esp_err_t write_nvs_func(
		 nvs_handle_t nvs_handle
		, const char * space_name
		, const char *key
		, uint32_t value_storage);


#endif /* NVS_INTERFACE_H_ */
