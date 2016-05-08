# Creature-Runtimes

This is the runtime for Creature, the advanced 2D Skeletal and Mesh Animation Tool. This runtime is for the Unreal Engine aka UE4 pipeline. It allows you to load in, play back and control your authored Creature Characters in the UE4 environment. Character playback and control are done through Creature's Blueprints enabled functionality.

![img](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/plugin_banner.png)

Trailer Video for the plugin is [**here**](https://youtu.be/S01sZY8mTz4).

For more information on how to use the runtimes, please head over to this [site](http://www.kestrelmoon.com/creaturedocs/Game_Engine_Runtimes_And_Integration/Runtimes_Introduction.html)

### Trying out the new fancy Creature Editor & Runtimes:
The directory containing this is called **CreatureEditorAndPlugin**. Most of the concepts from the old runtimes apply to the new ones so it should not be too big of a change. However, you will get access to some fancy new features, including the CreatureAsset, CreatureMeshComponent, new UI and a state machine.

**Much of the new features and editor work for the new plugin are due to amazing work by [God of Pen](https://github.com/ldl19691031/CreatureUE4PluginWithEditor). Please have a look at his modified plugin here.** Thanks for all the hard work and contributions!

Thanks also to [Eyesiah](https://github.com/Eyesiah) for numerous code fixes and contributions to the plugin!

### What is the roadmap ahead for Creature Runtime development?
Most of the new work will now be focused on the files in **CreatureEditorAndPlugin**. 

### Reporting Bugs/Feature Requests
Feature requests regarding the core Creature UE4 Runtime can be sent directly to: creature@kestrelmoon.com . For bugs/requests with regard to the Creature UE4 Runtime Editor, please send them to:  [God of Pen](https://github.com/ldl19691031/CreatureUE4PluginWithEditor)

##Dinosaur Parade
A scene in the age of dinosaurs, with animation authored and exported from Creature into UE4. This shows 200+ Creature Characters in realtime running with the Creature UE4 Plugin.

![img](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/plugin_banner2.png)

Video demo is [here](https://youtu.be/HWdnUODioko).

##Elephant Stomp! | Game Tech Demo
Elephant Stomp! is a demo game showcasing the Creature Animation System and its UE4 Plugin. The entire game runs off the Creature UE4 Plugin and is written 100% in the BP system.

Control the Elephant and its Magic Rider battling waves of soldiers. As you take out the soldiers, you gain spell powers enabling you to either summon in a rain drop of fish from the heavens or call in a flock of deadly stampeding ostriches!

The character art from the game is from the public domain [British Library Flickr Archive](https://www.flickr.com/photos/britishlibrary/albums).

![img](https://raw.githubusercontent.com/kestrelm/CreatureDemos/master/stomp.png)

View the **Demo Video** [here](https://youtu.be/KbKBJdJn7bA).

###Controls:
Movement - Click to the Left or Right of the Elephant to move

Summon Fish Raindrop - 1

Summon Ostrich Rush - 2

Stomp - Space


Please head over the [CreatureDemos](https://github.com/kestrelm/CreatureDemos) repository to download this demo.


##Creature British Library Art Project
A demo scene consisting of multiple characters with artwork all sourced from the public domain [British Library Flickr Archive](https://www.flickr.com/photos/britishlibrary/albums) is presented here. This demo is constructed in UE4 so you will need UE4.9 and above to open to run the project.

The scene demonstrates multiple advanced features of the Creature Animation Tool, including the usage of ***Force Field Motors for Cloth Dynamics, Custom Cycle Motors for walking and the Creature UE4 Runtime.***

The actual animation logic for the walking pheasants and tapir is all authored using the UE4 Blueprints system.

![img](https://raw.githubusercontent.com/kestrelm/CreatureDemos/master/BL.png)

View the Demo Video [here](https://youtu.be/MQK1mVSXaAk).

Please head over the [CreatureDemos](https://github.com/kestrelm/CreatureDemos) repository to download this demo.

## Buliding the Creature Static Library for other Platforms (PS4, XBox One etc.)

If you want to compile your game using the Creature Plugin for different platforms like the PS4, XBox One etc. you will need to build out the Creature Static Library located in the **ThirdParty** directory.

**Video Tutorial** describing the process is: **[here](https://youtu.be/Ghe-yFsO0u0).**

