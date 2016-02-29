# string-alignment
Implementation of local and global alignment of strings in c.
Currently only global align has been succesfully implemented.

Compile: 
  Get all the files in the same directory and compile with "make global-align"
  
Run:
  Example command: global-align vintners writers --scores=M20I-1D-1R-1 --type=max
  Example output:
      Alineamiento Global.
      Str1:   vintners
      Str2:   writers
      
      Alineamiento número 1:
      Score:  96.000000
      Str1:   _ v i n t n e r s 
      Str2:   w r i _ t _ e r s 
      EditTr: I R M D M D M M M 
      
      Alineamiento número 2:
      Score:  96.000000
      Str1:   v _ i n t n e r s 
      Str2:   w r i _ t _ e r s 
      EditTr: R I M D M D M M M 
      
  Next changes:
  * Optimization of traceback algorithm. (Too slow, currently based on continuous resizing of array)
  * Implement local alignments
