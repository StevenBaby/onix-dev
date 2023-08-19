
.PHONY: bochs
bochs: $(IMAGES) $(SRC)/utils/bochsrc
	bochs -q -f $(SRC)/utils/bochsrc -unlock
