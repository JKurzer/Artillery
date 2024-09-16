# Artillery: Guns & Abilities Plugin 
Determinism, threaded input processing, and access management primitives married to a core set of basic tools used to bind to a weapon or character actor in a compositional and elegant way, loosely backwards compatible with GAS while simplifying its usage considerably. Artillery provides, as of right now:

- Integration with GAS through ArtilleryGuns
- Elision of issues around complexity and reuse through ArtilleryGuns & DDG
    - Deprioritization of the concept of gameplay effects.
    - Direct access to attributes by safe key look-ups.
- Separation of game simulation ticks from frames
- Separation of game simulation ticks from the game thread
- Threaded input processing with support for strong timesync
- Locomotion state machine binding
- Initial integration with bristlecone
- An abstract controls model that uses a three layer system of key bindings -> intents -> actions
  
Atypically, Artillery is designed to operate in a mixed model between input stream replication, network prediction, and interpolation. Bristlecone streams remote player input to all clients, allowing the remote player proxies to execute abilities and requiring the local client to fully simulate them. The obvious immediate implications are as follows...
- **REQUIREMENT: CUES MUST BE COSMETIC ONLY.** This is already the norm.
- **REQUIREMENT: WE MUST HAVE A WAY TO ENSURE AT-MOST-ONCE EXECUTION OF GAMEPLAY CUES.**
- **REQUIREMENT: ATTRIBUTE STATE REPLICATION MUST BE RECONCILED, NOT JUST BLINDLY APPLIED.**
   
By collating all input streams and using an underlying game simulation, we allow deterministic gameplay without requiring deterministic presentation. This is incredibly powerful, as it separates the concerns in a new way. 

## How does this fit together with the other plugins?
Artillery depends on a number of other plugins maintained by Breach Dogs. These form a loosely coupled ecosystem that provides a powerful range of threading and simulation capabilities suitable for use with any UE5 game where control responsivity or multiplayer are a priority. 
### [Dependency Map](https://miro.com/app/board/uXjVKg1J6qo=/?share_link_id=795066474192)


## Upcoming Features
The main two upcoming features are:  
- **Integration of the jolt physics engine**: We'll be using jolt to create a two-layer simulation model and allow true outcome determinism. Jolt has excellent performance, strong support for multithreaded usage, an excellent developer community, and built-in support for rollbacks.
- **Non-Jank Multiplayer Support**: The big goal of artillery is that if you use it, multiplayer should just mostly work. I know, it's a lofty goal, and I don't promise that you can get to shippable without work, but my goal is to make prototyping multiplayer games far less miserable.
- **FP Determinism**: This will likely be broken into another plugin, but I've started building the mathematical framework required to allow floating point math to be correct without enforced operation ordering for small precisions.
- **Threaded Abilities**: In the longer term, abilities will run on the ArtilleryBusyWorker thread, allowing us to complete a rollback without interacting with the game thread in conjunction with the other features. This is the grail.

## Solution Design
Unlike regular unreal, we have powerful time synchronization primitives from bristlecone and a concept of subframe accuracy. This allows us to quickly compose updates by simply zipping them together using the timestamp. With the concept of zip-and-reconcile available to us, we have a way to do exact state recovery without needing true determinism OR a full gamestate transmission.

## **[Final Architecture](https://miro.com/app/board/uXjVK9qqzUc=/)**  
By tracking the GAMEPLAY ABILITIES as EVENTS and then ZIPPING them together like this - effectively eventual consistency via event sourcing, a la kafka - we can determine if an ability or cue would be a double trigger easily within a very high margin of precision by combining Bristlecone cycle, Bristlecone time, and the event properties.

I intend to expand the conserved attribute to support timestamping its changes, allowing us to do a highly granular reconciliation with a very low actual bandwidth cost, assuming this reconciliation is delta compressed and occurs irregularly. Finally, conserved attributes will be extended even further to tackle the problem of floating point determinism in a general way, based on some tricks around frame boundary based quantization.

