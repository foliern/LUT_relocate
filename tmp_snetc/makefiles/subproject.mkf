###############################################################################
#
# Calling conventions:
#
#  Start rules: 
#    default    compile default code (subproject dependent)
#    devel      compile developer code 
#    prod       compile product code
#    efence     compile efence debugging code
#    clean      cleanup derived files
#    cleandevel cleanup compiled files (developer code only)
#    cleanprod  cleanup compiled files (product code only)
#
#  Parameters:
#    DEPS="no"  de-activate dependency checking meachanism
#    HIDE=""    show important commands issued by make (debugging)
#
###############################################################################


#######################################################################################
#
# general setup:
#

PROJECT_ROOT := .

HIDE   := @
DEPS   := yes

PREFIX_LOCAL := 
PREFIX_ROOT  := $(PROJECT_ROOT)/

include $(PROJECT_ROOT)/makefiles/config.mkf
include $(PROJECT_ROOT)/makefiles/targets_snetc.mkf

SUBPROJECT       := snetc
SUBPROJECT_DIRS  := $(snetc)

targetdirname    := .
targetdir        := .

targets_devel    := $(addprefix $(targetdir)/,$(targets_snetc_devel))
targets_prod     := $(addprefix $(targetdir)/,$(targets_snetc_prod))
targets_efence   := $(addprefix $(targetdir)/,$(targets_snetc_efence))
targets_default  := $(addprefix $(targetdir)/,$(targets_snetc_default))

clean_also       := $(addprefix $(targetdir)/,$(clean_snetc))
clean_also_devel := $(addprefix $(targetdir)/,$(clean_snetc_devel))
clean_also_prod  := $(addprefix $(targetdir)/,$(clean_snetc_prod))


###############################################################################
#
# Dummy rules
#

.PHONY: clean clean_dirs %.clean 
.PHONY: cleanprod cleanprod_dirs %.cleanprod 
.PHONY: cleandevel cleandevel_dirs %.cleandevel
.PHONY: default devel prod efence make_%


###############################################################################
#
# Start rules
#

default: make_default

devel: make_devel

prod: make_prod

efence: make_efence


###############################################################################
#
# Main rule
#

make_%:
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Building S-Net snetc "
	@$(ECHO) "* on $(OS) for $(ARCH)"
	@$(ECHO) "************************************************************"
	@$(TOUCH) .make_track
	$(HIDE) $(MAKE) -f $(PROJECT_ROOT)/makefiles/core.mkf \
                         DEPS="$(DEPS)" HIDE="$(HIDE)" \
                         PREFIX_LOCAL="$(PREFIX_LOCAL)" \
                         PREFIX_ROOT="$(PREFIX_ROOT)" \
                         TARGETDIR="$(SUBPROJECT_DIRS)" \
                         TARGETS="$(targets_$*)" 
	@$(ECHO) ""





###############################################################################
#
# Rules for cleaning directories
#

clean: 
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Cleaning S-Net snetc"
	@$(ECHO) "************************************************************"
	@$(ECHO) "Cleaning directory $(PREFIX_ROOT)$(targetdirname)"
	$(HIDE) $(RM) $(targets_devel) $(targets_prod) $(clean_also)
	@$(MAKE) HIDE="$(HIDE)" clean_dirs
	@$(ECHO) ""

clean_no_targetdir: 
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Cleaning S-Net snetc"
	@$(ECHO) "************************************************************"
	@$(MAKE) HIDE="$(HIDE)" clean_dirs
	@$(ECHO) ""

clean_dirs: $(addsuffix .clean,$(SUBPROJECT_DIRS))

%.clean:
	@$(ECHO) "Cleaning directory $(PREFIX_LOCAL)$*"
	$(HIDE) $(MAKE) -C $* HIDE="$(HIDE)" clean


cleanprod: 
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Cleaning S-Net snetc  (product code only)"
	@$(ECHO) "************************************************************"
	@$(ECHO) "Cleaning directory $(PREFIX_ROOT)$(targetdirname)"
	$(HIDE) $(RM) $(targets_prod) $(clean_also_prod)
	@$(MAKE) HIDE="$(HIDE)" cleanprod_dirs
	@$(ECHO) ""

cleanprod_dirs: $(addsuffix .cleanprod,$(SUBPROJECT_DIRS))

%.cleanprod:
	@$(ECHO) "Cleaning directory (PREFIX_LOCAL)$*"
	$(HIDE) $(MAKE) -C $* HIDE="$(HIDE)" cleanprod


cleandevel: 
	@$(ECHO) ""
	@$(ECHO) "************************************************************"
	@$(ECHO) "* Cleaning S-Net snetc  (developer code only)"
	@$(ECHO) "************************************************************"
	@$(ECHO) "Cleaning directory $(PREFIX_ROOT)$(targetdirname)"
	$(HIDE) $(RM) $(targets_devel) $(clean_also_devel)
	@$(MAKE) HIDE="$(HIDE)" cleandevel_dirs
	@$(ECHO) ""

cleandevel_dirs: $(addsuffix .cleandevel,$(SUBPROJECT_DIRS))

%.cleandevel:
	@$(ECHO) "Cleaning directory (PREFIX_LOCAL)$*"
	$(HIDE) $(MAKE) -C $* HIDE="$(HIDE)" cleandevel

