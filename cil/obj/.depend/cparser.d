  $(OBJDIR)/cparser.cmo:   $(OBJDIR)/lexerhack.cmo  $(OBJDIR)/errormsg.cmi \
      $(OBJDIR)/cprint.cmo   $(OBJDIR)/cabshelper.cmo   $(OBJDIR)/cabs.cmo \
      $(OBJDIR)/cparser.cmi
  $(OBJDIR)/cparser.cmx:   $(OBJDIR)/lexerhack.cmx  $(OBJDIR)/errormsg.cmx \
      $(OBJDIR)/cprint.cmx   $(OBJDIR)/cabshelper.cmx   $(OBJDIR)/cabs.cmx \
      $(OBJDIR)/cparser.cmi
