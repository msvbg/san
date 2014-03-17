default: san

.DEFAULT:
	cd src && make $@

install:
	cd src && make $@

.PHONY: install
