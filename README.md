UTVehicles
==========
[![Slack](http://utvehicles-slack.herokuapp.com/badge.svg)](http://utvehicles-slack.herokuapp.com/)
[![downloads](https://img.shields.io/github/downloads/UTCommunity/UTVehicles/latest/total.svg)](https://github.com/UTCommunity/UTVehicles/releases/latest)
[![issues](https://img.shields.io/github/issues/UTCommunity/UTVehicles.svg)](https://github.com/UTCommunity/UTVehicles/issues)
[![tag](https://img.shields.io/github/tag/UTCommunity/UTVehicles.svg)](https://github.com/UTCommunity/UTVehicles/tags)
[![releases](https://img.shields.io/github/release/UTCommunity/UTVehicles.svg)](https://github.com/UTCommunity/UTVehicles/releases)
[![license](https://img.shields.io/badge/license-UE4%2FUT-blue.svg)](https://www.unrealengine.com/eula)

A vehicle framework for [Unreal Tournament](//www.unrealtournament.com/)

This repository is part of the *UTVehicles*-plugin which consists of the **code part** and the **asset part**. You can find the assets repository in its [latest state here](https://github.com/UTCommunity/UTVehiclesContent). That repository is included as a sub-module in the specific folder called [Content](Content/) which points to a specific state (to feature full compatibility to the code).

**You want to be part of the development team?** Join us! [![Join the UT Vehicles Community on Slack](http://utvehicles-slack.herokuapp.com/badge.svg)](http://utvehicles-slack.herokuapp.com/)

## Quick start

### Developing

- Download the [latest release](/../../releases/latest)
- Unpack the file into the `Plugins` folder located at:  
  `UnrealTournamentEditor\UnrealTournament\Plugins`
- Start the editor and open the `TestVehicles` map  
  (if the editor doesn't show the map/classes, tick **Show Plugin Content** under **View options** on the lower right in the content browser)
- Start creating vehicles. Check the Wiki for more information

**Note:** You can also create your own maps, but you have to set-up some game type properties. This is required to have fully support for vehicle driving, entering and such.


### Play-testing

**Note**: Currently not supported.

- Download the [latest release](/../../releases/latest) for the game
- Unpack the file and place the PAK-file into your `Content` folder  
  (`UnrealTournamentDev\UnrealTournament\Content\Paks`)
- Open the game
- Choose the game`TEMPGAME` and select any map
- Have fun driving around

## What's included

The current framework comes with limited content. There is a test map (which is basically a copy of the Example map) as a playground for vehicle driving. This map will evolve over time and change drastically.

That map has a vehicle available which can be entered and driven around.

The following classes can be used to create vehicles:

- **Vehicle**: Basic vehicle which supports steering and throttling, entering and exiting, a visible mesh, suiciding, and such but no _proper_ physics simulation
- **UTVehicle**: Currently a stub which will be the base class for most UT vehicles (probably even all of them)
- **SVehicle**: Quite similar to Vehicle (which is sub-classed to be specific) but with *PhysX*-physics simulation

Every asset for creating UT vehicles is accessible in the original repository of [Unreal Tournament](https://github.com/EpicGames/UnrealTournament)

## Contributing

Please read through our [contributing guidelines](CONTRIBUTING.md). Included are directions for opening issues, coding standards, and notes on development.

Have a bug or a feature request? Please first read the [issue guidelines](CONTRIBUTING.md#using-the-issue-tracker) and search for existing and closed issues. If your problem or idea is not addressed yet, [please open a new issue](issues/new).

Or just simply join our [communication platform](http://utvehicles-slack.herokuapp.com/) or chat directly wit us at our [public chat room](https://www.hipchat.com/gDpVJx3Gd) (no registration required).

## Compiling

This plugin is created with the code-base of the latest release of the Unreal Tournament Source code.  In order to fully support using this plugin with the [Launcher versions of Unreal Tournament](https://www.unrealtournament.com/download), a specific commit/version/tag needs to be checked out. You can always find the used commit in the [CHANGELIST file](CHANGELIST) which directs to the specific commit in the release branch of the UT repository.

Everything else is a simple setup:

- Clone the repository (or use your own fork)
- Move/symlink the repository folder into the `Plugins` folder of your Git-repository of Unreal Tournament
- Generate the project files with the specific batch file
- Compile the solution

For more information about the cooking part, check the specific [cooking guideline](COOKING.md)

## Credits

You can find the credits [here](CREDITS.md)

## License

Licensed under the terms of the latest version of the **Unreal® Engine End User License Agreement**, obtainable at [unrealengine.com/eula](//unrealengine.com/eula).

A copy of the license text, as obtained on 2015-06-26, is included in the [UE4EULA.pod](UE4EULA.pod) file (for reference purposes only).