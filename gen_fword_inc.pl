#!/usr/bin/perl

my @fwords;
my @imm_words;

my $gen_type = shift;

while (<>) {
    chomp;
    next if !m/^FWORD/;
    s/^FWORD_DO\(/(do_/;
    s/^FWORD_//;
    s/^FWORD//;
    if (m/^IMM/) { push(@imm_words, $_); }
    else         { push(@fwords, $_); }
}

if ($gen_type eq "-c") {
    #
    # Create the C code: a list of addresses of fword headers.  That list
    # will be indexed [0] ... [i-2] with the [i-1]th element containing
    # NULL.  (But this code doesn't generate the NULL entry.)
    #
    print "/* C code: References to the fword_xyz_header's */\n";
    while ($#fwords >= 0) {
        $_ = pop @fwords;
        s/^\(([^,)]+).*$/fcode_$1_header/;
        s/^2\(([^,)]+).*$/fcode_$1_header/;
        print "    &$_,\n";
    }

    while ($#imm_words >= 0) {
        $_ = pop @imm_words;
        s/^IMM2?\(([^,)]+).*$/fcode_$1_header/;
        print "    &$_,\n";
    }
} else {
    #
    # Create the header list of fcpde (which are defined in fprimitives.c)
    # and which need to be available in forth.c.
    #
    print "/* Header code: Declarations for the fcode_xyz()'s */\n";
    while ($#fwords >= 0) {
        $_ = pop @fwords;
        s/^2?\(([^,)]+).*$/fheader_t fcode_$1_header;/;
        print "$_\n";
    }
}
