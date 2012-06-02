  $(OBJDIR)/partial.cmo:  $(OBJDIR)/util.cmi    $(OBJDIR)/ptranal.cmi \
     $(OBJDIR)/pretty.cmi   $(OBJDIR)/heap.cmo  $(OBJDIR)/errormsg.cmi \
     $(OBJDIR)/cilutil.cmo  $(OBJDIR)/cil.cmi
  $(OBJDIR)/partial.cmx:  $(OBJDIR)/util.cmx    $(OBJDIR)/ptranal.cmx \
     $(OBJDIR)/pretty.cmx   $(OBJDIR)/heap.cmx  $(OBJDIR)/errormsg.cmx \
     $(OBJDIR)/cilutil.cmx  $(OBJDIR)/cil.cmx
