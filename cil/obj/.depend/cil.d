 $(OBJDIR)/cil.cmo:  $(OBJDIR)/util.cmi  $(OBJDIR)/pretty.cmi \
      $(OBJDIR)/machdep.cmo  $(OBJDIR)/inthash.cmi  $(OBJDIR)/escape.cmi \
     $(OBJDIR)/errormsg.cmi  $(OBJDIR)/clist.cmi   $(OBJDIR)/cilversion.cmo \
     $(OBJDIR)/alpha.cmi  $(OBJDIR)/cil.cmi
 $(OBJDIR)/cil.cmx:  $(OBJDIR)/util.cmx  $(OBJDIR)/pretty.cmx \
      $(OBJDIR)/machdep.cmx  $(OBJDIR)/inthash.cmx  $(OBJDIR)/escape.cmx \
     $(OBJDIR)/errormsg.cmx  $(OBJDIR)/clist.cmx   $(OBJDIR)/cilversion.cmx \
     $(OBJDIR)/alpha.cmx  $(OBJDIR)/cil.cmi
