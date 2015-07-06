# Cooking

0. Browse to `Engine\Build\BatchFiles`
0. Run the build automation tool with `RunUAT.bat MakeUTDLC -DLCName=UTVehicles -platform=Win64 -build`
0. Wait until cooking process is done (could take a while)
0. After cooking is done, the package plugin can be found under:
  `UnrealTournament\Saved\StagedBuilds\UTVehicles\Win64\UnrealTournament\Plugins\UTVehicles`

# Packaging

By packaging the plugin, every asset and the binary compiled code is included in a zip file. Every documentation is converted into a properly readable HTML file to be opened locally.

0. Browse to `Engine\Build\BatchFiles`
0. Run the build automation tool with `RunUAT.bat UTVehicles_PackageZip -plugin=UTVehicles -platform=Win64 -solution=Editor`
0. Wait until the packaging process is done
0. Once the process successfully completed, the newly created package is stored in the **Release** folder

## Internal build
 
Internal builds are shared privately and aren't published. The internal build process automatically increases the build number stored in the [VERSION file](VERSION).

The process doesn't differ from the [packaging part](#packaging-internal-build) but the additional flag has to be added.

- **-publish** - Package plugin and increase build number (internal or published build)

## Publishing plugin
  
The process doesn't differ from the [packaging part](#packaging-internal-build) but some additional flags have to be added. The **publish flag** also has to be used.

- **-release** - Package plugin, removes build number and creates publish package
- **-commit** - automatically creates a new commit and a new tag in Git which can be pushed

Append specific flags to your command. Most common command would be:

- *Internal build*  
  `RunUAT.bat UTVehicles_PackageZip -plugin=UTVehicles -platform=Win64 -solution=Editor -publish`

- *Public build*  
  `RunUAT.bat UTVehicles_PackageZip -plugin=UTVehicles -platform=Win64 -solution=Editor -publish -release`

- *Public build with auto-commit message*  
  `RunUAT.bat UTVehicles_PackageZip -plugin=UTVehicles -platform=Win64 -solution=Editor -publish -release -commit`

## Source code

Git features the current repository content in every release. However, if you want to package plugin to include the source code, you can append the following flag to your command:

```
-source
```

The packaged source files differ from the other version. Documentation files aren't converted and copied as original files. The build script is also only existing in the source code version.