Toolkit for ripping the data file of the game 
"Seargent Slotte"


UNFINISHED - There are still some mysteries open for research:


DONE... Content in general:
- Game content is encrypted XOR with a single byte.
  For the MOD (which I was interested in the first place of coz ;-) )
  this was 0x77 but for other files this is another value.
- There must be some algorithm how this byte is calculated.. maybe summing
  up the ASCII values of the filename or something like that
- UPDATE 0:17 Algo found: for each char in filename k ^= char (initial value for k = 0)


  
The MOD:
- The file seems to be mainly Protracker but has a uncommon header 
  "CSM4" which also matches the filename SLOT.CSM
- Initial speed seems to be somewhere in the header since it runs too
  slow when "fixing" the header to M.K. - there is no initial F03 command
  (03 seems to be the correct speed for this tune)
- Maybe it is worth checking if this is a known format somehow... first
  googling brought no useful results...
- UPDATE 0:17 CSM format seems to belong to some company called CSW Goup 
  Ltd. - The sound drivers (.DRV) contain a copyright notice of this company
