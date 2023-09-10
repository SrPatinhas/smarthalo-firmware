# This Makefile configures the various options for products, and selects the correct sub-Makefiles
# Given values for:
#    PRODUCT_ID
#    PCBVER
# it will set the appropriate values for:
#    PRODUCT_NAME
#    BUILD_ENVIRONMENT

ifeq ($(PRODUCT_ID),) # Blank Product ID
$(error PRODUCT_ID must be defined)
endif

ifeq ($(PCBVER),) # Blank PCB version
$(error PCBVER must be defined)
endif

# Space-separated list of all products
PRODUCT_IDS = 1 2

ifeq ($(filter $(PRODUCT_ID),$(PRODUCT_IDS)),)
$(error Product ID $(PRODUCT_ID) not valid. Options are $(PRODUCT_IDS))
endif


ifeq ($(PRODUCT_ID),1)
PRODUCT_NAME=SmartHalo 1
BUILD_FILE=sh1.mk
BUILD_ENVIRONMENT=ghcr.io/charliebruce/nrf5-docker-build:sdk-12.1.0
PCBVERS = 1
endif

ifeq ($(PRODUCT_ID),2)
PRODUCT_NAME=SmartHalo 2
BUILD_FILE=sh2.mk
BUILD_ENVIRONMENT=ghcr.io/charliebruce/nrf5-docker-build:sdk-16.0.0
PCBVERS = 1
endif

# Safety Checks

ifeq ($(PRODUCT_NAME),)
$(error PRODUCT_NAME not defined)
endif

ifeq ($(BUILD_ENVIRONMENT),)
$(error BUILD_ENVIRONMENT not defined)
endif

ifeq ($(filter $(PCBVER),$(PCBVERS)),)
$(error PCB Version $(PCBVER) not allowed for PRODUCT_ID=$(PRODUCT_ID). Allowed options are $(PCBVERS))
endif
