PROJECT_NAME := nrf40watch_fw
TARGETS := nrf40watch_fw

SDK_ROOT := nrf5_sdk
PROJ_DIR := .
OUTPUT_DIRECTORY := _build

SOFT_DEVICE_UC := S140
DEVICE := NRF52840_XXAA

# Determine platform
ifeq ($(OS),Windows_NT)
	RMD = powershell -Command "Remove-Item -Recurse -Force"
	MKDIR = powershell -Command "New-Item -ItemType Directory -Force -Path"
	MKDIR_IF_NOT_EXIST = if not exist "$(1)" $(MKDIR) "$(1)"
else
	RMD = rm -rf
	MKDIR = mkdir -p $(1)
	MKDIR_IF_NOT_EXIST = [ -d "$(1)" ] || $(MKDIR) "$(1)"
endif

# Function to create directory if not exist
define ensure_dir
	@$(call MKDIR_IF_NOT_EXIST,$(1))
endef

rwildcard = $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

$(OUTPUT_DIRECTORY)/$(TARGETS).out: \
	LINKER_SCRIPT  := $(PROJ_DIR)/config/gcc_nrf52.ld

SRC_FILES += \
	$(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_rtt.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_uart.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
	$(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
	$(SDK_ROOT)/components/libraries/button/app_button.c \
	$(SDK_ROOT)/components/libraries/util/app_error.c \
	$(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
	$(SDK_ROOT)/components/libraries/util/app_error_weak.c \
	$(SDK_ROOT)/components/libraries/timer/app_timer_freertos.c \
	$(SDK_ROOT)/components/libraries/util/app_util_platform.c \
	$(SDK_ROOT)/components/libraries/crc16/crc16.c \
	$(SDK_ROOT)/components/libraries/fds/fds.c \
	$(SDK_ROOT)/components/libraries/hardfault/nrf52/handler/hardfault_handler_gcc.c \
	$(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c \
	$(SDK_ROOT)/components/libraries/util/nrf_assert.c \
	$(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
	$(SDK_ROOT)/components/libraries/atomic_flags/nrf_atflags.c \
	$(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
	$(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
	$(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
	$(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
	$(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
	$(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
	$(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
	$(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
	$(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
	$(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
	$(SDK_ROOT)/components/libraries/sensorsim/sensorsim.c \
	$(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
	$(SDK_ROOT)/external/freertos/source/croutine.c \
	$(SDK_ROOT)/external/freertos/source/event_groups.c \
	$(SDK_ROOT)/external/freertos/source/portable/MemMang/heap_1.c \
	$(SDK_ROOT)/external/freertos/source/list.c \
	$(SDK_ROOT)/external/freertos/portable/GCC/nrf52/port.c \
	$(SDK_ROOT)/external/freertos/portable/CMSIS/nrf52/port_cmsis.c \
	$(SDK_ROOT)/external/freertos/portable/CMSIS/nrf52/port_cmsis_systick.c \
	$(SDK_ROOT)/external/freertos/source/queue.c \
	$(SDK_ROOT)/external/freertos/source/stream_buffer.c \
	$(SDK_ROOT)/external/freertos/source/tasks.c \
	$(SDK_ROOT)/external/freertos/source/timers.c \
	$(SDK_ROOT)/components/boards/boards.c \
	$(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
	$(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_uart.c \
	$(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
	$(SDK_ROOT)/components/libraries/bsp/bsp.c \
	$(SDK_ROOT)/components/libraries/bsp/bsp_btn_ble.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
	$(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
	$(SDK_ROOT)/components/ble/peer_manager/auth_status_tracker.c \
	$(SDK_ROOT)/components/ble/common/ble_advdata.c \
	$(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
	$(SDK_ROOT)/components/ble/common/ble_conn_params.c \
	$(SDK_ROOT)/components/ble/common/ble_conn_state.c \
	$(SDK_ROOT)/components/ble/common/ble_srv_common.c \
	$(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c \
	$(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c \
	$(SDK_ROOT)/components/ble/peer_manager/id_manager.c \
	$(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
	$(SDK_ROOT)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_database.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_id.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_manager.c \
	$(SDK_ROOT)/components/ble/peer_manager/peer_manager_handler.c \
	$(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c \
	$(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c \
	$(SDK_ROOT)/components/ble/peer_manager/security_manager.c \
	$(SDK_ROOT)/external/utf_converter/utf.c \
	$(SDK_ROOT)/components/ble/ble_services/ble_bas/ble_bas.c \
	$(SDK_ROOT)/components/ble/ble_services/ble_dis/ble_dis.c \
	$(SDK_ROOT)/components/ble/ble_services/ble_hrs/ble_hrs.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh_freertos.c \
	$(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
	$(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_spi.c \
	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spi.c \
	
#	$(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spim.c \


LVGL_DIR =  $(PROJ_DIR)/lvgl

# Usage: collect all .c files in LVGL_DIR/src
SRC_FILES += $(call rwildcard,$(LVGL_DIR)/src,*.c)

# Main firmware source and object files
SRC_FILES += $(wildcard $(PROJ_DIR)/main/*.c)
SRC_FILES += $(wildcard $(PROJ_DIR)/drivers/*.c)


INC_FOLDERS += \
	$(PROJ_DIR)/main \
	$(PROJ_DIR)/drivers \
	$(PROJ_DIR)/config \

INC_FOLDERS += $(LVGL_DIR)
INC_FOLDERS += $(LVGL_DIR)/src

INC_FOLDERS += \
	$(SDK_ROOT)/components/nfc/ndef/generic/message \
	$(SDK_ROOT)/components/nfc/t2t_lib \
	$(SDK_ROOT)/components/nfc/t4t_parser/hl_detection_procedure \
	$(SDK_ROOT)/components/ble/ble_services/ble_ancs_c \
	$(SDK_ROOT)/components/nfc/platform \
	$(SDK_ROOT)/components/ble/ble_services/ble_ias_c \
	$(SDK_ROOT)/components/libraries/pwm \
	$(SDK_ROOT)/components/libraries/usbd/class/cdc/acm \
	$(SDK_ROOT)/components/libraries/usbd/class/hid/generic \
	$(SDK_ROOT)/components/libraries/usbd/class/msc \
	$(SDK_ROOT)/components/libraries/usbd/class/hid \
	$(SDK_ROOT)/modules/nrfx/hal \
	$(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/le_oob_rec_parser \
	$(SDK_ROOT)/components/libraries/log \
	$(SDK_ROOT)/components/ble/ble_services/ble_gls \
	$(SDK_ROOT)/components/libraries/fstorage \
	$(SDK_ROOT)/components/nfc/ndef/text \
	$(SDK_ROOT)/components/libraries/mutex \
	$(SDK_ROOT)/components/libraries/gpiote \
	$(SDK_ROOT)/components/libraries/bootloader/ble_dfu \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/common \
	$(SDK_ROOT)/components/boards \
	$(SDK_ROOT)/components/nfc/ndef/generic/record \
	$(SDK_ROOT)/components/nfc/t4t_parser/cc_file \
	$(SDK_ROOT)/components/ble/ble_advertising \
	$(SDK_ROOT)/external/utf_converter \
	$(SDK_ROOT)/components/ble/ble_services/ble_bas_c \
	$(SDK_ROOT)/modules/nrfx/drivers/include \
	$(SDK_ROOT)/components/libraries/experimental_task_manager \
	$(SDK_ROOT)/components/ble/ble_services/ble_hrs_c \
	$(SDK_ROOT)/components/softdevice/s140/headers/nrf52 \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/le_oob_rec \
	$(SDK_ROOT)/components/libraries/queue \
	$(SDK_ROOT)/components/libraries/pwr_mgmt \
	$(SDK_ROOT)/external/freertos/portable/GCC/nrf52 \
	$(SDK_ROOT)/components/ble/ble_dtm \
	$(SDK_ROOT)/components/toolchain/cmsis/include \
	$(SDK_ROOT)/components/ble/ble_services/ble_rscs_c \
	$(SDK_ROOT)/components/ble/common \
	$(SDK_ROOT)/components/ble/ble_services/ble_lls \
	$(SDK_ROOT)/components/libraries/hardfault/nrf52 \
	$(SDK_ROOT)/components/libraries/bsp \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/ac_rec \
	$(SDK_ROOT)/components/ble/ble_services/ble_bas \
	$(SDK_ROOT)/components/libraries/mpu \
	$(SDK_ROOT)/components/libraries/experimental_section_vars \
	$(SDK_ROOT)/components/ble/ble_services/ble_ans_c \
	$(SDK_ROOT)/components/libraries/slip \
	$(SDK_ROOT)/components/libraries/delay \
	$(SDK_ROOT)/components/libraries/csense_drv \
	$(SDK_ROOT)/components/libraries/memobj \
	$(SDK_ROOT)/components/ble/ble_services/ble_nus_c \
	$(SDK_ROOT)/components/softdevice/common \
	$(SDK_ROOT)/components/ble/ble_services/ble_ias \
	$(SDK_ROOT)/components/libraries/usbd/class/hid/mouse \
	$(SDK_ROOT)/components/libraries/low_power_pwm \
	$(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/ble_oob_advdata_parser \
	$(SDK_ROOT)/components/ble/ble_services/ble_dfu \
	$(SDK_ROOT)/external/fprintf \
	$(SDK_ROOT)/components/libraries/svc \
	$(SDK_ROOT)/components/libraries/atomic \
	$(SDK_ROOT)/components \
	$(SDK_ROOT)/components/libraries/scheduler \
	$(SDK_ROOT)/components/libraries/cli \
	$(SDK_ROOT)/components/ble/ble_services/ble_lbs \
	$(SDK_ROOT)/components/ble/ble_services/ble_hts \
	$(SDK_ROOT)/components/libraries/crc16 \
	$(SDK_ROOT)/components/nfc/t4t_parser/apdu \
	$(SDK_ROOT)/external/freertos/config \
	$(SDK_ROOT)/components/libraries/util \
	$(SDK_ROOT)/external/freertos/portable/CMSIS/nrf52 \
	$(SDK_ROOT)/components/libraries/usbd/class/cdc \
	$(SDK_ROOT)/components/libraries/csense \
	$(SDK_ROOT)/components/libraries/balloc \
	$(SDK_ROOT)/components/libraries/ecc \
	$(SDK_ROOT)/components/libraries/hardfault \
	$(SDK_ROOT)/components/ble/ble_services/ble_cscs \
	$(SDK_ROOT)/components/libraries/hci \
	$(SDK_ROOT)/components/libraries/usbd/class/hid/kbd \
	$(SDK_ROOT)/components/libraries/timer \
	$(SDK_ROOT)/components/softdevice/s140/headers \
	$(SDK_ROOT)/integration/nrfx \
	$(SDK_ROOT)/components/nfc/t4t_parser/tlv \
	$(SDK_ROOT)/components/libraries/sortlist \
	$(SDK_ROOT)/components/libraries/spi_mngr \
	$(SDK_ROOT)/components/libraries/led_softblink \
	$(SDK_ROOT)/components/nfc/ndef/conn_hand_parser \
	$(SDK_ROOT)/components/libraries/sdcard \
	$(SDK_ROOT)/components/nfc/ndef/parser/record \
	$(SDK_ROOT)/modules/nrfx/mdk \
	$(SDK_ROOT)/components/ble/ble_services/ble_cts_c \
	$(SDK_ROOT)/components/ble/ble_services/ble_nus \
	$(SDK_ROOT)/components/libraries/twi_mngr \
	$(SDK_ROOT)/components/ble/ble_services/ble_hids \
	$(SDK_ROOT)/components/libraries/strerror \
	$(SDK_ROOT)/components/libraries/crc32 \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_oob_advdata \
	$(SDK_ROOT)/components/nfc/t2t_parser \
	$(SDK_ROOT)/external/freertos/source/include \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_pair_msg \
	$(SDK_ROOT)/components/libraries/usbd/class/audio \
	$(SDK_ROOT)/components/libraries/sensorsim \
	$(SDK_ROOT)/components/nfc/t4t_lib \
	$(SDK_ROOT)/components/ble/peer_manager \
	$(SDK_ROOT)/components/libraries/mem_manager \
	$(SDK_ROOT)/components/libraries/ringbuf \
	$(SDK_ROOT)/components/ble/ble_services/ble_tps \
	$(SDK_ROOT)/components/nfc/ndef/parser/message \
	$(SDK_ROOT)/components/ble/ble_services/ble_dis \
	$(SDK_ROOT)/components/nfc/ndef/uri \
	$(SDK_ROOT)/components/ble/nrf_ble_gatt \
	$(SDK_ROOT)/components/ble/nrf_ble_qwr \
	$(SDK_ROOT)/components/libraries/gfx \
	$(SDK_ROOT)/components/libraries/button \
	$(SDK_ROOT)/modules/nrfx \
	$(SDK_ROOT)/components/libraries/twi_sensor \
	$(SDK_ROOT)/integration/nrfx/legacy \
	$(SDK_ROOT)/components/libraries/usbd \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/ep_oob_rec \
	$(SDK_ROOT)/external/segger_rtt \
	$(SDK_ROOT)/components/libraries/atomic_fifo \
	$(SDK_ROOT)/components/ble/ble_services/ble_lbs_c \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/ble_pair_lib \
	$(SDK_ROOT)/components/libraries/crypto \
	$(SDK_ROOT)/components/ble/ble_racp \
	$(SDK_ROOT)/components/libraries/fds \
	$(SDK_ROOT)/components/nfc/ndef/launchapp \
	$(SDK_ROOT)/components/libraries/atomic_flags \
	$(SDK_ROOT)/components/ble/ble_services/ble_hrs \
	$(SDK_ROOT)/components/ble/ble_services/ble_rscs \
	$(SDK_ROOT)/components/nfc/ndef/connection_handover/hs_rec \
	$(SDK_ROOT)/components/nfc/ndef/conn_hand_parser/ac_rec_parser \
	$(SDK_ROOT)/components/libraries/stack_guard \
	$(SDK_ROOT)/components/libraries/log/src \


# Optimization flags
OPT = -Os -g3 -fno-exceptions -fno-non-call-exceptions
# Uncomment the line below to enable link time optimization
#OPT += -flto


OPT += -DNRF_LOG_ENABLED

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DCFG_DEBUG=0
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DBOARD_PCA10056
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DFREERTOS
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
CFLAGS += -D$(SOFT_DEVICE_UC)
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums
CFLAGS += -MMD -MP

CFLAGS += -DNRF_LOG_ENABLED=1
CFLAGS += -DNRF_LOG_BACKEND_UART_ENABLED=1
CFLAGS += -DNRF_LOG_BACKEND_UART_TX_PIN=25
CFLAGS += -DNRF_LOG_BACKEND_UART_BAUDRATE=30801920
CFLAGS += -DNRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE=64

# C++ flags common to all targets
CXXFLAGS += $(OPT)
# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DBOARD_PCA10056
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52840_XXAA
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=7
ASMFLAGS += -D$(SOFT_DEVICE_UC)
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DFREERTOS


# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs
LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--print-memory-usage

$(PROJECT_NAME): CFLAGS += -D__HEAP_SIZE=1024
$(PROJECT_NAME): CFLAGS += -D__STACK_SIZE=2048
$(PROJECT_NAME): ASMFLAGS += -D__HEAP_SIZE=1024
$(PROJECT_NAME): ASMFLAGS += -D__STACK_SIZE=2048

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

INCLUDES := $(addprefix -I, $(INC_FOLDERS))


.PHONY: default app help clean

# Default target - first one defined
default: directories app

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		$(PROJECT_NAME)
	@echo		...

# Create necessary directories
directories:
	$(call ensure_dir,$(OUTPUT_DIRECTORY))

#------------------------------------------------------------------------------

# Main firmware
app: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

clean:
	$(RMD) $(OUTPUT_DIRECTORY)

flash: default
	@echo	** Program nRF40Watch with $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex and settings
	python ./dfu/hexmerge.py --overlap=replace ./dfu/bl_settings.hex $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex -o ./dfu/$(PROJECT_NAME)_settings.hex
	openocd.exe -c "tcl_port disabled" -c "gdb_port 3333" -c "telnet_port 4444" -f interface/stlink.cfg -c 'transport select hla_swd' -f target/nrf52.cfg -c "program ./dfu/$(PROJECT_NAME)_settings.hex" -c reset -c shutdown

flash_app: default
	@echo	** Program nRF40Watch with $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex	
	openocd.exe -c "tcl_port disabled" -c "gdb_port 3333" -c "telnet_port 4444" -f interface/stlink.cfg -c 'transport select hla_swd' -f target/nrf52.cfg -c "program $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex" -c reset -c shutdown

flash_erase:
	@echo	** Erase nRF40Watch flash
	openocd.exe -f interface/stlink.cfg -c 'transport select hla_swd' -f target/nrf52.cfg -c init -c 'reset halt' -c 'nrf5 mass_erase' -c reset -c shutdown

flash_sd:
	@echo	** Program nRF40Watch with Softdevice
	openocd.exe -c "tcl_port disabled" -c "gdb_port 3333" -c "telnet_port 4444" -f interface/stlink.cfg -c 'transport select hla_swd' -f target/nrf52.cfg -c "program $(SDK_ROOT)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex" -c reset -c shutdown

settings:
	@echo	** Generating Settings
	D:\tools\nrfutil.exe settings generate --family NRF52840 --key-file d:/Work/PineTime/nordic_pem_keys/pinetime.pem --bootloader-version 4 --application-version 1 --bl-settings-version 2 --app-boot-validation NO_VALIDATION --sd-boot-validation NO_VALIDATION --softdevice $(SDK_ROOT)/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex --application $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex ./dfu/bl_settings.hex

flash_boot:
	@echo	** Program Bootloader and Settings
	python ./dfu/hexmerge.py --overlap=replace ./dfu/bl_settings.hex ./dfu/bootloader.hex -o ./dfu/bootloader_settings.hex
	openocd.exe -c "tcl_port disabled" -c "gdb_port 3333" -c "telnet_port 4444" -f interface/stlink.cfg -c 'transport select hla_swd' -f target/nrf52.cfg -c "program ./dfu/bootloader_settings.hex" -c reset -c shutdown
