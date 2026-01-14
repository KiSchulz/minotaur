; CHECK: icmp sle i32 %__canonArg, 2147483647

define i1 @func(i32 %a) {
  %cmp = icmp sle i32 %a, 2147483647
  ret i1 %cmp
}
