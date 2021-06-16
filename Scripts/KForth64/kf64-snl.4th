
\ include ans-words.4th
\ include strings.4th
\ include utils.4th
\ include struct.4th
\ include struct-ext.4th

\ include kf64-tlb.4th
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
0 ptr remove.first

: snl-init ( snl -- = Initialise the snl list )
    dup snl>first nil!
    dup snl>last nil!
    snl>length 0!
;

: snl-new ( -- snl = Create a new snl list on the heap )
    snl% %alloc dup snl-init
    \ dup to snl-ptr snl-ptr
;

: snl-(free)   ( xt scl -- = Free the nodes from the heap using xt )
  swap
    BEGIN
	over remove.first
	execute
	nil<>? \ snl.remove-first nil<>?      \ while remove first node <> nil do
    WHILE
	    over execute                      \   Free the node
  REPEAT
  2drop
;

: snl-free     ( snl -- = Free the list from the heap )
    ['] snn-free over \ to snl-ptr snl-ptr
    snl-(free)
    free  throw
;

: snl-length@ ( snl -- u = Get the number of nodes in the list )
    snl>length @
;

: snl-empty? ( snl -- flag = Check for on empty list )
    snl-length@ 0=
;
\ 500 step
: snl-first@   ( snl -- snn | nil  = Get the first node from the list )
    snl>first @
    to snl-ptr
    snl-ptr
;
\ 501 step

: snl-last@    ( snl -- snn | nil = Get the last node from the list )
    snl>last @
    to snl-ptr
    snl-ptr
;

\ This should be checked. May not work.
: snl-first! ( addr snn -- )
    snl-first@ !
;

( Private words )
  
: snl+offset   ( n1 n2 -- n3 = Determine the offset n3 from the index n1 and length n2, incl. validation )
  tuck index2offset                    \ convert to offset
  dup rot 0 within                     \ check outside 0..length-1
  throw \ exp-index-out-of-range AND throw     \ raise exception
;

: snl-node  ( n snl -- snn1 | nil  snn2 | nil = Get the nth node in the list, return this node snn2 and its leading node snn1 )
    nil -rot                   \ prv  = nil
    snl-first@              \ cur  = first
    swap                       \ S: prv cur off
    BEGIN
	2dup 0> swap nil<> AND   \ while n>0 and cur<> nil do
    WHILE
	    1- >r                    \  off--
	    nip                      \  prv = cur
	    dup snn-next@            \  cur = cur->next
	    r>
    REPEAT
    drop
;



\ 502 step
( List words )

: snl-append   ( snn snl -- = Append the node snn to the list )
    dup  snl>length 1+!        \ snl.length++
    over swap
    dup snl-first@ nil= IF     \ If snl.first = nil Then
	2dup snl>first !         \   snl.first = snn
    THEN
    snl>last @!       \ snl.last = snn
    dup nil<> IF               \ If snl.last != nil Then
	2dup snn-next!           \   snl.last.next = snn
    THEN
    drop snn>next nil!         \ snn.next = nil
;

: snl-prepend  ( snn snl -- = Prepend the node snn in the list )
    dup  snl>length 1+!        \ snl.length++
    over swap
    dup snl-last@ nil= IF      \ If snl.last = nil Then
	2dup snl>last !          \   snl.last = snn
    THEN
    snl>first @!               \ snl.first = snn
    swap snn-next!             \ snn.next = snl.first
;

: snl-insert-after  ( snn1 snn2 snl -- = Insert the node snn1 after the reference node snn2 in the list )
    dup snl>length 1+!
    >r
    over swap snn>next @!      \ ref.next = new
    2dup swap snn-next!        \ new.next = ref.next
    r>
    swap nil= IF               \ If ref.next = nil Then
	snl>last !               \   snl.last = new
    ELSE
	2drop
    THEN
;



: snl-remove-first   ( snl -- snn | nil = Remove the first node from the list, return the removed node )

  dup snl-first@ to snl-ptr snl-ptr
  dup nil<> IF               \ If first <> nil Then
    2dup snn-next@
    dup nil= IF              \   If first.next = nil Then
      over snl>last nil!     \     last.nil
    THEN
    swap snl>first !         \   first = first.next
    swap snl>length 1-!      \   length--
  ELSE
    nip
  THEN
;

' snl-remove-first to remove.first  remove.first is snl.remove-first

: snl-remove-after   ( snn1 snl -- snn2 | nil = Remove the node after the list, return the removed node )
  swap
  dup nil= throw \ exp-invalid-parameters AND throw
  dup snn-next@ >r                \ S: snl prv
  r@ nil<> IF                     \ If prv.next <> nil Then
    nil r@ snn>next @!            \   prv.next.next = nil
    dup nil= IF                   \   If prv.next.next = nil Then
      >r
      2dup swap snl>last !        \     snl.last = prv
      r>
    THEN
    swap snn-next!                \   prv.next = prv.next.next
    snl>length 1-!
  ELSE
    2drop
  THEN
  r>
;

( Index words )

: snl-index?   ( n snl -- flag = Check if the index n is valid in the list )
  snl-length@
  tuck index2offset
  swap 0 swap within
;


: snl-get      ( n snl -- snn = Get the nth node from the list )
  tuck snl-length@ snl+offset     \ S: snl offset
  swap snl-node nip               \ S: snn | nil
;



: snl-insert   ( snn n snl -- = Insert a node before the nth node in the list )
    tuck snl-length@ 1+ snl+offset  \ S: snn snl offset
    ?dup 0= IF
	snl-prepend
    ELSE
	over snl-length@ over = IF
	    drop snl-append
	ELSE                          \ Insert the new node
	    over 
	    snl-node drop          \ S: snn snl prv | nil
	    dup  nil= throw \ exp-invalid-state AND throw
	    swap snl-insert-after       \ Insert after prv
	THEN
    THEN
;

: snl-delete   ( n snl -- snn = Delete the nth node from the list, return the deleted node )
  tuck snl-length@ snl+offset     \ S: snl offset
  ?dup 0= IF                      \ If offset = 0 Then
    snl-remove-first              \   First node
  ELSE                            \ Else
    over snl-node drop            \   Offset -> node S: snl prv
    swap snl-remove-after         \   Remove after prv
  THEN                            \ S: snn
;

( LIFO words )

: snl-push     ( snn snl -- = Push the node snn at the top of the stack [= start of the list] )
  snl-prepend
;

  
: snl-pop      ( snl -- snn | nil = Pop the node at the top of the stack [= start of the list], return the popped node )
  snl-remove-first
;


: snl-tos      ( snl -- snn | nil = Get the node at the top of the stack [= start of the list], return this node )
  snl-first@
;


( FIFO words )

: snl-enqueue  ( snn snl -- = Enqueue the node snn at the start of the queue [=end of the list] )
  snl-append
;


: snl-dequeue  ( snl -- snn | nil = Dequeue the node at the end of the queue [= start of the list], return this node )
  snl-remove-first
;


( Special words )
\ 504 step
: snl-execute      ( i*x xt snl -- j*x = Execute xt for every node in list )
  snl-first@ to snl-ptr snl-ptr                \ walk = first
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

: snl-execute?     ( i*x xt snl -- j*x flag = Execute xt for every node in the list or until xt returns true, flag is true if xt returned true )
  snl-first@ to snl-ptr snl-ptr                \ walk = first
  false                      \ keep searching
  BEGIN
    over nil<> over 0= AND   \ while walk <> nil and keep searching do
  WHILE
    drop
    2>r
    2r@ swap execute         \   execute xt with node
    2r>
    snn-next@                \   walk = walk->next
    rot                      \   keep searching to tos
  REPEAT
  nip nip
;

: snl-reverse  ( snl -- = Reverse or mirror the list )
  nil over
  snl-first@                 \ walk = first
  BEGIN
    nil<>?
  WHILE                      \ while walk<>nil do
    dup snn-next@
    >r
    tuck snn-next!           \  walk->next = prev
    r>
  REPEAT
  drop
  dup  snl-first@
  over dup snl-last@
  swap snl>first !           \ first = last
  swap snl>last  !           \ last  = first
;

( Private sort words )

: snl-merge-qmove  ( size p^ -- psize qsize p^ q^ = Move the q^ based on p^ and size )
  2dup
  BEGIN                           \ S:qsize p^ psize q^
    2dup nil<> AND
  WHILE                           \ Move q^ until nil or size
    snn-next@
    >r 1- r>
  REPEAT
  >r >r over r> - -rot r>         \ Rearrange the stack
;


: snl-merge-node   ( snn snl -- = Append the snn node to the snl list during sorting )
  dup snl-last@ nil<> IF          \ If last <> nil Then
    2dup snl-last@ snn>next !     \   last->next = node
  ELSE                            \ Else
    2dup snl>first !              \   first = node
  THEN
  snl>last !                      \ last = node
;


: snl-merge-sort   ( psize qsize p^ q^ result snl -- psize qsize p^ q^ = Based on the compare result move p^ or q^ to the list )
  >r
  0> IF                \ If p^ bigger Then
    dup r> snl-merge-node
    snn-next@          \  Append q^ and move to next
    2>r 1- 2r>
  ELSE                 \ Else
    over r> snl-merge-node
    >r
    snn-next@          \  Append p^ and move to next
    2>r 1- 2r>
    r>
  THEN
;

: snl-merge-nodes  ( psize p^ snl -- 0 p^ | psize nil == Move all nodes from p^ in the list )
  >r
  BEGIN                           \ S:psize p^
    2dup nil<> AND
  WHILE                           \ While nodes in psize
    dup r@ snl-merge-node
    snn-next@                     \ Append them in the list
    >r 1- r>
  REPEAT
  rdrop
;

( Sort word )

: snl-sort         ( xt snl -- = Sort the list snl using mergesort, xt compares the nodes )
  >r >r
  1
  BEGIN                      \ S:size
    0
    r'@ snl-first@           \ p^
    r'@ snl>first nil!
    r'@ snl>last  nil!
    BEGIN                    \ S:size steps p^
      dup nil<>
    WHILE
      >r 1+ over r>          \ steps++
      snl-merge-qmove        \ Move the q^ max. size nodes
      BEGIN                  \ S:size steps psize qsize p^ q^
        2over 0>    AND
        over  nil<> AND
      WHILE
        2dup r@ execute      \ Compare the nodes
        r'@ snl-merge-sort   \ Merge the node from p^ or q^ based on compare result
      REPEAT
      rot swap 2swap         \ S:size steps qsize q^ psize p^
      r'@ snl-merge-nodes    \ Merge p^ if still nodes present
      2drop                  \ psize p^
      r'@ snl-merge-nodes    \ Merge q^ if still nodes present
      nip                    \ Save q^ as p^, drop qsize
    REPEAT
    drop                     \ p^
    r'@ snl-last@ snn>next nil!  \ last->next = nil
    >r 2* r>                 \ size*=2
  2 < UNTIL                  \ until steps < 2
  drop                       \ size
  rdrop rdrop
;


( Inspection )
\ 506 step
: snl-dump ( snl -- =Dump the list )
    ." snl : " dup . cr
    ." first : " dup snl>first ? cr
    ." last : " dup snl>last ? cr
    ." length : " dup snl>length ? cr
    ['] snn-dump swap snl-execute cr
;
\ 508 step
\ ==============================================================================
\ 100 step

0 [IF]
\ include strings.4th
\ include utils.4th
\ include struct.4th
\ include struct-ext.4th

\ include kf64-tlb.4th
\ include kf64-snn.4th
    include kf64-tst.4th
    include kf64-tst-snl.4th
    
    \ .( Testing: snl, snn and sni )
\    0 ptr ptst-snl
\    t{ snl-new to ptst-snl  }t 
\    t{ ptst-snl snl-length@ ?0 }t
\    t{ ptst-snl snl-empty? ?true }t
    
    snl-new ptr snl-p1
    .( snl dump ) cr
    snl-p1 snl-dump cr
[THEN]

\ 102 step
