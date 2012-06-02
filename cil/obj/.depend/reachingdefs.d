  $(OBJDIR)/reachingdefs.cmo:  $(OBJDIR)/util.cmi   $(OBJDIR)/usedef.cmo \
     $(OBJDIR)/stats.cmi  $(OBJDIR)/pretty.cmi   $(OBJDIR)/liveness.cmo \
     $(OBJDIR)/inthash.cmi  $(OBJDIR)/errormsg.cmi   $(OBJDIR)/dataflow.cmi \
     $(OBJDIR)/cil.cmi
  $(OBJDIR)/reachingdefs.cmx:  $(OBJDIR)/util.cmx   $(OBJDIR)/usedef.cmx \
     $(OBJDIR)/stats.cmx  $(OBJDIR)/pretty.cmx   $(OBJDIR)/liveness.cmx \
     $(OBJDIR)/inthash.cmx  $(OBJDIR)/errormsg.cmx   $(OBJDIR)/dataflow.cmx \
     $(OBJDIR)/cil.cmx
