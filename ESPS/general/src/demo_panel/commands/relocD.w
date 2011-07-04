# @(#)relocD.w	1.2 6/18/91 ERL - waves command script for self modifying panel
shell awk '/make/ {print $10, $12, ($14 + 100)}' commands/reloc.w > reloc.tmp
shell awk '{print "waves make_panel name reloc file reloc.wb title Self-Alteration columns", $1, "loc_x", $2, "loc_y", $3}' reloc.tmp > commands/reloc.w
shell echo return >> commands/reloc.w
call commandname commands/reloc.w
return
