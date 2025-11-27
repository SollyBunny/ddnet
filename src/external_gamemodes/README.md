# external gamemodes

ddnet-insta offers a way for you to create own gamemodes.
Their source code can be in its own git repository that can be integrated
in the main server without having to patch a single line of code.

An example can be setup by running the following command in the current directory:

```
git clone git@github.com:ddnet-insta/external_gamemode_sample.git
```

Then recompile the main code base and in game you now have the `sv_gametype sample` available.
Because this uses globbing you probably have to rerun the cmake step manually before it works.


