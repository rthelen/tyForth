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
    print $obj->word
  end
  if $obj->type > 8
    printf "Unknown\n"
  end
end
document pobj
pobj <fobj_t *>

pobj will print the object as whatever type it is.
end
