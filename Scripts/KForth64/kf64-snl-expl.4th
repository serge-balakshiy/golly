\ include kf64-snn.4th
include strings.4th
include utils.4th
include struct.4th
include struct-ext.4th
include kf64-tlb.4th
include kf64-snn.4th
include kf64-snl.4th

1000 step
snl-new ptr plist
\ 0 ptr snl-ptr
struct
    snn%
    cell% field node>node
    cell% field node>num
end-struct nnode%
110 step
: nlist-insert ( n -- )
    \ 94 step
    0 do
	\ 96 step
	nnode% %alloc \ 98 step
	dup \ node>node
	snn-init
	\ 102 step
	dup I swap node>num !
	\ 104 step
	plist snl-append
    loop
;
drop drop
92 step
9 nlist-insert
0 ptr curptr
0 ptr ptr>node
\ plist @ snn>next to curptr
\ curptr .
\ curptr @ .
\ curptr @ to curptr
\ curptr .
\ curptr @ .
\ curptr @ to curptr
\ curptr .
\ curptr @ .
\ curptr @ to curptr

plist snl-dump
plist @ to curptr
curptr to ptr>node
curptr .
curptr @ .
curptr node>num @ .
curptr @ to curptr

curptr node>num @ .
curptr @ to curptr
curptr node>num @ .

: nnode-dump (  -- )
    plist @ to curptr
    plist snl-length@
    0 do
	." node>num: "
	curptr node>num @ . cr
	curptr @ to curptr
    loop
;
cr
nnode-dump

snl-new to plist
9 nlist-insert
plist snl-dump cr
nnode-dump cr
plist snl-free