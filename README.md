# TSort - Text file sorting program
Copyright (c) 1999 Trevor Barnett

This is an old C sorting program I discovered in the archive. It takes a text
file as input and sorts the lines, much like the sort function in any good text
editor.

Sort can be configured with various typical options, such as ascending and
descending alphabetical, and start from specific column. 
A slightly unusual feature is the ability to sort blocks of lines.  Block sort
groups lines and sorts on a specific line within the block but keeping lines
within each block in the original relative order.  By default sort is on the
first line in each block, but line within block can be specified via a command
line parameter (-l). A possible use could be to sort a text-based records file
containing a fixed set of fields one per line as key-value pairs.
