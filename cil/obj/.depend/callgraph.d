  $(OBJDIR)/callgraph.cmo:  $(OBJDIR)/trace.cmi  $(OBJDIR)/pretty.cmi \
     $(OBJDIR)/inthash.cmi  $(OBJDIR)/errormsg.cmi  $(OBJDIR)/cil.cmi \
      $(OBJDIR)/callgraph.cmi
  $(OBJDIR)/callgraph.cmx:  $(OBJDIR)/trace.cmx  $(OBJDIR)/pretty.cmx \
     $(OBJDIR)/inthash.cmx  $(OBJDIR)/errormsg.cmx  $(OBJDIR)/cil.cmx \
      $(OBJDIR)/callgraph.cmi
