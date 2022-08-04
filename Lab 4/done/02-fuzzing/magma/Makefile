CBDIR := $(abspath codebase)
TMPDIR := $(abspath tmp)
PATCHDIR := $(abspath patches)
BINDIR := $(abspath build)
export BINDIR

BUG_PATCHES := $(addprefix bugs/,$(notdir $(wildcard $(PATCHDIR)/bugs/*.patch)))
SETUP_PATCHES := $(addprefix setup/,$(notdir $(wildcard $(PATCHDIR)/setup/*.patch)))
PATCHES := $(SETUP_PATCHES) $(BUG_PATCHES)
TARGETS := $(shell find $(CBDIR) -maxdepth 1 -type d -printf %P\\n)

BUG_COUNT := $(shell echo $(lastword $(sort $(patsubst bugs/%.patch,%,$(BUG_PATCHES)))) | sed 's/^0*//')
export BUG_COUNT

# Clang is needed for -fsanitize=fuzzer
export CC=clang
export CXX=clang++

.PHONY: all all_targets all_patches clean copy setup $(PATCHES) $(TARGETS)

.NOTPARALLEL:

all: all_patches all_targets
	@rm -r $(TMPDIR)

all_targets: $(TARGETS)

all_patches: $(PATCHES)

clean:
	rm -rf tmp build/*

copy:
	@echo "Copying codebase to tmp directory"
	@if [ -d "$(TMPDIR)" ]; then \
		rm -r "$(TMPDIR)"; \
	fi;
	@cp -r "$(CBDIR)" "$(TMPDIR)"

setup: copy $(SETUP_PATCHES)

$(PATCHES): copy
	@echo "Applying patch $(basename $@)"
	@sed -- 's/$(notdir $(CBDIR))/$(notdir $(TMPDIR))/g' $(PATCHDIR)/$@ | patch -p1

$(TARGETS): setup
	@mkdir -p $(BINDIR)/programs
	@echo "Building $@"
	@cd $(TMPDIR); $(MAKE) $@;
