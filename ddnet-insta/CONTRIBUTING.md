# Contributing code to ddnet-insta

ddnet-insta tries to stay as close as possible to its upstream [ddnet](https://github.com/ddnet/ddnet)
so all guidelines from [ddnet's CONTRIBUTING.md](../CONTRIBUTING.md) apply here as well.
This file only contains the ddnet-insta specific additions.

## conventional commits

This repository uses a flavor of https://www.conventionalcommits.org/en/v1.0.0/
which is enforced quite strictly. And used for the release changelogs as well.
Because this is not the case in ddnet and ddnet-insta regularly merges into it
the git log is not fully following this but all commits made directly to this repository have to follow it.


The prefixes are mostly non standard ones here is a list of prefixes you should use:
- `logs:` if your commit only changes the servers log output
- `refactor:` if there is a internal change that the end user can not observe
- `fix:` If you fix a bug. If you fix a bug that is mode specific it is preferred to do `zcatch: fix respawn logic` instead.
- `docs:` if you change something in the main README.md file
- `chore:` if you do some small thing that does not really belong anywhere but has to be done
- `ci:` if your commit touches the github actions
- `docker:` if you work on the Dockerfile directly or on related files like the docker compose
- `ddrteams:` if your commit touches the ddrace team feature (/team chat command)
- `stats:` if you work on the round stats or any stats related chat command or similar
- `config:` if you add, remove or rename a config variable. Where rename and remove need to be `config!:` because it is a breaking change.
- `rcon:` if you add, remove, rename or edit a rcon command
- avoid `feat:` use the gametype name it affects instead like `vanilla:`, `ctf:`, `instagib:`, `laser:`, `fng:`, `bomb:` or so on
- `controller:` if you add, remove or edit a controller command that is not specific to any feature or mode yet

These prefixes are for the users of the project and aim to be useful to show relevant information in the [release changelogs](https://github.com/ddnet-insta/ddnet-insta/releases).
The `chore:` and `refactor:` prefixes will always be stripped from the changelog. The `fix:` will only be included if it fixes a bug that was present in the previous release.


Here are some example commit messages of things that happen often:
- fng: add fng gametype
- stats: add /rank_kills command
- config!: rename `sv_foo` to `sv_bar`
- rcon: add `kill_all` command
