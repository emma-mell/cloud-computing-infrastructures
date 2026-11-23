SHV_DIR := $(shell pwd)/shv
SHV := $(SHV_DIR)/shv

GUEST_DIR := $(shell pwd)/guest
APP := $(GUEST_DIR)/app/app
BOOT := $(GUEST_DIR)/boot/boot

# Argument parsing for the run target.
# https://stackoverflow.com/a/14061796
ifeq (run,$(firstword $(MAKECMDGOALS)))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
	# Cancel them as targets: turn them into do-nothing targets (silent Shell no-op).
  $(eval $(RUN_ARGS):;@:)
endif

.PHONY: run clean $(SHV) $(BOOT) $(APP)

all: $(SHV) $(BOOT) $(APP)

$(SHV) $(BOOT) $(APP):
	$(MAKE) -C $(@D)

run: $(SHV) $(BOOT) $(APP)
	$(SHV) $(BOOT) $(APP) $(RUN_ARGS)

clean:
	$(MAKE) -C $(SHV_DIR) clean
	$(MAKE) -C $(GUEST_DIR) clean
