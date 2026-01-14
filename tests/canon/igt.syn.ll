; CHECK: icmp slt i32 %__canonArg1, %__canonArg

define i1 @func(i32 %a, i32 %b) {
  %cmp = icmp sgt i32 %a, %b
  ret i1 %cmp
}
