\ ioctl.4th
\
\ Ported from Linux 2.6.x, /usr/include/asm-i386/ioctl.h 
\ (for use in Forth interfaces to device drivers)
\
\ K. Myneni, 2006-08-09
\
\ Revisions:
\
0 constant nil

[undefined] 0! [IF] : 0! ( a-addr -- = Set address to zero )
    0 swap !
;
[THEN]
: nil! ( a-addr -- = Set address to nil )
    nil swap !
;

[UNDEFINED] nil= [IF]
    : nil= ( addr -- flag = Check for nil )
	nil =
    ;
[THEN]

[UNDEFINED] nil<> [IF]
    : nil<> ( addr -- flag = Check for unequal to nil )
	nil <>
    ;
[THEN]

[UNDEFINED] nil<>? [IF]

0 nil= [IF]
: nil<>?    ( addr -- false | addr true = If addr is nil, then return false, else re )
  state @ IF
    postpone ?dup
  ELSE
    ?dup
  THEN
; immediate
[ELSE]
: nil<>?
  dup nil<> IF
    true
  ELSE
    drop
    false
  THEN
;
[THEN

[THEN]


[UNDEFINED] 1+! [IF]
    : 1+! ( a-addr -- = Increase contents of address by 1 )
	1 swap +!
    ;
[THEN]

[UNDEFINED] @! [IF]
    : @! ( x1 a-addr -- x2 = First fetch the contents and then store value x1 )
	dup @ -rot !
    ;
[THEN]

