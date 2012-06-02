 $(OBJDIR)/formatcil.cmo:  $(OBJDIR)/trace.cmi  $(OBJDIR)/stats.cmi \
     $(OBJDIR)/pretty.cmi   $(OBJDIR)/formatparse.cmi \
      $(OBJDIR)/formatlex.cmo  $(OBJDIR)/errormsg.cmi  $(OBJDIR)/cil.cmi \
     $(OBJDIR)/formatcil.cmi
 $(OBJDIR)/formatcil.cmx:  $(OBJDIR)/trace.cmx  $(OBJDIR)/stats.cmx \
     $(OBJDIR)/pretty.cmx   $(OBJDIR)/formatparse.cmx \
      $(OBJDIR)/formatlex.cmx  $(OBJDIR)/errormsg.cmx  $(OBJDIR)/cil.cmx \
     $(OBJDIR)/formatcil.cmi
