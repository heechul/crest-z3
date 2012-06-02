  $(OBJDIR)/liveness.cmo:   $(OBJDIR)/usedef.cmo  $(OBJDIR)/pretty.cmi \
     $(OBJDIR)/inthash.cmi  $(OBJDIR)/errormsg.cmi   $(OBJDIR)/dataflow.cmi \
     $(OBJDIR)/cil.cmi   $(OBJDIR)/cfg.cmi
  $(OBJDIR)/liveness.cmx:   $(OBJDIR)/usedef.cmx  $(OBJDIR)/pretty.cmx \
     $(OBJDIR)/inthash.cmx  $(OBJDIR)/errormsg.cmx   $(OBJDIR)/dataflow.cmx \
     $(OBJDIR)/cil.cmx   $(OBJDIR)/cfg.cmx
