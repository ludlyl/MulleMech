# Overseer


![alt text](http://images.ctrustnetwork.com/static_pages/gaming/starcraft/unit_images_white/starcraft.2.overseer.png "Image of overseer")

Library for analyzing Starcraft 2 maps by region decomposition. Based on the [Brood War Easy Map architecture](http://bwem.sourceforge.net/) created by Igor Dimitrijevic. It uses the MIT license.

## Getting started

Demo for Commandcenter [is found here.](https://gitlab.com/OverStarcraft/Overseer/blob/master/demo/commandcenter.md) Which is a very short demo just to get started.

Include the file `MapImpl.h` into your project to get started.

`#include "Overseer/src/MapImpl.h"`

You need to pass a pointer to your Agent to the map to have it fully configured. Then you need to call `Intialize()` to construct the map.
Now you're good to go! This is how it would look on Interloper LE

```c++
{
	Overseer::MapImpl map;

	map.setBot(&bot); //Pass a pointer to your sc2::Agent
	map.initialize(); //intialize the map

	std::cout << "Number of tiles on map: " << map.size() << std::endl;
	std::cout << "Number of regions: " << map.getRegions().size() << std::endl;
}
```

Example output:

```
Number of tiles on map: 26752
Number of regions: 18
```

If you want the number of `ChokePoint` you have to check for each region pair since a pair of regions could have multiple `ChokePoint`

## Project status

Overseer is currently under construction. Feel free to make a pull/merge request!

## Documentation

The documentation is for local use only and can be [found here.](https://mejan.github.io) To open it on your local machine generate it via doxygen.
```
$ mkdir doc
$ doxygen docConfig
```

## License

The license for this software (Overseer) can be found [here.](https://gitlab.com/OverStarcraft/Overseer/blob/master/LICENSE.md)

### Third party software license

* [Boost Software License Version 1.0](http://www.boost.org/LICENSE_1_0.txt): Used in Spatial c++ library
