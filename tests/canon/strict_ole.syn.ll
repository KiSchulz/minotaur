; CHECK: fcmp olt float %__canonArg, 0x00000001

define i1 @func(float %a) {
  %cmp = fcmp ole float %a, 0.0
  ret i1 %cmp
}
