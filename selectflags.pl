while (<>) {
    chomp;
    foreach $a (split(/ +/)) {
	if ($a=~/-I.*/) {
	    print "$a ";
	}
	if ($a=~/-L.*/) {
	    print "$a ";
	}
    }
}
print "\n";
