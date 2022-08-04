CFLAGS := -std=c11 -g -O0 -D_POSIX_C_SOURCE=200112L $(CFLAGS)
CXXFLAGS := -std=c++14 -g -O0 $(CXXFLAGS)

DIRS := $(notdir $(shell find . -mindepth 1 -maxdepth 1 -type d))
MAGMA_DIR := $(shell pwd)/magma
MAGMA_LOG := -D'MAGMA_LOG(b,c)=do{magma_log((b),(int)(c));}while(0)' -DMAGMA_ENABLE_CANARIES
MAGMA_CFLAGS := -include "$(MAGMA_DIR)/canary.h"
MAGMA_CXXFLAGS := -include "$(MAGMA_DIR)/canary.h"
MAGMA_LDFLAGS :=
MAGMA_LIBS := -lrt

HOST ?= x86_64
MAGMA_STORAGE ?= MAGMA3
MAGMA_ISAN := 1

ifdef MAGMA_ISAN
	CFLAGS := $(CFLAGS) -DMAGMA_FATAL_CANARIES
	CXXFLAGS := $(CXXFLAGS) -DMAGMA_FATAL_CANARIES
endif

ifeq "$(HOST)" "x86"
	CFLAGS := -m32 $(CFLAGS)
	CXXFLAGS := -m32 $(CXXFLAGS)
	LDFLAGS := -m32 $(LDFLAGS)
endif

.PHONY: all $(DIRS)

all: $(DIRS)
	@echo "Benchmark binaries written to $(BINDIR)"

libpng16:
	@cd libpng16; \
	 ./autogen.sh --maintainer --clean; \
	 ./autogen.sh --maintainer; \
	 export CFLAGS="$(CFLAGS) $(MAGMA_CFLAGS)"; \
	 export CXXFLAGS="$(CXXFLAGS) $(MAGMA_CXXFLAGS)"; \
	 export LDFLAGS="$(LDFLAGS) $(MAGMA_LDFLAGS)"; \
	 export LIBS="$(LIBS) $(MAGMA_LIBS)"; \
	 export PNG_COPTS="$(MAGMA_LOG) -fsanitize=address,fuzzer"; \
	 { ./configure --host=$(HOST) && $(MAKE) clean all; } ;
	@cp libpng16/.libs/libpng16.a $(BINDIR)/libpng16.a$(MAGMA_SUFFIX)
