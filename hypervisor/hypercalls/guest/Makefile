.PHONY: all clean

all: boot/boot app/app

boot/boot app/app:
	$(MAKE) -C $(@D)

clean:
	$(MAKE) -C boot clean
	$(MAKE) -C app clean
