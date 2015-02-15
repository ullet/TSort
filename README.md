# TSort - Text file sorting program
Copyright (c) 1999 Trevor Barnett

This is an old C sorting program I discovered in the archive. It takes a text
file as input and sorts the lines, much like the sort function in any good text
editor.

Sort can be configured with various typical options, such as ascending and
descending alphabetical, and start from specific column. 
Slightly unusual options are to start sorting at a specific line, perhaps to
skip over a file header, and to sort blocks of lines.  Block sort groups lines
and sorts by the first line in each block, keeping each line within the block in
the original order. Possible use could be to sort a text-based records file
containing sort key on first line and record fields on next lines in block.
