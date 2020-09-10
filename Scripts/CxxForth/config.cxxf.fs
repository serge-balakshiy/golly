: step ( n -- )
	."  " . ." )" .s cr
;

\ 0 step 
variable ffl.endian   1 ffl.endian !

8  constant #bits/byte   ( -- n = Number of bits in a byte )

#bits/byte 1 chars * constant #bits/char   ( -- n = Number of bits in a char )

#bits/byte 1 cells * constant #bits/cell   ( -- n = Number of bits in a cell )

ffl.endian c@ 0=
constant bigendian?   ( -- flag = Check for bigendian hardware )

include tlb.cxxf.fs

( Extension words )

begin-enumeration
  enum: exp-index-out-of-range
  enum: exp-invalid-state
  enum: exp-no-data
  enum: exp-invalid-parameters
  enum: exp-wrong-file-type
  enum: exp-wrong-file-version
  enum: exp-wrong-file-data
  enum: exp-wrong-checksum
  enum: exp-wrong-length
  enum: exp-invalid-data
  enum: exp-invalid-memory
end-enumeration

: throw ( n -- )
	dup exp-index-out-of-range -
	if dup exp-invalid-state -
		if dup exp-no-data -
			if dup exp-invalid-parameters -
				if dup exp-wrong-file-type
					if dup exp-wrong-file-version -
						if dup exp-wrong-file-data -
							if dup exp-wrong-checksum -
								if dup exp-wrong-length -
									if exp-invalid-data -
										if exp-invalid-memory -
											if else s" Invalid mempry allocated" abort-message then
										else s" Invalid data" abort-message	then
									else	s" Wrong length" abort-message then
								else s" Wrong checksum" abort-message then
							else s" Wrong file data" abort-message then
						else s" Wrong file version" abort-message then
					else s" Wrong file abort-message" abort-message then
				else s" Invalid parameters" abort-message then
			else s" No data available"  abort-message then
		else s" Invalid state" abort-message then
	else s" Index out of range" abort-message then
	exit
;
: throw? ( n n -- )
	swap 0=
	if drop exit else throw then
;
