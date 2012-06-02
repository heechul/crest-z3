  $(OBJDIR)/deadcodeelim.cmo:   $(OBJDIR)/usedef.cmo  $(OBJDIR)/stats.cmi \
      $(OBJDIR)/reachingdefs.cmo  $(OBJDIR)/pretty.cmi  $(OBJDIR)/inthash.cmi \
      $(OBJDIR)/expcompare.cmo  $(OBJDIR)/errormsg.cmi   $(OBJDIR)/ciltools.cmo \
     $(OBJDIR)/cil.cmi
  $(OBJDIR)/deadcodeelim.cmx:   $(OBJDIR)/usedef.cmx  $(OBJDIR)/stats.cmx \
      $(OBJDIR)/reachingdefs.cmx  $(OBJDIR)/pretty.cmx  $(OBJDIR)/inthash.cmx \
      $(OBJDIR)/expcompare.cmx  $(OBJDIR)/errormsg.cmx   $(OBJDIR)/ciltools.cmx \
     $(OBJDIR)/cil.cmx
