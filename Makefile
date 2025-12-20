# ESPHome BTHome Examples Makefile
# Compile and flash all example configurations

.PHONY: help compile-all flash clean list

# All example configurations (excluding packages, secrets, etc.)
EXAMPLES := \
	cpu_temp_esp32.yaml \
	cpu_temp_esp32_nimble.yaml \
	cpu_temp_receiver.yaml \
	esp32_devkit_basic.yaml \
	two_gang_switch_esp32.yaml \
	two_gang_switch_nrf52.yaml \
	bthome_receiver_bluedroid.yaml \
	bthome_receiver_nimble.yaml \
	weather_display_t5_47.yaml

# ESP32 examples only (for faster CI)
ESP32_EXAMPLES := \
	cpu_temp_esp32.yaml \
	cpu_temp_esp32_nimble.yaml \
	cpu_temp_receiver.yaml \
	esp32_devkit_basic.yaml \
	two_gang_switch_esp32.yaml \
	bthome_receiver_bluedroid.yaml \
	bthome_receiver_nimble.yaml \
	weather_display_t5_47.yaml

# nRF52 examples
NRF52_EXAMPLES := \
	two_gang_switch_nrf52.yaml

# Default target
help:
	@echo "ESPHome BTHome Examples"
	@echo ""
	@echo "Usage:"
	@echo "  make compile-all      Compile all examples"
	@echo "  make compile-esp32    Compile ESP32 examples only"
	@echo "  make compile-nrf52    Compile nRF52 examples only"
	@echo "  make compile FILE=x   Compile specific file"
	@echo "  make flash FILE=x     Flash specific file to device"
	@echo "  make run FILE=x       Compile and flash specific file"
	@echo "  make logs FILE=x      View logs from device"
	@echo "  make clean            Clean build artifacts"
	@echo "  make list             List all example files"
	@echo ""
	@echo "Examples:"
	@echo "  make compile FILE=cpu_temp_esp32.yaml"
	@echo "  make flash FILE=weather_display_t5_47.yaml"
	@echo "  make run FILE=esp32_devkit_basic.yaml"

# List all examples
list:
	@echo "Available examples:"
	@for f in $(EXAMPLES); do echo "  $$f"; done

# Compile all examples
compile-all:
	@echo "Compiling all examples..."
	@for f in $(EXAMPLES); do \
		echo "\n=== Compiling $$f ==="; \
		esphome compile $$f || exit 1; \
	done
	@echo "\nAll examples compiled successfully!"

# Compile ESP32 examples only
compile-esp32:
	@echo "Compiling ESP32 examples..."
	@for f in $(ESP32_EXAMPLES); do \
		echo "\n=== Compiling $$f ==="; \
		esphome compile $$f || exit 1; \
	done
	@echo "\nAll ESP32 examples compiled successfully!"

# Compile nRF52 examples only
compile-nrf52:
	@echo "Compiling nRF52 examples..."
	@for f in $(NRF52_EXAMPLES); do \
		echo "\n=== Compiling $$f ==="; \
		esphome compile $$f || exit 1; \
	done
	@echo "\nAll nRF52 examples compiled successfully!"

# Compile specific file
compile:
ifndef FILE
	$(error FILE is required. Usage: make compile FILE=example.yaml)
endif
	esphome compile $(FILE)

# Flash specific file
flash:
ifndef FILE
	$(error FILE is required. Usage: make flash FILE=example.yaml)
endif
	esphome upload $(FILE)

# Compile and flash (run)
run:
ifndef FILE
	$(error FILE is required. Usage: make run FILE=example.yaml)
endif
	esphome run $(FILE)

# View logs
logs:
ifndef FILE
	$(error FILE is required. Usage: make logs FILE=example.yaml)
endif
	esphome logs $(FILE)

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf .esphome/build
	@echo "Done!"

# Validate all configurations (fast check without compiling)
validate-all:
	@echo "Validating all examples..."
	@for f in $(EXAMPLES); do \
		echo "Validating $$f..."; \
		esphome config $$f > /dev/null || exit 1; \
	done
	@echo "\nAll examples valid!"
