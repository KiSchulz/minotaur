; CHECK: icmp slt i32 %__canonArg, 1

define i1 @func(i32 %a) {
  %cmp = icmp sle i32 %a, 0
  ret i1 %cmp
}
