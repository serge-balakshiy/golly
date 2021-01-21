include ans-words.4th
include strings.4th
include utils.4th
include struct.4th
include struct-ext.4th

include kf64-tlb.4th
include kf64-snn.4th

[undefined] step [IF]
    : step ( n -- )
	. ." )step" .s \ cr
    ;
[THEN]
\ include ~/projects/gh/kForth-64/forth-src/strings.4th

struct
    cell% field snl>first
    cell% field snl>last
    cell% field snl>length
end-struct snl%

0 ptr snl-ptr

defer snl.remove-first


: snl-init ( snl -- = Initialise the snl list )
    dup snl>first nil!
    dup snl>last nil!
    snl>length 0!
;

: snl-new ( -- snl = Create a new snl list on the heap )
    snl% %alloc dup snl-init
    dup to snl-ptr snl-ptr
;

: snl-length@ ( snl -- u = Get the number of nodes in the list )
    snl>length @
;

: snl-empty? ( snl -- flag = Check for on empty list )
    snl-length@ 0=
;
500 step
: snl-first@   ( snl -- snn | nil  = Get the first node from the list )
    snl>first @ to snl-ptr
    snl-ptr
;
501 step

: snl-last@    ( snl -- snn | nil = Get the last node from the list )
    snl>last @ to snl-ptr
    snl-ptr
;

\ This should be checked. May not work.
: snl-first! ( addr snn -- )
    snl-first@ !
;

502 step
    
: snl-append   ( snn snl -- = Append the node snn to the list )
    dup  snl>length 1+!        \ snl.length++
    over swap
    dup snl-first@ nil= IF     \ If snl.first = nil Then
	2dup snl-first@ !         \   snl.first = snn
    THEN
    snl-last@ !                \ snl.last = snn
    dup nil<> IF               \ If snl.last != nil Then
	2dup snn-next!           \   snl.last.next = snn
    THEN
    drop snn>next nil!         \ snn.next = nil
;

( Special words )
504 step
: snl-execute      ( i*x xt snl -- j*x = Execute xt for every node in list )
  snl-first@                 \ walk = first
  BEGIN
    nil<>?                   \ while walk<>nil do
  WHILE
    2>r 
    2r@ swap execute         \  execute xt with node
    2r>
    snn-next@                \  walk = walk->next
  REPEAT
  drop
;

( Inspection )
506 step
: snl-dump ( snl -- =Dump the list )
    ." snl : " dup . cr
    ." first : " dup snl>first ? cr
    ." last : " dup snl>last ? cr
    ." length : " dup snl>length ? cr
    ['] snn-dump swap snl-execute cr
;
508 step
\ ==============================================================================
100 step

1 [IF]
\    snn-new ptr snn-p1
\    .( snn dump ) cr
\    snn-p1 snn-dump cr
[THEN]

102 step
