# Contents
This folder contains the IMPLEMENTATIONS for the various necessary ticklites.

You can find the Ticklite template facade in Essential Types. A Ticklite combines:
-  The Ticklite Facade and the base impl which provides a few functions most ticklites use.
-  An implementation for the Tick_ and HACK_ functions used by the template types.
-  The Ticklites thread

And this creates a ticklite object. This is a typesafe and encapsulating way
of presenting a tickable function. It requires that any side-effects happen
only during the Apply function, and that it be fully thread-agnostic.

Thread safety is of course recommended, but like, you do you I guess.  
  
Finally, note that while ticklite templates currently use pointers, this may have to change to a key driven system to fully support  
rollback and determinism without also requiring unusual lifecycle management by users. The other option we're considering  
is providing pool or slab allocation for tickable_impls and memory_blocks. Ultimately, both approaches are of interest.  

# Reminder: Ticklites are _not_ run on the game thread.
They can trigger things that are by eventing against the Artillery Dispatcher, and many of the apply helpers provided do 
end up executing on the game thread, but this is going to change over time.
Do not write code that assumes you are on the gamethread. The good news is that because
Ticklites are data configurable, it probably won't be that much of an issue. Most ticklites should already be written by
that point in time.

# Reminder: Ticklite resim is needed for multiplayer
Again, we ship with most of the ticklites you're likely to want and Game Sim abilities use read-only records of prior states to allow safe multithreaded access in a novel and elegant way. So this isn't as bad as it sounds. But it does mean that reset needs to work 100% of the time, and Apply must be very fast.

## Best Practices
I strongly recommend including the following header comment in any ticklite implementation:
```c++
//A ticklite's impl component(s) must provide:
//TICKLITE_StateReset on the memory block aspect
//TICKLITE_Calculate on the impl aspect
//TICKLITE_Apply(MemoryBlock*) on the impl aspect, consuming the memory block aspect's state
//TICKLITE_CoreReset on the impl aspect
//TICKLITE_CheckForExpiration on the impl aspect, though this should use one of the standard helpers.
//TICKLITE_OnExpiration, though this can be a no-op.

```
I also recommend specifying if this tickable will use containment, facade, or composition with the following:
```c++
/* CHOICE OF INHERITANCE */
```