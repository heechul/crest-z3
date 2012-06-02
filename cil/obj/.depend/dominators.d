  $(OBJDIR)/dominators.cmo:  $(OBJDIR)/util.cmi  $(OBJDIR)/pretty.cmi \
     $(OBJDIR)/inthash.cmi  $(OBJDIR)/errormsg.cmi   $(OBJDIR)/dataflow.cmi \
     $(OBJDIR)/cil.cmi   $(OBJDIR)/dominators.cmi
  $(OBJDIR)/dominators.cmx:  $(OBJDIR)/util.cmx  $(OBJDIR)/pretty.cmx \
     $(OBJDIR)/inthash.cmx  $(OBJDIR)/errormsg.cmx   $(OBJDIR)/dataflow.cmx \
     $(OBJDIR)/cil.cmx   $(OBJDIR)/dominators.cmi
