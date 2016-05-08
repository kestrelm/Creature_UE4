#Creature Pack UE4 Animation Plugin

This document describes the **Creature Pack Animation Plugin** for **Unreal Engine**. The plugin allows you to playback and control characters exported by the [**Creature Animation Tool**](http://creature.kestrelmoon.com/).

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/packshort.gif)

Trailer video is [here](https://youtu.be/S01sZY8mTz4).

##Creature Pack File Format vs Creature JSON/Flat Data

The **Creature Pack File Format** is a **lightweight version** of the **Creature JSON/Flat Data** format. It contains a bit less functionality than its original sibling but has faster playback and a much more compact representation.

The **Creature Pack Plugin** is suitable for:

	- Playing back Creature Animations for General Usage
	- Blending and Switching between different Animation Clips
	- Attaching external objects/spawning particles off vertex positions of your character mesh
	- Having a large group of Creature Animations for playback ( crowds scenarios )

You can see this file format handles most of the major functionality you will require for a 2D game.

This plugin **does not have bone/skeletal information** encoded in it. This results in a much more **compact representation and faster playback speed(s)**. If you require the full functionality of the **Creature Runtime**, please have a look at the [**Creature Plugin**](https://github.com/kestrelm/Creature_UE4) which uses the **Creature JSON/Flat Data** format.

##Using the Plugin

###Installation
You need a **C++ UE4 Project** so set this up first. Make a **Plugins** folder in your project directory. Drop the **CreaturePackPlugin** folder into the **Plugins** folder. Now **Generate Project Files** to generate the appropriate project files to compile your project. Double click and open up your project to build and install the plugin.

###Export Animation from Creature

With your character's project open, go into the **Animation Mode** in **Creature**. Click on **Export Animation -> Game Engines**:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen1.png)


A new window pops up:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen2.png)

The options you care about are contained in the **CreaturePack Web Format** panel:

- **Gap Step:** How to sample the animation. The higher the value, the higher the approximation level. **Higher Approximation Levels result in smaller file sizes at the expense of quality**. Try values between 2 to 6.

- **Pack Uv Swaps:** Check this box if you are using Sprite Swapping Animations. Leave unchecked to reduce the file size.

- **Pack Colors:** Check this box if you are animating opacity of your meshes over time. Leave unchecked to reduce the file size.

Click **Export** to export your project into a folder of your choice. The **exported folder** will contain 2 files of interest:

- **YourCharacter.creature_pack** - Your exported animation pack file

- **YourCharacter_img.png** - Your character's texture atlas

###Importing Animation Data into UE4

First, load up **UE4** and start a new project.

####Import Character Texture Atlas & Create Material
From the previous step, find your character's image atlas from the export folder and import it into **UE4**. **Create a Material** out of that image. The recommended **Material Type** is the **Mask** material. Remember to connect up the **Opacity** channel of your material from the **Texture Sampler**. Save this material.


####Create Character Animation Asset
In your assets panel, **right click** and create a new **Creature Pack Animation Asset**:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen3.png)

A new window will pop up allowing you to select the exported animation file from the export folder.


###Creating a Character for Playback
First create a new **Empty Actor**. Convert that actor into a **Blueprint** object. Open it up and add a new **CreaturePackMesh** component:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen4.png)

Now let us assign the animation asset to our new component. Click on **Animation Asset** in the right hand side panel and select your animation asset created from the previous steps:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen5.png)

The next thing we need to do is to assign the Material created from the previous steps:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen6.png)

With both those 2 steps completed, you should see your character displayed in the Blueprint Window:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen7.png)

When you play your level, the character should start animating with its default animation!

###Controlling playback of your Character with Blueprints

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen8.png)

A number of functions are available for you to control your character:

- **Set Active Animation**: Instantly switches the animation of your character with a given name

- **Set Blend Active Animation**: Blends smoothly to another animation  with a given name and a blend factor. The blend factor is a value > 0 and <= 1.0. A value of 0.05 will give you a nice gradual transition to the target animation.

- **Set Should Loop**: Sets whether the animation should loop or not.

There are also properties you can query/change at runtime:

- **Animation Speed**: Changing this will change how quickly the animation plays back

- **Animation Frame**: Returns the curent frame the animation is on

- **Region Offset Z**: How far the z value of each region is pushed in relatively to each other. Increase this value if you are experiencing z-fighting rendering artifacts.

###Attaching objects/spawning off parts of your Character

You can attach or spawn emitters off your character at a **per vertex** level. A visualization tool and property is available to allow you to do just that.

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen9.png)

- **Attach Vertex Id**: This value starts at -1 which means no vertex is selected. Increase the value from 0 upwards to see a **rounded yellow sphere rendered at the selected vertex.** You can scroll your mouse through the list of vertex ids to quickly pick a vertex for attachment.

With the id selected, you can go into Blueprints and setup the following graph:

![Demo Scene](https://raw.githubusercontent.com/kestrelm/Creature_UE4/master/CreaturePackPlugin/pack_screen10.png)

The function to pay attention to is:

- **Get Attachment Position**: This returns a FVector position in World Space of the selected vertex id. If you leave the input parameter id to -1, it will use the id you set in the **Attach Vertex Id** property value. You can also set another id to extract the position from.

Use **Get Attachment Position** to grab vertex positions off your character and spawn or attach objects to it.



