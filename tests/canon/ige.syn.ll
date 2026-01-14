; CHECK: icmp sle i32 %__canonArg1, %__canonArg

define i1 @func(i32 %a, i32 %b) {
  %cmp = icmp sge i32 %a, %b
  ret i1 %cmp
}
