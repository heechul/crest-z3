  $(OBJDIR)/clexer.cmo:   $(OBJDIR)/whitetrack.cmi  $(OBJDIR)/pretty.cmi \
      $(OBJDIR)/machdep.cmo   $(OBJDIR)/lexerhack.cmo \
     $(OBJDIR)/growArray.cmi  $(OBJDIR)/errormsg.cmi   $(OBJDIR)/cprint.cmo \
      $(OBJDIR)/cparser.cmi   $(OBJDIR)/cabshelper.cmo   $(OBJDIR)/cabs.cmo \
      $(OBJDIR)/clexer.cmi
  $(OBJDIR)/clexer.cmx:   $(OBJDIR)/whitetrack.cmx  $(OBJDIR)/pretty.cmx \
      $(OBJDIR)/machdep.cmx   $(OBJDIR)/lexerhack.cmx \
     $(OBJDIR)/growArray.cmx  $(OBJDIR)/errormsg.cmx   $(OBJDIR)/cprint.cmx \
      $(OBJDIR)/cparser.cmx   $(OBJDIR)/cabshelper.cmx   $(OBJDIR)/cabs.cmx \
      $(OBJDIR)/clexer.cmi
