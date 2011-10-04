Likely publication targets: 
    Advanced Topics in HDF5

Original author and date: 
    RobB Matzke, 1999
    Quincey Koziol, 2000 revision

JIRA task: 7511 -- full task
           7759 -- "light" task

Files:
    hdf5doc/trunk/html/TechNotes/VFL.html
    hdf5doc/trunk/html/TechNotes/VFLfunc.html

Current state:
    This documentation currently consists of two files in "HDF5 Technical 
    Notes" (TechNotes):

    HDF5 Virtual File Layer
        (Proposal 1999-08-11)
        (VFL.html)

        This still reads as a proposal and would need to be edited to 
        convey the design and offer guidelines and instructions.

        Basic edit:  The basics of such an edit might require one to 
            two weeks.

        To fully review and complete this document would require more
        time, yet to be determined.

    List of HDF5 VFL Functions
        (VFLfunc.html)

        This is a simple list of function signatures.

        Basic edit: These signatures could be reviewed for accuracy and 
        any additional functions added with 2 to 8 hours of effort.
        (Does not incllude the time required to identify additional 
        functions, if any.)

        To fully document these functions would require several weeks of
        dedicated developer time.  Very rough initial estimate: 400 hours.

Reccommendation:
    [Based on discussion between Frank Baker and Dana Robinson, 3 Oct 2011.]

    -- The audience for this task is very small.
    -- The risk of a proliferation of mediocre file drivers is substantial.
    For example, several projects might try to a cloud driver.  Each 
    might do part of the job or "work well enough for <i>my</i> project".
    That might not be a desirable state of affairs.

    So the major investment offers a questionable return.

    On the other hand, it might be worth it to do the basic edit on both
    documents and move them from TechNotes to "Advanced Topics in HDF5."  
    That would make the functionality somewhat more visible without 
    offering too much encouragement to less experienced developers.

