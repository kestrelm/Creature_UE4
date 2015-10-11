# Creature-Runtimes

This is the runtime for Creature, the advanced 2D Skeletal and Mesh Animation Tool. This runtime is for the Unreal Engine aka UE4 pipeline. It allows you to load in, play back and control your authored Creature Characters in the UE4 environment. Character playback and control are done through Creature's Blueprints enabled functionality.

For more information on how to use the runtimes, please head over to this [site](http://www.kestrelmoon.com/creaturedocs/Game_Engine_Runtimes_And_Integration/Runtimes_Introduction.html)

### Trying out the new fancy Creature Editor & Runtimes:
The directory containing this is called **CreatureEditorAndPlugin**. Most of the concepts from the old runtimes apply to the new ones so it should not be too big of a change. However, you will get access to some fancy new features, including the CreatureAsset, CreatureMeshComponent, new UI and a state machine.

**Much of the new features and editor work for the new plugin are due to amazing work by [God of Pen](https://github.com/ldl19691031/CreatureUE4PluginWithEditor). Please have a look at his modified plugin here.** Thanks for all the hard work and contributions!

### What is the roadmap ahead for Creature Runtime development?
Most of the new work will now be focused on the files in **CreatureEditorAndPlugin**. 

### Reporting Bugs/Feature Requests
Feature requests regarding the core Creature UE4 Runtime can be sent directly to: creature@kestrelmoon.com . For bugs/requests with regard to the Creature UE4 Runtime Editor, please send them to:  [God of Pen](https://github.com/ldl19691031/CreatureUE4PluginWithEditor)

## Dragon Demo
[![Non](http://www.kestrelmoon.com/creaturedocs/img/ue4-dragon.png)](https://youtu.be/ymOOUtaEcsI)

The DragonDemo is an archive under the directory **"DragonDemo"**, expand it to get the full project, runtimes etc. 
The Demo character is authored in Creature and exported out into UE4 using Creature's UE4 runtimes.
Actual character gameplay is done using BP.

###Controls:

Move Left - a

Move Right - d

Breathe Fire - s

## Prepackaged Plugin Code for Compilation

If you want to compile the plugins for a C++ project, I have included a Prepackaged archive that you can use
to save you the steps in copying the files over and setting up the appropriate directory structures.

Look in the folder **PrePackagePluginSetup** for that archive.

***The above documentation pertains to the old set of Creature runtimes. they are still accessible under the CreaturePluginOld directory.*** 
