#!/usr/bin/perl

my @fwords;
my @imm_words;

while (<>) {
    chomp;
    next if !m/^FWORD/;
    next if m/^FWORD_DO/;
    s/^FWORD_//;
    s/^FWORD//;
    if (m/^IMM/) { push(@imm_words, $_); }
    else         { push(@fwords, $_); }
}

while ($#fwords >= 0) {
    $_ = pop @fwords;
    s/^\(([^,)]+).*$/fword_$1_header/;
    s/^2\(([^,)]+).*$/fword_$1_header/;
    print "    &$_,\n";
}

while ($#imm_words >= 0) {
    $_ = pop @imm_words;
    s/^IMM2?\(([^,)]+).*$/fword_$1_header/;
    print "    &$_,\n";
}
