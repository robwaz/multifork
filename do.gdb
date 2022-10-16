start
set follow-fork-mode child

b *mf_block+232
commands
  set $rdx = 0
  x/gx $rbp - 0x58
  x/10i $rip
end

b *thread_entry + 34
commands
  x/4i $rip
  x/a $rsp
  x/a $rsp + 0x08
  c
end
c
