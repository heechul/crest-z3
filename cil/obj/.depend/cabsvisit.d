  $(OBJDIR)/cabsvisit.cmo:  $(OBJDIR)/trace.cmi  $(OBJDIR)/pretty.cmi \
     $(OBJDIR)/errormsg.cmi   $(OBJDIR)/cabshelper.cmo   $(OBJDIR)/cabs.cmo \
      $(OBJDIR)/cabsvisit.cmi
  $(OBJDIR)/cabsvisit.cmx:  $(OBJDIR)/trace.cmx  $(OBJDIR)/pretty.cmx \
     $(OBJDIR)/errormsg.cmx   $(OBJDIR)/cabshelper.cmx   $(OBJDIR)/cabs.cmx \
      $(OBJDIR)/cabsvisit.cmi
