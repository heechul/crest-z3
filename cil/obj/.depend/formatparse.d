  $(OBJDIR)/formatparse.cmo:  $(OBJDIR)/pretty.cmi   $(OBJDIR)/lexerhack.cmo \
     $(OBJDIR)/errormsg.cmi  $(OBJDIR)/cil.cmi   $(OBJDIR)/formatparse.cmi
  $(OBJDIR)/formatparse.cmx:  $(OBJDIR)/pretty.cmx   $(OBJDIR)/lexerhack.cmx \
     $(OBJDIR)/errormsg.cmx  $(OBJDIR)/cil.cmx   $(OBJDIR)/formatparse.cmi
