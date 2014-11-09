MemoryPatternSearch
===================

Algorithm for finding an address in memory that contains a certain byte array pattern.

***Main code is in dllmain.cpp in MemoryPatternSearch directory.***

== PATTERN SEARCH ALGORITHM ==

If you have the byte array of a function you want to find in a game,
or any program by that matter (that doesn't have anti-debugging),
then you can use this function to find the address where that byte array occurs.

I will provide an example of its utility with a "shotbot program".
You can also use this to find addresses of other things, like infinite ammo, player health, enemy health, etc.,
as long as you know the byte array of the structure that contains such information.

See repository Gunz2PAQ on how to actually write to these addresses once you have obtained them.
(This algorithm can enable "auto-updating" of hacks given that you know the byte array pattern of the place you are doing your memory writing.) The "mask" allows for flexibility in the byte array.
