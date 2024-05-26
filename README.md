# Artillery

Guns, GAS, and shields!

Complexity and access management primitives married to the core set of basic weapon abilities used to bind to a weapon actor. 
Built on top of GAS. And yes, I know the beetle is actually the bombardier beetle, formally.

https://github.com/tranek/GASDocumentation is a really useful place to start for understanding GAS. We're using it very slightly differently,
in a more composition oriented way that'll hopefully ease the dependency management and granularity issues we ran into at Riot if we go
with using Game Feature Plugins.
  
## Networking  
This plugin will need to accomplish four major goals:   
- Disable replication and RPCs for all played or used abilities TO the server.   
- Accept push updates from the server. As a result, replication FROM the server must still work.   
- Integrate with the reconciliation logic needed to do an eventually consistent server auth game. See 2.   
- Prevent known issues around latency and cooldown misbehavior in the normal up-replication by offlining it entirely if needed.   
- Ensure integration with Iris, which will be our replication provider for Lions.   
   
## Design   
This plugin has five main design goals:  
- All data is managed through Data-Driven Gameplay Elements or configs. See https://dev.epicgames.com/documentation/en-us/unreal-engine/data-driven-gameplay-elements-in-unreal-engine?application_version=5.4 for more.  
- Abilities are kept small and atomic, intended for use by composition rather than as monolithic abilities.  
- Map simply and elegantly to designs that could be found in ANY OR ALL OF: borderlands 2, Vampire Survivors, Destiny 2.  
- Cleanly express weapon-centric design patterns AND character-centric design patterns. When picking, prefer weapons.  
- Avoid tight coupling between characters, abilities, weapons, spawnables, and pickups. Failure to do this will be excruciating later.
   
  
To meet these needs, we anticipate a mix of components and gameplay abilities, with relatively few tags and relatively few cues. Cues should, as much as possible, be used only for presentation layer effects. If a cue triggers something that has a physics engine interaction, it could produce some very very unpleasant bugs during the process of attempting reconciliation and replication. The GAS library goes to great pains to prevent this, but fundamentally, evented systems are difficult to express in a way that is deterministically ordered without additional primitives that would overcomplicate GAS.  
  
  Finally, this library aims to use GAS in an idiomatic way. It's used extensively for games very much like ours, by very good designers and very smart engineers. 