## Networking  
This plugin will:   
- Disable replication and RPCs for all played or used abilities FROM CLIENT TO SERVER. Clients trigger Artillery abilities based on input and on replication from the server.
  - We are **fully server authoritative** AND clients **do not** push cues to the server.
  - HOWEVER clients do FULLY simulate all player characters down to their ability systems.
  - This is because the cloned input from bristlecone is used to literally control those actors, out of band from the replication system.
  - This means that MOST of the time, when attributes and transforms are replicated from the server, they represent no changes or imperceptible changes.
  - Attributes and tags are replicated FROM the server in the PUSH model using relevance.
  - Cues MUST only have cosmetic effects.
  - Cues that have attribute changes associated with them WILL cause bugs.
  - We will need to enforce an At-Most-Once rule for Cues triggering visual effects, as remote player input representing an action may arrive LONG before the cues it would generate.
- Accept push updates from the server. As a result, replication FROM the server must still work.   
- Integrate with the reconciliation logic needed to do an eventually consistent server auth game. See 2.   
- Prevent known issues around latency and cooldown misbehavior in the normal up-replication by offlining it entirely if needed.   
- Ensure integration with Iris, which will be our replication provider for Lions.   
   
## Design   
This plugin has five main design goals:  
- All data is managed through Data-Driven Gameplay Elements, Registries or Configs. You can use data assets if you like, but personally, I try to avoid them for systems like this
- Abilities are kept small and atomic, intended for use by composition rather than as monolithic abilities.  
- Map simply and elegantly to designs that could be found in ANY OR ALL OF: borderlands 2, Vampire Survivors, Destiny 2.  
- Cleanly express weapon-centric design patterns AND character-centric design patterns. When picking, prefer weapons.  
- Avoid tight coupling between characters, abilities, weapons, spawnables, and pickups. Failure to do this will be excruciating later.
- Where necessary, integrate with Thistle via a hard API boundary and use of Cues.   
  
To meet these needs, we anticipate a mix of components and gameplay abilities, with relatively few tags and relatively few cues. Cues should, as much as possible, be used only for presentation layer effects. If a cue triggers something that has a physics engine interaction, it could produce some very very unpleasant bugs during the process of attempting reconciliation and replication. The GAS library goes to great pains to prevent this, but fundamentally, evented systems are difficult to express in a way that is deterministically ordered without additional primitives that would overcomplicate GAS.  
  
Finally, this library aims to use GAS in an idiomatic way. It's used extensively for games very much like ours, by very good designers and very smart engineers. It is not safe to assume that the design is bad because it has unusual properties.

## Reference Matter
Artillery will also serve to insulate our other tech from spaghetti usage of GAS, in case we do decide to use another tooling for ability and gun composing. [This guide](https://github.com/tranek/GASDocumentation) is a really useful place to start for understanding GAS. We're using it very differently, in a more composition oriented way that'll hopefully ease the dependency management and granularity issues we ran into at Riot if we go with using Game Feature Plugins.
- [Data Driven Gameplay](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-driven-gameplay-elements-in-unreal-engine?application_version=5.4)
- [GAS, Top-level Docs](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/GameplayAbilitySystem/)
- [Data Registries](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-registries-in-unreal-engine)
- [How to do damage](https://github.com/tranek/GASDocumentation?tab=readme-ov-file#433-meta-attributes) 
- [Procedural Animation With Character](https://www.youtube.com/watch?v=KPoeNZZ6H4s) - useful for missiles, turning turrets, etc.
- [Momentum in Sonic](https://www.youtube.com/watch?v=w1CEN5gVs5Q) - our goal is to make something like this easy.
- [JUICE IT OR LOSE IT](https://www.youtube.com/watch?v=Fy0aCDmgnxg) - really useful for thinking about iterating on feel.
- [Tech Design In Animation](https://www.youtube.com/watch?v=ueEmiDM94IE)
