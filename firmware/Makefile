MCU = atmega32u4
ARCH = AVR8
F_CPU = 16000000
LUFA = lufa

SRC = main.c usb.c relay.c descriptors.c button.c led.c
SRC += $(LUFA)/LUFA/Drivers/USB/Class/Common/HIDParser.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/AVR8/Device_AVR8.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/AVR8/EndpointStream_AVR8.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/AVR8/Endpoint_AVR8.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/AVR8/USBController_AVR8.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/AVR8/USBInterrupt_AVR8.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/ConfigDescriptors.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/DeviceStandardReq.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/Events.c
SRC += $(LUFA)/LUFA/Drivers/USB/Core/USBTask.c

ifeq ($(V),1)
P = @:
Q =
else
P = @echo
Q = @
endif

O ?= output
VPATH = $(sort $(dir $(SRC)))
OBJ = $(addprefix $(O)/,$(notdir $(SRC:.c=.o)))
DEP = $(addprefix $(O)/,$(notdir $(SRC:.c=.d)))
ELF = $(O)/firmware.elf
HEX = $(ELF:.elf=.hex)
LSS = $(ELF:.elf=.lss)
EEP_ELF = $(O)/eeprom_loader.elf
EEP_HEX = $(EEP_ELF:.elf=.hex)

CFLAGS = \
	-pipe -gdwarf-2 -g2 -Os -mmcu=$(MCU) -I. -Ilufa \
	-std=gnu99 -Wall -Wstrict-prototypes -Werror \
	-fshort-enums -funsigned-char -funsigned-bitfields -flto \
	-fno-inline-small-functions -fno-strict-aliasing -fuse-linker-plugin \
	-fno-jump-tables -mrelax -ffunction-sections -fdata-sections \
	-DARCH=ARCH_AVR8 -DBOARD=BOARD_NONE -DUSE_LUFA_CONFIG_HEADER \
	-DF_CPU=$(F_CPU)UL -DF_USB=$(F_CPU)UL
LDFLAGS = -lm -Wl,--gc-sections -Wl,--relax

EEL_SRC = $(LUFA)/LUFA/Build/DMBS/DMBS/HID_EEPROM_Loader/HID_EEPROM_Loader.c
VPATH += $(sort $(dir $(EEL_SRC)))
EEL_OBJ = $(addprefix $(O)/,$(notdir $(EEL_SRC:.c=.o)))
EEL_OBJ_INPUT = $(O)/InputEEData.o
EEL_DEP = $(addprefix $(O)/,$(notdir $(EEL_SRC:.c=.d)))
EEL_ELF = $(O)/eeprom_loader.elf
EEL_HEX = $(EEL_ELF:.elf=.hex)


all: $(HEX) $(EEL_HEX) $(LSS)

$(SRC):
	$(error $@: source file does not exist)

$(O):
	$(Q) mkdir -p $@

HEXFLAGS = -O ihex -R .eeprom -R .fuse -R .lock -R .signature

$(O)/%.hex: $(O)/%.elf
	$(P) '  HEX    $@'
	$(Q) avr-objcopy $(HEXFLAGS) $< $@

$(O)/%.lss: $(O)/%.elf
	$(P) '  DISASS $@'
	$(Q) avr-objdump -h -d -S -z $< > $@

$(O)/%.elf:
	$(P) '  LINK   $@'
	$(Q) avr-gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(ELF): $(OBJ)

$(O)/%.o: %.c $(MAKEFILE_LIST) | $(O)
	$(P) '  CC     $@'
	$(Q) avr-gcc -c $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ $<

$(EEL_ELF): $(EEL_OBJ) $(EEL_OBJ_INPUT)

EEPFLAGS = -j .eeprom \
	--no-change-warnings --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0
EEL_EEPFLAGS = \
	-B avr`avr-objdump -f main.o | sed -nre 's/architecture: avr:([0-9]+),.*/\1/p'` \
	--rename-section .data=.progmem.data,contents,alloc,readonly,data

$(EEL_OBJ_INPUT): $(ELF) $(O)/main.o
	$(P) '  EEDATA $@'
	$(Q) avr-objcopy -O binary $(EEPFLAGS) $< $(@:.o=.bin)
	$(Q) cd $(O) && avr-objcopy -I binary -O elf32-avr $(EEL_EEPFLAGS) $(@F:.o=.bin) $(@F)

teensy: $(O)/teensy_loader_cli $(EEL_HEX) $(HEX)
	$(P) '  TEENSY $(EEL_HEX)'
	$(Q) $< -v -w --mcu=$(MCU) $(EEL_HEX)
	$(P) '  TEENSY $(HEX)'
	$(Q) $< -v -w --mcu=$(MCU) $(HEX)

TEENSY_CFLAGS = -O2 -g -Wall -Werror -DUSE_LIBUSB
TEENSY_LDFLAGS = -lusb

$(O)/teensy_loader_cli: teensy_loader/teensy_loader_cli.c | $(O)
	$(P) '  PROG   $@'
	$(Q) gcc $(TEENSY_CFLAGS) -o $@ $< $(TEENSY_LDFLAGS)

clean:
	$(Q) rm -vrf $(O)

-include $(DEP) $(EEL_DEP)

.DELETE_ON_ERROR:
.PHONY: all clean teensy
