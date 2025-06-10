PROJECT_NAME := nrf40watch_fw

SDK_ROOT := ./nrf5_sdk
PROJ_DIR := .

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
  RM = del /Q /F
  MKDIR = if not exist $(subst /,\\,$(1)) mkdir $(subst /,\\,$(1))
else
  RM = rm -f
  MKDIR = mkdir -p $(1)
endif

# Optimization flags
OPT = -Os -g3 -fno-exceptions -fno-non-call-exceptions
# Uncomment the line below to enable link time optimization
OPT += -flto

CFLAGS += $(OPT)
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DNRF52840_XXAA
CFLAGS += -std=gnu99 -Wall -Werror
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

SDK_SRC_FILES := \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
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


SRC_FILES := \
  $(PROJ_DIR)/main/main.c \
  $(PROJ_DIR)/main/gui.c \
  $(PROJ_DIR)/services/ble.c \
  $(PROJ_DIR)/services/time.c \
  $(PROJ_DIR)/drivers/lcd_st7789.c \
  $(PROJ_DIR)/drivers/touch_cst816s.c \
  $(PROJ_DIR)/drivers/backlight.c \
  $(wildcard $(PROJ_DIR)/lvgl/**/*.c)

SRC_FILES += $(SDK_SRC_FILES)

INC_FOLDERS := \
  $(PROJ_DIR)/main \
  $(PROJ_DIR)/services \
  $(PROJ_DIR)/drivers \
  $(PROJ_DIR)/lvgl \
  $(PROJ_DIR)/lvgl/src \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/softdevice/s140/headers \
  $(SDK_ROOT)/components/softdevice/s140/headers/nrf52 \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/ble/ble_services \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(PROJ_DIR)/config

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -T$(SDK_ROOT)/gcc_nrf52.ld
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

BUILD_DIR := _build

INCLUDES := $(addprefix -I, $(INC_FOLDERS))
OBJS := $(SRC_FILES:.c=.o)

OBJS := $(SRC_FILES:.c=.o)

all: $(BUILD_DIR)/$(PROJECT_NAME).hex

$(BUILD_DIR)/$(PROJECT_NAME).hex: $(SRC_FILES)
	@$(call MKDIR,$(BUILD_DIR))
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC_FILES) -o $(BUILD_DIR)/$(PROJECT_NAME).out $(LDFLAGS)
	$(OBJCOPY) -O ihex $(BUILD_DIR)/$(PROJECT_NAME).out $(BUILD_DIR)/$(PROJECT_NAME).hex

clean:
	$(RM) $(BUILD_DIR)/*.o $(BUILD_DIR)/*.out $(BUILD_DIR)/*.hex

