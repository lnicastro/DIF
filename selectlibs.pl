while (<>) {
    chomp;
    $i = 0;
    foreach $a (split(/ +/)) {
	if ($a=~/-l.*/) {
	    $a =~ s/^-l//;
	    $b[$i++] = $a;
	}
    }
}
@b = reverse(@b);
$c = join(' ', @b);
print "$c \n";
