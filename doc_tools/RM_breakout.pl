#! /usr/bin/perl

    # ==========================================================================
    # Script Name:  breakout.pl
    #
    # Programmer:   Mike McGreevy
    #               <mamcgree@hdfgroup.org>
    #               Tuesday, February 10, 2009
    #
    # Description:  This script pulls the functions out of a given RM_H5*.html
    #               and puts them into individual files of the form
    #               <function_name>.htm, and locates them in their own
    #               subdirectory. A new file entitled RM_H5*.breakout.html is
    #               created containing php redirects to the new individual 
    #               function files where they were originally located.
    #
    # Notes:        This script does minimal error checking on input files,
    #               as it was written for a quick and dirty separation of the
    #               reference manual, and isn't going to be maintained after 
    #               the intitial separation (though may be saved for future
    #               reference). As such, use caution when specifying input files
    #               to ensure the correct file is sent through to be parsed.
    #    
    # ==========================================================================
    
 
    # Check that exactly one argument is given.
    if ($#ARGV != 0) {
        
        print "  Incorrect usage. Please use the form: ./breakout.pl <filename>\n";
        exit;
    }

    # Save needed file names 
    $inputfile = $ARGV[0];    
    
    # Process the file input name to determine output name(s).  
    # 
    #    This will create:
    #     $inputfilename = name of the input file, unedited.
    #     $no_ext = name of the input file minus its extension.
    #     $subdir_name = name of the subdirectory to store functions.  

    # Split before the '.' in the file name.
    @filenamesplit = split(/\./, $inputfile);

    # Store first portion into no_ext
    $no_ext = @filenamesplit[0];

    # Pull out the RM_ in the file name, if present.
    if ((index($no_ext, "RM_", -1) > -1)) {

        @filenamesplit = split(/RM_/, $no_ext);
        $subdir_name = @filenamesplit[1];

    } else {

        #if not present, the subdir_name is the same
        # as the filename minus extension.
        $subdir_name = $no_ext;
    }   
    
    # Open the input file.
    open(FILE, $inputfile);

    # Copy the file's lines into an array.
    @linesarray = <FILE>;

    # Remove '\n' from each line.
    foreach $line (@linesarray) {
        chomp $line
    }

    # Make note of the total number of lines in the file
    $count = @linesarray;

    # Initialize some tracking variables 
    #   $start = line on which a function starts
    #   $end = line on which a function ends
    #   $num_functions = number of functions already processed
    $start = -1;
    $end = -1;
    $num_functions = 0;

    # Open a file in which to write the new 'base' file. This is
    # where the php redirects to the function files will be located.
    open(BASE, ">$no_ext.breakout.html");

    # Create the subdirectory to store function files.
    mkdir $subdir_name;

    # =========================================================
    #     Main Loop:
    #      Scans down the file in search of function blocks.
    #
    #      Upon locating an entire block, it is written to its
    #      own new file, and a php link is written into the
    #      new base file.
    # =========================================================

    # Progress down the file one line at a time.
    for $i (0 .. $count) {

        # use the phrase "HEADER RIGHT" as a key to indicate function location.
        if (index($linesarray[$i], "HEADER RIGHT", -1) > 0) {

            # If we're currently collecting a function, close it off.
            if ($start > -1) {
        
                # This block indicates that we are already tracking a function.            
                # Write that function to a file before starting a new one.

                # specify end location.
                $end = $i - 2;
                
                print " -- Writing function $name (lines $start to $end in $inputfile) into $subdir_name/$name.htm\n";

                # Write the function into its own function file.
                open(FUNCTION, ">$subdir_name/$name.htm");            

                    for $j ($start .. $end) {
                        print FUNCTION "$linesarray[$j]\n";
                    }

                close(FUNCTION);

                # increment processed functions tracker
                $num_functions += 1;

                # write the php link into the new base file.
                print BASE qq(<?php include("$subdir_name/$name.htm"); ?>\n);

                #reset line tracking information for this function.
                $start = -1;
                $end = -1;
                $name = 0;
            }
            
            # Split the line containting HEADER RIGHT in order to pull the function name between the quotations.    
            @split_list = split('"', $linesarray[$i]);
            
            # store the function name.
            $name = @split_list[1];
            
            # Check to see if this is a function or if the name field is blank.
            if (($name eq " ") && ($num_functions > 0)) {
            # We've hit a line at the END of the file with " " as its function name.

                print "End of functions. $num_functions new function file(s) created.\n";

                #copy rest of file into base file.
                
                for $k ($i - 1 .. $count) {

                    print BASE "$linesarray[$k]\n";
                }    
                
            } elsif (($name eq " ")||($name eq "HDF5 Tools")) {
            # We've hit a line somewhere in the beginning of the file with " " as its function name.

                #do nothing, as we haven't reached any functions yet.

            } else {
            # We've hit a function line, marking the beginning of a function. Start tracking it.
                $start = $i - 1;

                # If we haven't written anything to a new file, then this is the first function. Copy
                # the original file into the base file up to this point.
                if ($num_functions == 0) {
                    for $k (0 .. $i - 2) {
                        print BASE "$linesarray[$k]\n";
                    }
                }

                # Pull the spaces out of the function name, if any are present 
                $name =~ s/ /_/g;

            } # end else

        } #end if

    } #end for
 
    # Close down the new base file.
    close(BASE);
