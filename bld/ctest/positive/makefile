ctest_name = positive

test_flags = -we-w0 -Isource

!ifdef test1
test_flags_386 = -d2-mf
test_flags_i86 = -d2-mc-2
test_flags_axp = -d2-si
alt_error = 01
!else ifdef test2
test_flags_386 = -d1-oxtan-mf
test_flags_i86 = -d1-oxtan-mm
test_flags_axp = -d1-oxtan-si
alt_error = 02
!else ifdef test3
test_flags_386 = -d2-ms
test_flags_i86 = -d2-ml
test_flags_axp = -d2
alt_error = 03
!else ifdef test4
test_flags_386 = -d1-oxs-mf
test_flags_i86 = -d1-oxs-ms
test_flags_axp = -d1-oxs-si
alt_error = 04
!else ifdef test5
test_flags_386 = -d1-ei-mf
test_flags_i86 = -d1-ei-mc-2
test_flags_axp = -d1-ei
alt_error = 05
!else ifdef test6
test_flags_386 = -d2-mf
test_flags_i86 = -d2-mm
test_flags_axp = -d2
alt_error = 06
!else ifdef test7
test_flags_386 = -d1-ms-6s-fp6
test_flags_i86 = -d1-ml-3-fp3
test_flags_axp = -d1
alt_error = 07
!else ifdef test8
test_flags_386 = -d2-mf-zro
test_flags_i86 = -d2-ms-zro
test_flags_axp = -d2
alt_error = 08
!else ifdef test9
test_flags_386 = -d1-oe-zri-mf
test_flags_i86 = -d1-oe-zri-ms
test_flags_axp = -d1-oe
alt_error = 09
!else
test_flags_386 = -ox-d1-ms
test_flags_i86 = -ox-d1-ms
test_flags_axp = -ox-d1
alt_error = 00
!endif

!include testenv.mif
!include ../positive.mif

!ifeq alt_error 00
add_br_ = $(targ_br_$(targ_env_$(arch)))
!else ifeq alt_error 01
add_br_ = $(targ_br_$(targ_env_$(arch)))
!endif

test_flags += $(add_br_$(no_br_$^*))

stack_i86 = opt stack=12k

.c: source
.p: source

test : .SYMBOLIC start test.lnk $(positive_exes) test_set_result
    $(noecho)%make common_set_result_check

check_result : .procedure
    -diff -b error.out error.chk

test_set_result : .SYMBOLIC
    -@%write error.out OK
    -chk_exec exec.out >> error.out

clean : .procedure
    @%make echo_clean
    @%make common_clean
    @if exist failure $(noecho)rm -f failure
    @if exist *.i $(noecho)rm -f *.i
