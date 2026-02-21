OPENSCAD := openscad
PYTHON   := python3

SRCDIR   := .
OUTDIR   := output

# Source files
CONFIG   := $(SRCDIR)/config.scad
LID_SRC  := $(SRCDIR)/lid.scad
BASE_SRC := $(SRCDIR)/base.scad
SHELF_SRC := $(SRCDIR)/shelf.scad
BRACKET_SRC := $(SRCDIR)/terminal_bracket.scad
SLICE_SRC := $(SRCDIR)/test_slices.scad
XSEC_SRC := $(SRCDIR)/cross_sections.scad

# Output files
LID_STL     := $(OUTDIR)/lid.stl
BASE_STL    := $(OUTDIR)/base.stl
SHELF_STL   := $(OUTDIR)/shelf.stl
BRACKET_STL := $(OUTDIR)/terminal_bracket.stl

SLICE_RJ45  := $(OUTDIR)/test_slice_rj45_wall.stl
SLICE_SD    := $(OUTDIR)/test_slice_sd_wall.stl

XSEC_RJ45     := $(OUTDIR)/cross_section_rj45.svg
XSEC_SD       := $(OUTDIR)/cross_section_sd.svg
XSEC_TERMINAL := $(OUTDIR)/cross_section_terminal.svg

ALL_STLS  := $(LID_STL) $(BASE_STL) $(SHELF_STL) $(BRACKET_STL)
ALL_SLICES := $(SLICE_RJ45) $(SLICE_SD)
ALL_XSECS := $(XSEC_RJ45) $(XSEC_SD) $(XSEC_TERMINAL)

# ===================================================================
# Targets
# ===================================================================

.PHONY: all stls slices cross-sections verify analyze clean

all: stls slices cross-sections

stls: $(ALL_STLS)

slices: $(ALL_SLICES)

cross-sections: $(ALL_XSECS)

# ===================================================================
# STL rendering
# ===================================================================

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(LID_STL): $(LID_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -o $@ $(LID_SRC)

$(BASE_STL): $(BASE_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -o $@ $(BASE_SRC)

$(SHELF_STL): $(SHELF_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -o $@ $(SHELF_SRC)

$(BRACKET_STL): $(BRACKET_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -o $@ $(BRACKET_SRC)

# ===================================================================
# Test slices
# ===================================================================

$(SLICE_RJ45): $(SLICE_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -D 'slice="rj45"' -o $@ $(SLICE_SRC)

$(SLICE_SD): $(SLICE_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -D 'slice="sd"' -o $@ $(SLICE_SRC)

# ===================================================================
# Cross-section SVGs
# ===================================================================

$(XSEC_RJ45): $(XSEC_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -D 'section="rj45"' -o $@ $(XSEC_SRC)

$(XSEC_SD): $(XSEC_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -D 'section="sd"' -o $@ $(XSEC_SRC)

$(XSEC_TERMINAL): $(XSEC_SRC) $(CONFIG) | $(OUTDIR)
	$(OPENSCAD) -D 'section="terminal"' -o $@ $(XSEC_SRC)

# ===================================================================
# Verification
# ===================================================================

verify:
	$(PYTHON) $(SRCDIR)/verify_alignment.py

analyze:
	$(PYTHON) $(SRCDIR)/analyze_stl.py

# ===================================================================
# Clean
# ===================================================================

clean:
	rm -rf $(OUTDIR)
