# Artillery: Guns & Abilities Plugin for Lions
Complexity and access management primitives married to a core set of basic weapon abilities used to bind to a weapon or character actor in a compositional and elegant way, compatible with GAS. Artillery is built on top of GAS, and is generally a fairly thin layer over it, providing convenience functions and ECS-like capabilities. Critically, Artillery is also used for making a few changes to ensure that we can keep debug complexity in the networked case to a minimum.   
  
GAS is used in fortnite, among other major titles, including a number of Vampire Survivor successors. Artillery will also serve to insulate our other tech from spaghetti usage of GAS, in case we do decide to use another tooling for ability and gun composing. [This guide](https://github.com/tranek/GASDocumentation) is a really useful place to start for understanding GAS. We're using it very slightly differently, in a more composition oriented way that'll hopefully ease the dependency management and granularity issues we ran into at Riot if we go with using Game Feature Plugins.  

||Oh Boy!||      
:---|:---:|---:|
||![Yellow_Alert](https://github.com/JKurzer/Artillery/assets/7749511/c4fe6e1b-e402-4a0e-8d73-638896b9f79f)||
||Required Prior Reading: [Official Replication Discussion](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system)||  

  
As of the Tranek docs' updates, we MAY have issues [REPLICATION](https://github.com/tranek/GASDocumentation?tab=readme-ov-file#concepts-asc) for the [Ability System Component (ASC)](https://github.com/tranek/GASDocumentation?tab=readme-ov-file#concepts-asc) and [AttributeSets (AtSets)](https://github.com/tranek/GASDocumentation?tab=readme-ov-file#concepts-as). Much of this centers on how cues work, but attribute data isn't really exempt either. This is because Bristlecone streams remote player input to all clients, allowing the remote player proxies to execute abilities and requiring the local client to fully simulate them. This is a meaningful breach of expectations, and a potential pain point which might lead us to seriously consider the Network Prediction Plugin. I would Really like to avoid using it, as it is extremely unfinished. The obvious immediate implications are as follows...
- **REQUIREMENT: CUES MUST BE COSMETIC ONLY.** This is already the norm.
- **REQUIREMENT: WE MUST HAVE A WAY TO ENSURE AT-MOST-ONCE EXECUTION OF GAMEPLAY CUES.**
- **REQUIREMENT: ATTRIBUTE STATE REPLICATION MUST BE RECONCILED, NOT JUST BLINDLY APPLIED.**
   
Thankfully, it looks like 5.x has made progress on this. I'll update as I go. GAS is Iris enabled, so we may ALSO be able to solve this with some delicate but fairly simple work here in artillery with custom replication fragments. I'm pretty anxious about it. I think we can get away with simply not replicating cues except in a push-to-client model with our single authoritative server and meeting the above requirements. I'm much more worried about how to figure out the case where we have newer player input than the state update was based on BUT are also missing one of the inputs it was based on. I'm worried deterministic rollback will be necessary, because that's cripplingly slow for games with high numbers of entities. I can almost see a solution though, and we'll need to pretty much embody that solution here in artillery.

To finish out the fun, we may also have an issue with how and where and when ASCs and AtSets can be instantiated. I'm hoping we can skirt this stuff, to be honest. I really want to be able to use GAS but I'm still assessing if that's optimism or intellect talking.  

## Solution Design
Unlike regular unreal, we have powerful time synchronization primitives from bristlecone and a concept of subframe accuracy. This allows us to quickly compose updates by simply zipping them together using the timestamp. Gameplay Cues remain an outstanding question, but with the concept of zip-and-reconcile available to us, we have a way to do exact state recovery without needing true determinism OR a full gamestate transmission.

## **[Final Architecture](https://miro.com/app/board/uXjVK9qqzUc=/)**  
By tracking the GAMEPLAY ABILITIES as EVENTS and then ZIPPING them together like this - effectively eventual consistency via event sourcing, a la kafka - we can determine if an ability or cue would be a double trigger easily within a very high margin of precision by combining Bristlecone cycle, Bristlecone time, and the event properties. I'm not exactly sure what form this will need to take, and it's not clear to me yet what the best approach is for reconciliation, but we do have the primitives needed to actually perform it.

I intend to expand the conserved attribute to support timestamping its changes, allowing us to do a highly granular reconciliation with a very low actual bandwidth cost, assuming this reconciliation is delta compressed and occurs irregularly. There's still a bunch of questions here, but I now believe it to be tractable. I think there's actually already some stuff for preventing double triggers of cues in Unreal, but I haven't refreshed my memory yet. Now it feels like it's just a matter of sorting the puzzle pieces.

## Networking  
This plugin will:   
- Disable replication and RPCs for all played or used abilities FROM CLIENT TO SERVER. Clients trigger GAS abilities based on input and on replication from the server.
  - We are **fully server authoritative** AND clients **do not** push cues to the server.
  - HOWEVER clients do FULLY simulate all player characters down to their ability systems.
  - This is because the cloned input from bristlecone is used to literally control those actors, out of band from the replication system.
  - This means that MOST of the time, when attributes and transforms are replicated from the server, they represent no changes or imperceptible changes.
  - Cues and tags are replicated FROM the server in the PUSH model.
  - Cues MUST only have cosmetic effects.
  - Cues that have attribute changes associated with them WILL cause bugs.
  - We will need to enforce an At-Most-Once rule for Cues triggering visual effects, as remote player input representing an action may arrive LONG before the cues it would generate.
- Accept push updates from the server. As a result, replication FROM the server must still work.   
- Integrate with the reconciliation logic needed to do an eventually consistent server auth game. See 2.   
- Prevent known issues around latency and cooldown misbehavior in the normal up-replication by offlining it entirely if needed.   
- Ensure integration with Iris, which will be our replication provider for Lions.   
   
## Design   
This plugin has five main design goals:  
- All data is managed through Data-Driven Gameplay Elements, Registries or Configs.
- Abilities are kept small and atomic, intended for use by composition rather than as monolithic abilities.  
- Map simply and elegantly to designs that could be found in ANY OR ALL OF: borderlands 2, Vampire Survivors, Destiny 2.  
- Cleanly express weapon-centric design patterns AND character-centric design patterns. When picking, prefer weapons.  
- Avoid tight coupling between characters, abilities, weapons, spawnables, and pickups. Failure to do this will be excruciating later.
- Where necessary, integrate with Thistle via a hard API boundary and use of Cues. Minimize this.
- Either own or explicitly do not own animation and game feel of abilities and guns.
   
  
To meet these needs, we anticipate a mix of components and gameplay abilities, with relatively few tags and relatively few cues. Cues should, as much as possible, be used only for presentation layer effects. If a cue triggers something that has a physics engine interaction, it could produce some very very unpleasant bugs during the process of attempting reconciliation and replication. The GAS library goes to great pains to prevent this, but fundamentally, evented systems are difficult to express in a way that is deterministically ordered without additional primitives that would overcomplicate GAS.  
  
Finally, this library aims to use GAS in an idiomatic way. It's used extensively for games very much like ours, by very good designers and very smart engineers. It is not safe to assume that the design is bad because it has unusual properties. On FLX at Riot, we lost months due to people skipping GAS in favor of hand-rolling a nearly identical system. I'd like to not repeat that mistake.

## Reference Matter
- [Data Driven Gameplay](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-driven-gameplay-elements-in-unreal-engine?application_version=5.4)
- [GAS, Top-level Docs](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/GameplayAbilitySystem/)
- [Data Registries](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-registries-in-unreal-engine)
- [Unofficial Companion](https://github.com/tranek/GASDocumentation) - probably THE best actual guide to how GAS and why GAS. Dated in places, be careful!
  - [How to do damage](https://github.com/tranek/GASDocumentation?tab=readme-ov-file#433-meta-attributes) 
- [Procedural Animation With Character](https://www.youtube.com/watch?v=KPoeNZZ6H4s) - useful for missiles, turning turrets, etc.
- [Momentum in Sonic](https://www.youtube.com/watch?v=w1CEN5gVs5Q)
- [JUICE IT OR LOSE IT](https://www.youtube.com/watch?v=Fy0aCDmgnxg)
- [Tech Design In Animation](https://www.youtube.com/watch?v=ueEmiDM94IE)
