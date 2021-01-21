
\ include ~/projects/gh/kForth-64/forth-src/strings.4th

\ include ans-words.4th
\ include strings.4th
\ include utils.4th
\ include struct.4th
\ include struct-ext.4th
\ include kf64-tlb.4th

[undefined] step [IF]
    : step ( n -- )
	. ." )step" .s \ cr
    ;
[THEN]

[undefined] value [IF] : value constant ; [THEN]
: value! >body ! ;
[undefined] to [IF] : to state @ if
    postpone ['] postpone value! 
    else
	' value!
    then
; immediate
[THEN]

[undefined] a@ [IF]  : a@ @ ; [THEN]

[undefined] ptr [IF] : ptr create 1 cells allot? ! does> a@ ; [THEN]

0 ptr snn-ptr

struct
    cell% field snn>next
end-struct snn%

32 step
: snn-init ( snn -- = Initialise the node )
    snn>next nil!
;
34 step
: snn-new ( -- snn = Create a new node on the heap )
    snn% %alloc dup snn-init dup to snn-ptr
;
36 step
: snn-free ( snn -- = Free then node from the heap )
    free
;
38 step
: snn-next@ ( snn1 -- snn2 = Get for the node snn1 the next node to snn2 )
    snn>next @ to snn-ptr
    snn-ptr
;
40 step
: snn-next!    ( snn1 snn2 -- = Set for the node snn1 the next node to snn2 )
    snn>next to snn-ptr
    snn-ptr !
;
42 step
( Inspection )

: snn-dump     ( snn -- = Dump the single list node )
  ." snn: " dup . cr
  ." next: "     snn>next  ? cr
;

\ ==============================================================================
100 step

0 [IF]
    snn-new ptr snn-p1
    .( snn dump ) cr
    snn-p1 snn-dump cr
[THEN]

102 step
