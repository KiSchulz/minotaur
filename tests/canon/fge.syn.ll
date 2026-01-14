; CHECK: fcmp ule float %__canonArg1, %__canonArg

define i1 @func(float %a, float %b) {
  %cmp = fcmp uge float %a, %b
  ret i1 %cmp
}
