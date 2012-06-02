  $(OBJDIR)/logcalls.cmo:  $(OBJDIR)/trace.cmi  $(OBJDIR)/stats.cmi \
     $(OBJDIR)/pretty.cmi  $(OBJDIR)/errormsg.cmi  $(OBJDIR)/cilutil.cmo  $(OBJDIR)/cil.cmi \
      $(OBJDIR)/logcalls.cmi
  $(OBJDIR)/logcalls.cmx:  $(OBJDIR)/trace.cmx  $(OBJDIR)/stats.cmx \
     $(OBJDIR)/pretty.cmx  $(OBJDIR)/errormsg.cmx  $(OBJDIR)/cilutil.cmx  $(OBJDIR)/cil.cmx \
      $(OBJDIR)/logcalls.cmi
