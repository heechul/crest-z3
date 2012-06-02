  $(OBJDIR)/frontc.cmo:   $(OBJDIR)/whitetrack.cmi  $(OBJDIR)/trace.cmi \
     $(OBJDIR)/stats.cmi  $(OBJDIR)/pretty.cmi   $(OBJDIR)/patch.cmi \
     $(OBJDIR)/errormsg.cmi   $(OBJDIR)/cprint.cmo   $(OBJDIR)/cparser.cmi \
      $(OBJDIR)/clexer.cmi   $(OBJDIR)/cabs2cil.cmi   $(OBJDIR)/cabs.cmo \
      $(OBJDIR)/frontc.cmi
  $(OBJDIR)/frontc.cmx:   $(OBJDIR)/whitetrack.cmx  $(OBJDIR)/trace.cmx \
     $(OBJDIR)/stats.cmx  $(OBJDIR)/pretty.cmx   $(OBJDIR)/patch.cmx \
     $(OBJDIR)/errormsg.cmx   $(OBJDIR)/cprint.cmx   $(OBJDIR)/cparser.cmx \
      $(OBJDIR)/clexer.cmx   $(OBJDIR)/cabs2cil.cmx   $(OBJDIR)/cabs.cmx \
      $(OBJDIR)/frontc.cmi
