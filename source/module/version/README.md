# Version module

Allows you to set the version number of your project.

You can set it by calling `version_set` once with a 16bit number representing the version number and the number of digits representing the major version with the rest of them representing the minor version.  
For example calling `version_set(10000, 1)` will result in the string representation of the version number being `1.0000`, while `version_set(10000, 2)` will result in `10.000`.

`version_get_numeric` returns the 16bit representation that was given in `version_set` and can be used in calculations for example comparing the current version to the version of a potential firmware update.

`version_get_string` can be used to display the string representation of the version number, for example on a display, a website or via the `app version` command of the debug console.
