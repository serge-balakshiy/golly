\ include kf64-snn.4th

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
    94 step
    0 do
	96 step
	nnode% %alloc 98 step
	dup node>node snn-init
	102 step
	dup I swap node>num !
	104 step
	plist snl-append
    loop
;
drop drop
92 step
3 nlist-insert