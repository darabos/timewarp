#!/usr/bin/perl

#add menus to static html pages
#
#This program opens a file called "menu.html" and remembers its contents.  Then
#the files specified on the commandline are scanned for an HTML tag like this:
#<!--MENU-->
#If the file contains two such tags, the contents of menu.html will clobber
#whatever appears between the tags, and leave the rest of the file untouched.

$size = @ARGV;
if ($size == 0) {
  print "usage: perl InsertMenu.pl FILES_TO_SCAN\n\n";
  print "Add menus to static html pages\n\n";
  print "Will scan the specified files (wildcards allowed) for the HTML tag:\n";
  print "<!--MENU-->\n";
  print "If the file contains two such tags, the contents of menu.html will clobber\n";
  print "whatever appears between the tags, and leave the rest of the file untouched.\n\n";
  print "menu.html must appear in the current directory.\n";
}


open MENU, "menu.html";
push @menu, <MENU>;
close MENU;

foreach $file (@ARGV) {
    print "Processing $file...";
    #delete @lines [0..-1];
    @lines = ();
    $skipping = 0;
    $redMenu = 0;

    open FILE, $file;
    foreach $line (<FILE>) {
        if ($line =~ /<!--MENU-->/)
        {
            $skipping = 1 - $skipping;
            if (skipping && ! $redMenu) {
                print "adding menu...";
                push @lines,"<!--MENU-->";
                $redMenu = 1;
                push @lines, @menu;
                
            }
        }
        if (! $skipping) 
        { 
            push @lines, $line;
        }
    }
    close FILE;
    print "\n";

    
    open OUTPUT, ">$file";
    foreach $line (@lines)
    { print OUTPUT $line ; }
    close OUTPUT;
}


