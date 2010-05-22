#
# GDB commands useful for debugging tyForth.
#
define pobj
  set $obj = $arg0
  printf "Type %d: ", $obj->type
  if $obj->type == 0
    print "Undefined"
  end
  if $obj->type == 1
    printf "Number: %lg\n", $obj->u.num.n
  end
  if $obj->type == 2
    printf "String: (len = %d) str = \"%s\"\n", $obj->u.str.len, $obj->u.str.buf
  end
  if $obj->type == 3
    printf "Table\n"
    print $obj->u.table
  end
  if $obj->type == 4
    printf "Array: %d elements\n", $obj->u.array.num
    print $obj->u.array
  end
  if $obj->type == 5
    printf "Hash: %d elements\n", $obj->u.hash.num_kv
    print $obj->u.hash
  end
  if $obj->type == 6
    printf "Stack: depth %d\n", $obj->u.stack.sp
    print $obj->u.stack
  end
  if $obj->type == 7
    printf "Index\n"
    print $obj->u.index
  end
  if $obj->type == 8
    printf "Word: Name = \"%s\"\n", $obj->u.word.name->u.str.buf
    print $obj->u.word
  end
  if $obj->type > 8
    printf "Unknown\n"
  end
end
document pobj
pobj <fobj_t *>

pobj will print the object as whatever type it is.
end

define fbreak
  break zzz
end
document fbreak
Set break points in FASSERT and fassert.
end

define mdump
  set $M = f->obj_memory
  printf "Number of free objects: %d\n", $M->num_free_objs
  printf "inuse_bitmap: "
  p/x $M->inuse_bitmap
  set $N = $M->num_free_objs
  set $MAX_I = 1024
  printf "Free object numbers: "
  if $N > (2 * $MAX_I)
    set $N = $MAX_I
    set $I = 0
    while $I < $N
      printf "%d, ", $M->next_free[$I]
      set $I = $I + 1
    end
    printf " ... "
    set $N = $M->num_free_objs
    set $I = $N - $MAX_I
    while $I < $N
      printf "%d, ", $M->next_free[$I]
      set $I = $I + 1
    end
    printf "\n"
  else
    set $I = 0
    while $I < $N
      printf "%d, ", $M->next_free[$I]
      set $I = $I + 1
    end
    printf "\n"
  end
  set $IDX = 0
  while $IDX < 1024
    if $M->inuse_bitmap[$IDX / 32] & (1 << ($IDX & 31))
      printf "In use object %d: ", $IDX
      pobj $M->objs[$IDX]
    end
    set $IDX = $IDX + 1
  end
end
document mdump
mdump
Print out the f->obj_memory in a useful fasion.  (f is extracted from the current scope.)
end

define pcode
  set $W = $arg0
  if $W->type != 8
    printf "Argument is not a word object:\n"
    pobj $W
  end
  if $W->u.word.code != fcode_do_colon
    printf "Word is not a colon definition.\n"
    pobj $W
  end
  set $offset=0
  while $offset < $W->u.word.body_offset
    printf "[%d]", $offset
    if $W->u.word.u.body[$offset].n != 0
      printf " (offset: %d)", $W->u.word.u.body[$offset].n
    end
    printf "\n"
    pobj $W->u.word.u.body[$offset].word
    set $offset = $offset + 1
  end
end
document pcode
pcode <w>

Print the code of colon defintion word <w>.  <w> must be a word where the code field points to fcode_do_colon.
end
