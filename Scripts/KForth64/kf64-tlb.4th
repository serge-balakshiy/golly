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

[UNDEFINED] 3dup [IF]
: 3dup    ( x1 x2 x3 -- x1 x2 x3 x1 x2 x3 = Duplicate the three cells on stack )
  dup 2over rot
;
[THEN]


[UNDEFINED] 4dup [IF]
: 4dup    ( x1 x2 x3 x4 -- x1 x2 x3 x4 x1 x2 x3 x4 = Duplicate the four cells on stack )
  2over 2over
;
[THEN]

[UNDEFINED] 3drop [IF]
: 3drop   ( x1 x2 x3 -- = Drop the three cells on stack )
  2drop drop
;
[THEN]

[UNDEFINED] 4drop [IF]
: 4drop   ( x1 x2 x3 x4 -- = Drop the four cells on stack )
  2drop 2drop
;
[THEN]

[UNDEFINED] d<> [IF]
: d<>              ( d d - f = Check if two two double are unequal )
  d= 0=
;
[THEN]

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
: nil<>?    ( addr -- false | addr true = If addr is nil, then return false, else return address with true )
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

[UNDEFINED] 1-! [IF]
: 1-!   ( a-addr -- = Decrease contents of address by 1 )
  -1 swap +!
;
[THEN]

[UNDEFINED] @! [IF]
    : @! ( x1 a-addr -- x2 = First fetch the contents and then store value x1 )
	dup @ -rot !
    ;
[THEN]

[UNDEFINED] index2offset [IF]
: index2offset   ( n1 n2 -- n3 = Convert the index n1 range [-n2..n2> into offset n3 range [0..n2>, negative values of n1 downward length n2 )
  over 0< IF
    +
  ELSE
    drop
  THEN
;
[THEN]

[UNDEFINED] rdrop [IF]
: rdrop            ( R: x -- = Drop the first cell on the return stack )
  postpone r> postpone drop
; immediate
[THEN]

[UNDEFINED] r'@ [IF]
: r'@              ( R: x1 x2 -- x1 x2; -- x1 = Fetch the second cell on the return stack )
  postpone 2r@ postpone drop
; immediate
[THEN]


