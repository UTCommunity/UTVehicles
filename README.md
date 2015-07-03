UTVehicles
==========
A vehicle framework for [Unreal Tournament](www.unrealtournament.com/)

This repository is a part of the *UTVehicles*-plugin which consists of the **code part** and the **asset part**. You can find the assets repository in its [latest state here](https://github.com/UTCommunity/UTVehiclesContent). That repository is included as a sub-module in the specific folder called [Content](Content/) which points to a specific state (to feature full compatibility to the code).

**You want to be part of the development team?** Join us! [![Join the UT Vehicles Community on Slack](http://utvehicles-slack.herokuapp.com/badge.svg)](http://utvehicles-slack.herokuapp.com/)

## Quick start

### Developing

- Download the [latest release](/../../releases/latest)
- Unpack the file and place the DLL-files into your `Binaries` folder
- Start the editor and open the `TestVehicles` map
- Start creating vehicles. Check the following Wiki for more information

**Note:** You can also create your own maps, but you have to set-up some game type properties. This is required to have fully support for vehicle driving, entering and such.

### Play-testing

- Download the [latest release](/../../releases/latest)
- Unpack the file and place the PAK-file into your `Content` folder  
  (`UnrealTournamentDev\UnrealTournament\Content\Paks`)
- Open the game
- Choose the game`TEMPGAME` and select any map
- Have fun driving around

## What's included

The current framework comes with limited content. There is a test map (which is basically a copy of the Example map) as a playground for vehicle driving. This map will evolve over time and change drastically.

That map as a vehicle available which can be entered and driven around.

The following classes can be used to create vehicles:

- **Vehicle**: Basic vehicle which supports steering and throttling, entering and exiting, a visible mesh, suiciding, and such but no _proper_ physics simulation
- **UTVehicle**: Current a stub which will be the base class for most UT vehicles (probably even all of them)
- **SVehicle**: Quite similar to Vehicle (which is sub-classed to be specific) but with *PhysX*-physics simulation

Every asset for creating UT vehicles is accessible in the original repository of [Unreal Tournament](https://github.com/EpicGames/UnrealTournament)

## Contributing

Please read through our [contributing guidelines](CONTRIBUTING.md). Included are directions for opening issues, coding standards, and notes on development.

Have a bug or a feature request? Please first read the [issue guidelines](CONTRIBUTING.md#using-the-issue-tracker) and search for existing and closed issues. If your problem or idea is not addressed yet, [please open a new issue](issues/new).

Or just simply join our [communication platform](http://utvehicles-slack.herokuapp.com/) or chat directly wit us at our [public chat room](https://www.hipchat.com/gDpVJx3Gd) (no registration required).

## Compiling

This plugin is created with the code-base of the latest release of the Unreal Tournament Source code.  In order to fully support using this plugin with the [Launcher versions of Unreal Tournament](https://www.unrealtournament.com/download)), a specific commit/version/tag needs to be checked out. You can always find the used commit in the [VERSION file](VERSION) which directs to the specific commit in the release branch of the UT repository.

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