; CHECK: fcmp olt float 0x80000001, %__canonArg

define i1 @func(float %a) {
  %cmp = fcmp oge float %a, 0.0
  ret i1 %cmp
}
