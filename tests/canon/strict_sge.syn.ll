; CHECK: icmp slt i32 -1, %__canonArg

define i1 @func(i32 %a) {
  %cmp = icmp sge i32 %a, 0
  ret i1 %cmp
}
