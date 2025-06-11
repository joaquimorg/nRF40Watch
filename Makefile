PROJECT_NAME := nrf40watch_fw

SDK_ROOT := nrf5_sdk
PROJ_DIR := .
BUILD_DIR := _build

DEVICE := NRF52840_XXAA
BOARD := custom

# Toolchain commands
# Should be added to your PATH
CROSS_COMPILE ?= arm-none-eabi-
CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AS      = $(CROSS_COMPILE)as
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE    = $(CROSS_COMPILE)size

# Determine platform
ifeq ($(OS),Windows_NT)
	RM = powershell -Command "Remove-Item -Recurse -Force"
	MKDIR = powershell -Command "New-Item -ItemType Directory -Force -Path"
	MKDIR_IF_NOT_EXIST = if not exist "$(1)" $(MKDIR) "$(1)"
else
	RM = rm -f
	MKDIR = mkdir -p $(1)
	MKDIR_IF_NOT_EXIST = [ -d "$(1)" ] || $(MKDIR) "$(1)"
endif

# Function to create directory if not exist
define ensure_dir
	@$(call MKDIR_IF_NOT_EXIST,$(1))
endef

rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Optimization flags
OPT = -Os -g3 -fno-exceptions -fno-non-call-exceptions
# Link time optimization disabled for compatibility
# OPT += -flto

CFLAGS += $(OPT)
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DNRF52840_XXAA
CFLAGS += -std=gnu99 -Wall
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

SDK_ASMSRC_FILES := $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S

SDK_ASMSRC_OBJS = $(addprefix $(BUILD_DIR)/, $(SDK_ASMSRC_FILES:.S=.o))

SDK_SRC_FILES := \
	$(SDK_ROOT)/components/ble/common/ble_advdata.c \
	$(SDK_ROOT)/components/ble/common/ble_conn_params.c \
	$(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
	$(SDK_ROOT)/components/libraries/util/app_error.c \
	$(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
	$(SDK_ROOT)/components/libraries/util/app_util_platform.c \
	$(SDK_ROOT)/components/libraries/util/nrf_assert.c \
	$(SDK_ROOT)/components/libraries/timer/app_timer.c \
	$(SDK_ROOT)/components/libraries/util/sdk_mapped_flags.c \
	$(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spim.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_twim.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
	$(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
	$(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
	$(SDK_ROOT)/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c \
	$(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
	$(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \

SDK_SRC_OBJS = $(addprefix $(BUILD_DIR)/, $(SDK_SRC_FILES:.c=.o))

LVGL_DIR =  $(PROJ_DIR)/lvgl

# Usage: collect all .c files in LVGL_DIR/src
LVGL_SRC_FILES := $(call rwildcard,$(LVGL_DIR)/src,*.c)

LVGL_SRC_OBJS = $(addprefix $(BUILD_DIR)/, $(LVGL_SRC_FILES:.c=.o))

# Main firmware source and object files
APP_SRC_FILES += $(wildcard $(PROJ_DIR)/main/*.c)
APP_SRC_FILES += $(wildcard $(PROJ_DIR)/services/*.c)
APP_SRC_FILES += $(wildcard $(PROJ_DIR)/drivers/*.c)

APP_SRC_OBJS = $(addprefix $(BUILD_DIR)/, $(APP_SRC_FILES:.c=.o))

INC_FOLDERS := \
	$(SDK_ROOT)/components/softdevice/s140/headers \
	$(SDK_ROOT)/components/softdevice/s140/headers/nrf52 \
	$(SDK_ROOT)/components/softdevice/common \
	main \
	services \
	drivers \
	$(SDK_ROOT)/components \
	$(SDK_ROOT)/components/toolchain/cmsis/include \
	$(SDK_ROOT)/components/libraries/util \
	$(SDK_ROOT)/components/ble/common \
	$(SDK_ROOT)/components/ble/ble_advertising \
	$(SDK_ROOT)/components/ble/ble_services \
	$(SDK_ROOT)/components/ble/nrf_ble_gatt \
	$(SDK_ROOT)/components/boards \
	$(SDK_ROOT)/components/libraries/timer \
	$(SDK_ROOT)/components/libraries/pwr_mgmt \
	$(SDK_ROOT)/components/libraries/delay \
	$(SDK_ROOT)/components/libraries/log \
	$(SDK_ROOT)/components/libraries/log/src \
	$(SDK_ROOT)/components/libraries/strerror \
	$(SDK_ROOT)/components/libraries/experimental_section_vars \
	$(SDK_ROOT)/modules/nrfx \
	$(SDK_ROOT)/integration/nrfx \
	$(SDK_ROOT)/integration/nrfx/legacy \
	$(SDK_ROOT)/modules/nrfx/mdk \
	$(SDK_ROOT)/modules/nrfx/drivers/include \
	$(PROJ_DIR)/config \
	$(LVGL_DIR)/ \
	$(LVGL_DIR)/src \


# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -T$(PROJ_DIR)/config/gcc_nrf52.ld
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

OBJECTS := $(LVGL_SRC_OBJS) $(SDK_ASMSRC_OBJS) $(SDK_SRC_OBJS) $(APP_SRC_OBJS)

INCLUDES := $(addprefix -I, $(INC_FOLDERS))

#------------------------------------------------------------------------------
# Phony targets
.PHONY: all app directories clean

#all: $(BUILD_DIR)/$(PROJECT_NAME).hex
all: directories app

# Create necessary directories
directories:
	$(call ensure_dir,$(BUILD_DIR))

-include $(OBJECTS:.o=.d)

$(BUILD_DIR)/%.o: %.c
	@echo CC $<
	$(call ensure_dir,$(@D))
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Assemble files
$(BUILD_DIR)/%.o: %.S
	@echo AS $<
	$(call ensure_dir,$(@D))
	@$(CXX) -x assembler-with-cpp $(CFLAGS) $(INCLUDES) -c $< -o $@

#------------------------------------------------------------------------------

# Main firmware
app: $(BUILD_DIR)/$(PROJECT_NAME).out

$(BUILD_DIR)/$(PROJECT_NAME).out: $(OBJECTS)
	@echo LD $@
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	@$(OBJCOPY) -O ihex $(BUILD_DIR)/$(PROJECT_NAME).out $(BUILD_DIR)/$(PROJECT_NAME).hex

#$(BUILD_DIR)/$(PROJECT_NAME).hex: $(SRC_FILES)
#	@$(call MKDIR,$(BUILD_DIR))
#	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_FILES) -o $(BUILD_DIR)/$(PROJECT_NAME).out $(LDFLAGS)
#	$(OBJCOPY) -O ihex $(BUILD_DIR)/$(PROJECT_NAME).out $(BUILD_DIR)/$(PROJECT_NAME).hex



clean:
	@if exist $(BUILD_DIR) $(RM) $(BUILD_DIR)

